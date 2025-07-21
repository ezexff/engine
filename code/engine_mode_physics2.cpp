internal void
UpdateAndRenderPhysics2(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_physics2 *ModePhysics2 = &GameState->ModePhysics2;
    if(!ModePhysics2->IsInitialized)
    {
        ModePhysics2->InitializedEntityCount = 1;
        ModePhysics2->Series = RandomSeed(200);
        
        test_entity *Ground = &ModePhysics2->EntityArray[0];
        Ground->IsInitialized = true;
        Ground->IsStatic = true;
        Ground->Type = TestEntityType_Rect;
        Ground->VertexCount = ArrayCount(Ground->VertexArray);
        Ground->VertexArray[0] = V2(-0.5f, -0.5f);
        Ground->VertexArray[1] = V2(0.5f, -0.5f);
        Ground->VertexArray[2] = V2(0.5f, 0.5f);
        Ground->VertexArray[3] = V2(-0.5f, 0.5f);
        Ground->P = V2(1920 / 2, 1080 / 6);
        Ground->Dim = V2(1920 * 0.6f, 70);
        Ground->Density = 1.0f;
        Ground->Restitution = 0.5f;
        r32 Area = Ground->Dim.x * Ground->Dim.y;
        Ground->Mass = Area * Ground->Density;
        Ground->InvMass = 0.0f;
        Ground->Color = V4(0, 1, 0, 1);
        Ground->OutlineColor = V4(1, 1, 1, 1);
        
        ModePhysics2->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
    {
        for(u32 Index = 0;
            Index < ArrayCount(ModePhysics2->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModePhysics2->EntityArray + Index;
            if(!Entity->IsInitialized)
            {
                ModePhysics2->InitializedEntityCount++;
                Entity->IsInitialized = true;
                Entity->IsStatic = false;
                Entity->Type = TestEntityType_Rect;
                Entity->VertexCount = ArrayCount(Entity->VertexArray);
                Entity->VertexArray[0] = V2(-0.5f, -0.5f);
                Entity->VertexArray[1] = V2(0.5f, -0.5f);
                Entity->VertexArray[2] = V2(0.5f, 0.5f);
                Entity->VertexArray[3] = V2(-0.5f, 0.5f);
                Entity->P = V2(Input->MouseP.x, Input->MouseP.y);
                Entity->Dim = V2((r32)RandomBetween(&ModePhysics2->Series, 40, 60),
                                 (r32)RandomBetween(&ModePhysics2->Series, 40, 60));
                Entity->Density = 2.0f;
                Entity->Restitution = 0.6f;
                r32 Area = Entity->Dim.x * Entity->Dim.y;
                Entity->Mass = Area * Entity->Density;
                Entity->InvMass = 1.0f / Entity->Mass;
                r32 R = RandomUnilateral(&ModePhysics2->Series);
                r32 G = RandomUnilateral(&ModePhysics2->Series);
                r32 B = RandomUnilateral(&ModePhysics2->Series);
                Entity->Color = V4(R, G, B, 1.0f);
                Entity->OutlineColor = V4(1, 1, 1, 1);
                
                break;
            }
        }
    }
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
    {
        for(u32 Index = 0;
            Index < ArrayCount(ModePhysics2->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModePhysics2->EntityArray + Index;
            if(!Entity->IsInitialized)
            {
                ModePhysics2->InitializedEntityCount++;
                Entity->IsInitialized = true;
                Entity->IsStatic = false;
                Entity->Type = TestEntityType_Circle;
                Entity->P = V2(Input->MouseP.x, Input->MouseP.y);
                r32 Diameter = (r32)RandomBetween(&ModePhysics2->Series, 40, 60);
                Entity->Radius = Diameter / 2.0f;
                Entity->Dim = V2(Diameter, Diameter);
                Entity->Density = 2.0f;
                Entity->Restitution = 0.6f;
                r32 Area = Entity->Radius * Entity->Radius * Pi32;
                Entity->Mass = Area * Entity->Density;
                Entity->InvMass = 1.0f / Entity->Mass;
                r32 R = RandomUnilateral(&ModePhysics2->Series);
                r32 G = RandomUnilateral(&ModePhysics2->Series);
                r32 B = RandomUnilateral(&ModePhysics2->Series);
                Entity->Color = V4(R, G, B, 1.0f);
                Entity->OutlineColor = V4(1, 1, 1, 1);
                
                break;
            }
        }
    }
    
    // NOTE(ezexff): transform rect vertices
    v4 WhiteColor = V4(1, 1, 1, 1);
    //v4 RedColor = V4(1, 0, 0, 1);
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModePhysics2->EntityArray);
        ++EntityIndex)
    {
        // NOTE(ezexff): pre transform work
        test_entity *Entity = ModePhysics2->EntityArray + EntityIndex;
        if(Entity->IsInitialized)
        {
            r32 MinX = F32Max;
            r32 MinY = F32Max;
            r32 MaxX = F32Min;
            r32 MaxY = F32Min;
            
            if(Entity->Type == TestEntityType_Rect)
            {
                v3 P = V3(Entity->P.x, Entity->P.y, 1.0f);
                v3 Dim = V3(Entity->Dim.x , Entity->Dim.y, 1.0f);
                m4x4 Model =  Translate(P) * Scale(Dim) * ZRotation(Entity->Angle);
                u32 VertexCount = ArrayCount(Entity->VertexArray);
                for(u32 Index = 0;
                    Index < ArrayCount(Entity->VertexArray);
                    ++Index)
                {
                    v2 *OriginalVertex = Entity->VertexArray + Index;
                    v2 *TransformedVertex = Entity->TransformedVertexArray + Index;
                    *TransformedVertex = (Model * V4(OriginalVertex->x, OriginalVertex->y, 0, 0)).xy;
                    
                    if(TransformedVertex->x < MinX){MinX = TransformedVertex->x;}
                    if(TransformedVertex->y < MinY){MinY = TransformedVertex->y;}
                    if(TransformedVertex->x > MaxX){MaxX = TransformedVertex->x;}
                    if(TransformedVertex->y > MaxY){MaxY = TransformedVertex->y;}
                }
                
                Entity->OutlineColor = WhiteColor;
            }
            else if(Entity->Type == TestEntityType_Circle)
            {
                MinX = Entity->P.x - Entity->Radius;
                MinY = Entity->P.y - Entity->Radius;
                MaxX = Entity->P.x + Entity->Radius;
                MaxY = Entity->P.y + Entity->Radius;
            }
            Entity->AABB = {V2(MinX, MinY), V2(MaxX, MaxY)};
        }
    }
    
    // NOTE(ezexff): move
    u32 IterationMax = 1;
    v2 Gravity = V2(0.0f, -9.81f);
    Gravity *= 100.0f;
    r32 dt = Input->dtForFrame;
    for(u32 Iteration = 0;
        Iteration < IterationMax;
        ++Iteration)
    { 
        for(u32 EntityIndex = 0;
            EntityIndex < ArrayCount(ModePhysics2->EntityArray);
            ++EntityIndex)
        {
            test_entity *Entity = ModePhysics2->EntityArray + EntityIndex;
            if(Entity->IsInitialized)
            {
                if(!Entity->IsStatic)
                {            
                    Entity->dP += Gravity * dt / (r32)IterationMax;
                    Entity->P += Entity->dP * dt / (r32)IterationMax;
                    Entity->Force = {};
                }
            }
        }
        
        // NOTE(ezexff): clear contact array
        ModePhysics2->ContactCount = 0;
        for(u32 Index = 0;
            Index < ModePhysics2->ContactCount;
            ++Index)
        {
            ModePhysics2->ContactArray[Index] = {};
        }
        
        // NOTE(ezexff): collision detection
        for(u32 EntityIndex = 0;
            EntityIndex < ArrayCount(ModePhysics2->EntityArray) - 1;
            ++EntityIndex)
        {
            test_entity *Entity = ModePhysics2->EntityArray + EntityIndex;
            if(Entity->IsInitialized)
            {
                for(u32 TestIndex = EntityIndex + 1;
                    TestIndex < ArrayCount(ModePhysics2->EntityArray);
                    ++TestIndex)
                {
                    test_entity *TestEntity = ModePhysics2->EntityArray + TestIndex;
                    
                    if(TestEntity->IsInitialized)
                    {
                        {
                            if(Entity->IsStatic && TestEntity->IsStatic)
                            {
                                continue;
                            }
                            
                            if(!RectanglesIntersect(Entity->AABB, TestEntity->AABB))
                            {
                                continue;
                            }
                            
                            intersect_result Result = {};
                            if(Entity->Type == TestEntityType_Rect)
                            {
                                if(TestEntity->Type == TestEntityType_Rect)
                                {
                                    Result = IntersectPolygonsOptimized(Entity->P, Entity->VertexCount, Entity->TransformedVertexArray,
                                                                        TestEntity->P, TestEntity->VertexCount, TestEntity->TransformedVertexArray);
                                }
                                else if(TestEntity->Type == TestEntityType_Circle)
                                {
                                    Result = IntersectCirclePolygonOptimized(TestEntity->P, TestEntity->Radius,
                                                                             Entity->P, Entity->VertexCount, Entity->TransformedVertexArray);
                                    Result.Normal = -Result.Normal;
                                }
                            }
                            else if(Entity->Type == TestEntityType_Circle)
                            {
                                if(TestEntity->Type == TestEntityType_Rect)
                                {
                                    Result = IntersectCirclePolygonOptimized(Entity->P, Entity->Radius,
                                                                             TestEntity->P, TestEntity->VertexCount, TestEntity->TransformedVertexArray);
                                }
                                else if(TestEntity->Type == TestEntityType_Circle)
                                {
                                    Result = IntersectCircles(Entity->P, Entity->Radius, TestEntity->P, TestEntity->Radius);
                                }
                            }
                            if(Result.IsCollides)
                            {
                                if(Entity->IsStatic)
                                {
                                    TestEntity->P += Result.Normal * Result.Depth;
                                }
                                else if(TestEntity->IsStatic)
                                {
                                    Entity->P += -Result.Normal * Result.Depth;
                                }
                                else
                                {
                                    r32 DepthDiv2 = Result.Depth / 2.0f;
                                    Entity->P += -Result.Normal * DepthDiv2;
                                    TestEntity->P += Result.Normal * DepthDiv2;
                                }
                                //TestEntity->OutlineColor = RedColor;
                                //Entity->OutlineColor = RedColor;
                                contact_points ContactPoints = FindContactPoints(Entity, TestEntity);
                                test_contact *Contact = &ModePhysics2->ContactArray[ModePhysics2->ContactCount];
                                Assert(Entity);
                                Assert(TestEntity);
                                Contact->BodyA = Entity;
                                Contact->BodyB = TestEntity;
                                Contact->Normal = Result.Normal;
                                Contact->Depth = Result.Depth;
                                Contact->ContactPoints = ContactPoints;
                                ModePhysics2->ContactCount++;
                            }
                        }
                    }
                }
            }
        }
        
        for(u32 Index = 0;
            Index < ModePhysics2->ContactCount;
            ++Index)
        {
            ResolveCollisionOptimized(&ModePhysics2->ContactArray[Index]);
        }
    }
    
    // NOTE(ezexff): draw
    for(u32 Index = 0;
        Index < ArrayCount(ModePhysics2->EntityArray);
        ++Index)
    {
        test_entity *Entity = ModePhysics2->EntityArray + Index;
        if(Entity->IsInitialized)
        {
            // NOTE(ezexff):
            {
                rectangle2 Rect = {V2(0, 0), V2((r32)Frame->Dim.x, (r32)Frame->Dim.y)};
                if(!IsInRectangle(Rect, Entity->P))
                {
                    Entity->ddP = V2(0.0f, 0.0f);
                    Entity->dP = V2(0.0f, 0.0f);
                    Entity->IsInitialized = false;
                    ModePhysics2->InitializedEntityCount--;
                    continue;
                }
            }
            
            switch(Entity->Type)
            {
                case TestEntityType_Rect:
                {
                    PushTrianglesOnScreen(&Renderer->PushBufferPhysics, Entity->VertexCount, Entity->TransformedVertexArray, Entity->Color, 10000);
                    PushLinesOnScreen(&Renderer->PushBufferPhysics, Entity->VertexCount, Entity->TransformedVertexArray, 1, Entity->OutlineColor, 10000);
                } break;
                
                case TestEntityType_Circle:
                {
                    PushCircleOnScreen(&Renderer->PushBufferPhysics, Entity->P, Entity->Radius, Entity->Color, 10000);
                    PushCircleOutlineOnScreen(&Renderer->PushBufferPhysics, Entity->P, Entity->Radius, 1, Entity->OutlineColor, 10000);
                } break;
                
                InvalidDefaultCase;
            }
        }
    }
    
    r32 Width = 10.0f;
    v2 WidthDiv2 = V2(Width / 2.0f, Width / 2.0f);
    for(u32 Index = 0;
        Index < ModePhysics2->ContactCount;
        ++Index)
    {
        contact_points CP = ModePhysics2->ContactArray[Index].ContactPoints;
        if(CP.Count == 1)
        {
            rectangle2 Rect = {CP.P1 - WidthDiv2, CP.P1 + WidthDiv2};
            PushRectOnScreen(&Renderer->PushBufferPhysics, Rect.Min, Rect.Max, V4(1, 0.5f, 0, 1), 10000);
        }
        else if(CP.Count == 2)
        {
            rectangle2 Rect1 = {CP.P1 - WidthDiv2, CP.P1 + WidthDiv2};
            PushRectOnScreen(&Renderer->PushBufferPhysics, Rect1.Min, Rect1.Max, V4(1, 0.5f, 0, 1), 10000);
            rectangle2 Rect2 = {CP.P2 - WidthDiv2, CP.P2 + WidthDiv2};
            PushRectOnScreen(&Renderer->PushBufferPhysics, Rect2.Min, Rect2.Max, V4(1, 0.5f, 0, 1), 10000);
        }
        /* 
                else
                {
                    InvalidCodePath
                }
         */
    }
}