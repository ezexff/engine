internal intersect_result
Collide(test_entity *BodyA, test_entity *BodyB)
{
    intersect_result Result = {};
    if(BodyA->Type == TestEntityType_Rect)
    {
        if(BodyB->Type == TestEntityType_Rect)
        {
            Result = IntersectPolygonsOptimized(BodyA->P, BodyA->VertexCount, BodyA->TransformedVertexArray,
                                                BodyB->P, BodyB->VertexCount, BodyB->TransformedVertexArray);
        }
        else if(BodyB->Type == TestEntityType_Circle)
        {
            Result = IntersectCirclePolygonOptimized(BodyB->P, BodyB->Radius,
                                                     BodyA->P, BodyA->VertexCount, BodyA->TransformedVertexArray);
            Result.Normal = -Result.Normal;
        }
    }
    else if(BodyA->Type == TestEntityType_Circle)
    {
        if(BodyB->Type == TestEntityType_Rect)
        {
            Result = IntersectCirclePolygonOptimized(BodyA->P, BodyA->Radius,
                                                     BodyB->P, BodyB->VertexCount, BodyB->TransformedVertexArray);
        }
        else if(BodyB->Type == TestEntityType_Circle)
        {
            Result = IntersectCircles(BodyA->P, BodyA->Radius, BodyB->P, BodyB->Radius);
        }
    }
    return(Result);
}

internal void
StepBodies(mode_physics2 *ModePhysics, r32 IterationMax, r32 dt, v2 Gravity)
{
    dt /= IterationMax;
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModePhysics->EntityArray);
        ++EntityIndex)
    {
        test_entity *Entity = ModePhysics->EntityArray + EntityIndex;
        if(Entity->IsInitialized)
        {
            if(!Entity->IsStatic)
            {
                Entity->dP += Gravity * dt;
                Entity->P += Entity->dP * dt;
                Entity->Angle += Entity->dPAngular * dt;
            }
        }
    }
}

internal void
BroadPhase(mode_physics2 *ModePhysics, u32 Iteration, u32 IterationMax)
{
    // NOTE(ezexff): collision detection
    ModePhysics->ContactPairCount = 0;
    for(u32 IndexA = 0;
        IndexA < ArrayCount(ModePhysics->EntityArray) - 1;
        ++IndexA)
    {
        test_entity *BodyA = ModePhysics->EntityArray + IndexA;
        if(BodyA->IsInitialized)
        {
            for(u32 IndexB = IndexA + 1;
                IndexB < ArrayCount(ModePhysics->EntityArray);
                ++IndexB)
            {
                test_entity *BodyB = ModePhysics->EntityArray + IndexB;
                if(BodyB->IsInitialized)
                {
                    if(BodyA->IsStatic && BodyB->IsStatic)
                    {
                        continue;
                    }
                    
                    if(!RectanglesIntersect(BodyA->AABB, BodyB->AABB))
                    {
                        continue;
                    }
                    
                    ModePhysics->ContactPairArray[ModePhysics->ContactPairCount].IndexA = IndexA;
                    ModePhysics->ContactPairArray[ModePhysics->ContactPairCount].IndexB = IndexB;
                    ModePhysics->ContactPairCount++;
                    Assert(ModePhysics->ContactPairCount < ENTITY_COUNT_MAX * ENTITY_COUNT_MAX);
                }
            }
        }
    }
}

internal void
NarrowPhase(mode_physics2 *ModePhysics)
{
    for(u32 Index = 0;
        Index < ModePhysics->ContactPairCount;
        ++Index)
    {
        contact_pair Pair = ModePhysics->ContactPairArray[Index];
        test_entity *BodyA = &ModePhysics->EntityArray[Pair.IndexA];
        test_entity *BodyB = &ModePhysics->EntityArray[Pair.IndexB];
        
        intersect_result Intersect = Collide(BodyA, BodyB);
        if(Intersect.IsCollides)
        {
            if(BodyA->IsStatic)
            {
                BodyB->P += Intersect.Normal * Intersect.Depth;
            }
            else if(BodyB->IsStatic)
            {
                BodyA->P += -Intersect.Normal * Intersect.Depth;
            }
            else
            {
                r32 DepthDiv2 = Intersect.Depth / 2.0f;
                BodyA->P += -Intersect.Normal * DepthDiv2;
                BodyB->P += Intersect.Normal * DepthDiv2;
            }
            
            test_contact Contact = {};
            Contact.BodyA = BodyA;
            Contact.BodyB = BodyB;
            Contact.Normal = Intersect.Normal;
            Contact.Depth = Intersect.Depth;
            Contact.ContactPoints = FindContactPoints(BodyA, BodyB);
            //ResolveCollisionBasic(Contact);
            //ResolveCollisionWithRotation(Contact);
            ResolveCollisionWithRotationAndFriction(Contact);
        }
    }
}

inline void
CalcInvMassAndInertia(test_entity *Entity)
{
    if(Entity->IsStatic)
    {
        Entity->InvMass = 0.0f;
        Entity->InvInertia = 0.0f;
    }
    else
    {
        Entity->InvMass = 1.0f / Entity->Mass;
        Entity->InvInertia = 1.0f / Entity->Inertia;
    }
}

internal test_entity *
AddRawEntity(mode_physics2 *ModePhysics, test_entity_type Type, b32 IsStatic,
             v2 P, r32 Angle,
             r32 Density, r32 Restitution,
             v4 Color, v4 OutlineColor)
{
    test_entity *Result = 0;
    Assert(ModePhysics->InitializedEntityCount < ENTITY_COUNT_MAX);
    for(u32 Index = 0;
        Index < ArrayCount(ModePhysics->EntityArray);
        ++Index)
    {
        test_entity *Entity = ModePhysics->EntityArray + Index;
        if(!Entity->IsInitialized)
        {
            Entity->Type = Type;
            Entity->IsStatic = IsStatic;
            
            Entity->P = P;
            Entity->Angle = Angle;
            Entity->Density = Density;
            Entity->Restitution = Restitution;
            Entity->StaticFriction = 0.6f;
            Entity->DynamicFriction = 0.4f;
            
            Entity->Color = Color;
            Entity->OutlineColor = OutlineColor;
            
            Entity->IsInitialized = true;
            ModePhysics->InitializedEntityCount++;
            Result = Entity;
            
            break;
        }
    }
    return(Result);
}

internal void
AddRectEntity(mode_physics2 *ModePhysics, b32 IsStatic,
              v2 P, v2 Dim, r32 Angle,
              r32 Density, r32 Restitution,
              v4 Color, v4 OutlineColor)
{
    test_entity *Entity = AddRawEntity(ModePhysics, TestEntityType_Rect, IsStatic,
                                       P, Angle,
                                       Density, Restitution,
                                       Color, OutlineColor);
    Entity->Dim = Dim;
    r32 Area = Entity->Dim.x * Entity->Dim.y;
    Entity->Mass = Area * Entity->Density;
    Entity->Inertia = (1.0f / 12.0f) * Entity->Mass * (Square(Entity->Dim.x) + Square(Entity->Dim.y));
    CalcInvMassAndInertia(Entity);
    Entity->VertexCount = ArrayCount(Entity->VertexArray);
    Entity->VertexArray[0] = V2(-0.5f, -0.5f);
    Entity->VertexArray[1] = V2(0.5f, -0.5f);
    Entity->VertexArray[2] = V2(0.5f, 0.5f);
    Entity->VertexArray[3] = V2(-0.5f, 0.5f);
}

internal void
AddCircleEntity(mode_physics2 *ModePhysics, b32 IsStatic,
                v2 P, r32 Radius, r32 Angle,
                r32 Density, r32 Restitution,
                v4 Color, v4 OutlineColor)
{
    test_entity *Entity = AddRawEntity(ModePhysics, TestEntityType_Circle, IsStatic,
                                       P, Angle,
                                       Density, Restitution,
                                       Color, OutlineColor);
    Entity->Radius = Radius;
    r32 Area = Entity->Radius * Entity->Radius * Pi32;
    Entity->Mass = Area * Entity->Density;
    Entity->Inertia = (1.0f / 2.0f) * Entity->Mass * Square(Entity->Radius);
    CalcInvMassAndInertia(Entity);
    Entity->LineVertexCount = ArrayCount(Entity->LineVertexArray);
    Entity->LineVertexArray[0] = V2(0.0f, 0.0f);
    Entity->LineVertexArray[1] = V2(1.0f, 0.0f);
}

internal void
UpdateAndRenderPhysics2(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    memory_arena *ConstArena = &GameState->ConstArena;
    memory_arena *TranArena = &TranState->TranArena;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_physics2 *ModePhysics2 = &GameState->ModePhysics2;
    if(!ModePhysics2->IsInitialized)
    {
        ModePhysics2->Series = RandomSeed(200);
        
        AddRectEntity(ModePhysics2, true,
                      V2(1920 / 2, 1080 / 6), V2(1920 * 0.6f, 70), 0.0f,
                      1.0f, 0.5f,
                      V4(0, 1, 0, 1), V4(1, 1, 1, 1));
        
        AddRectEntity(ModePhysics2, true,
                      V2(1920 / 2 - 200, 1080 / 3 + 100), V2(400, 50), Tau32 / -20.0f,
                      1.0f, 0.5f,
                      V4(0.7f, 0.7f, 0.7f, 1), V4(1, 1, 1, 1));
        
        AddRectEntity(ModePhysics2, true,
                      V2(1920 / 2 + 200, 1080 / 2 + 100), V2(400, 50), Tau32 / 20.0f,
                      1.0f, 0.5f,
                      V4(0.7f, 0.0f, 0.0f, 1), V4(1, 1, 1, 1));
        
        ModePhysics2->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
    {
        v2 Dim = V2((r32)RandomBetween(&ModePhysics2->Series, 40, 60),
                    (r32)RandomBetween(&ModePhysics2->Series, 40, 60));
        v4 Color = V4(RandomUnilateral(&ModePhysics2->Series),
                      RandomUnilateral(&ModePhysics2->Series),
                      RandomUnilateral(&ModePhysics2->Series),
                      1.0f);
        AddRectEntity(ModePhysics2, false,
                      V2(Input->MouseP.x, Input->MouseP.y), Dim, 0.0f,
                      2.0f, 0.6f,
                      Color, V4(1, 1, 1, 1));
    }
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
    {
        r32 Radius = (r32)RandomBetween(&ModePhysics2->Series, 20, 30);
        v4 Color = V4(RandomUnilateral(&ModePhysics2->Series),
                      RandomUnilateral(&ModePhysics2->Series),
                      RandomUnilateral(&ModePhysics2->Series),
                      1.0f);
        AddCircleEntity(ModePhysics2, false,
                        V2(Input->MouseP.x, Input->MouseP.y), Radius, 0.0f,
                        2.0f, 0.6f,
                        Color, V4(1, 1, 1, 1));
    }
    
    // NOTE(ezexff): transform vertices
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
                /* 
                                if(EntityIndex == 1)
                                {
                                    Entity->Angle += Input->dtForFrame;
                                    if(Entity->Angle > 360){Entity->Angle -= 360;}
                                }
                 */
                v3 P = V3(Entity->P.x, Entity->P.y, 1.0f);
                v3 Dim = V3(Entity->Dim.x , Entity->Dim.y, 1.0f);
                m4x4 Model =  Translate(P) * ZRotation(Entity->Angle) * Scale(Dim);
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
                
                v3 P = V3(Entity->P.x, Entity->P.y, 1.0f);
                m4x4 Model = Translate(P) * ZRotation(Entity->Angle) * Scale(Entity->Radius);
                u32 VertexCount = ArrayCount(Entity->LineVertexArray);
                for(u32 Index = 0;
                    Index < ArrayCount(Entity->VertexArray);
                    ++Index)
                {
                    v2 *LineOriginalVertex = Entity->LineVertexArray + Index;
                    v2 *LineTransformedVertex = Entity->LineTransformedVertexArray + Index;
                    *LineTransformedVertex = (Model * V4(LineOriginalVertex->x, LineOriginalVertex->y, 0, 0)).xy;
                }
            }
            Entity->AABB = {V2(MinX, MinY), V2(MaxX, MaxY)};
        }
    }
    
    // NOTE(ezexff): clear contact array
    /* 
        ModePhysics2->ContactCount = 0;
        for(u32 Index = 0;
            Index < ModePhysics2->ContactCount;
            ++Index)
        {
            ModePhysics2->ContactArray[Index] = {};
        }
     */
    
    // NOTE(ezexff): move
    u32 IterationMax = 1;
    v2 Gravity = V2(0.0f, -9.81f);
    Gravity *= 100.0f;
    for(u32 Iteration = 0;
        Iteration < IterationMax;
        ++Iteration)
    { 
        StepBodies(ModePhysics2, (r32)IterationMax, Input->dtForFrame, Gravity);
        BroadPhase(ModePhysics2, Iteration, IterationMax);
        NarrowPhase(ModePhysics2);
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
                    Entity->Angle = 0.0f;
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
                    
                    // NOTE(ezexff): line to see circle rotation
                    PushLinesOnScreen(&Renderer->PushBufferPhysics, Entity->LineVertexCount, Entity->LineTransformedVertexArray, 1, Entity->OutlineColor, 10000);
                } break;
                
                InvalidDefaultCase;
            }
        }
    }
    
    /* 
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
        }
     */
}