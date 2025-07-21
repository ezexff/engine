v2 FindSumVector(u32 VertexCount, v2 *VertexArray)
{
    v2 Result = {};
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        Result.x += VertexArray[Index].x;
        Result.y += VertexArray[Index].y;
    }
    
    Result.x = Result.x / (VertexCount);
    Result.y = Result.y / (VertexCount);
    
    return(Result);
}

intersect_result
IntersectCirclePolygon(v2 CircleCenter, r32 CircleRadius, u32 VertexCount, v2 *VertexArray)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    projection A = {};
    projection B = {};
    v2 Axis = {};
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        v2 *VA = VertexArray + Index;
        v2 *VB = VertexArray + (Index + 1) % VertexCount;
        v2 Edge = *VB - *VA;
        Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        A = ProjectVertices(VertexCount, VertexArray, Axis);
        B = ProjectCircle(CircleCenter, CircleRadius, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    s32 CpIndex = FindClosestPointOnPolygion(CircleCenter, VertexCount, VertexArray);
    v2 Cp = VertexArray[CpIndex];
    Axis = Cp - CircleCenter;
    Axis = Normalize(Axis);
    
    A = ProjectVertices(VertexCount, VertexArray, Axis);
    B = ProjectCircle(CircleCenter, CircleRadius, Axis);
    
    if(A.Min >= B.Max || B.Min >= A.Max)
    {
        Result.IsCollides = false;
        return(Result);
    }
    
    r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
    if(AxisDepth < Result.Depth)
    {
        Result.Depth = AxisDepth;
        Result.Normal = Axis;
    }
    
    Result.IsCollides = true;
    
    v2 PolygonCenter = FindSumVector(VertexCount, VertexArray);
    v2 Direction = PolygonCenter - CircleCenter;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    return(Result);
}

intersect_result
IntersectPolygons(u32 VertexCountA, v2 *VertexArrayA, u32 VertexCountB, v2 *VertexArrayB)
{
    intersect_result Result = {};
    Result.Depth = F32Max;
    
    for(u32 Index = 0;
        Index < VertexCountA;
        ++Index)
    {
        v2 *VA = VertexArrayA + Index;
        v2 *VB = VertexArrayA + (Index + 1) % VertexCountA;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    for(u32 Index = 0;
        Index < VertexCountB;
        ++Index)
    {
        v2 *VA = VertexArrayB + Index;
        v2 *VB = VertexArrayB + (Index + 1) % VertexCountB;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        Axis = Normalize(Axis);
        
        projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            Result.IsCollides = false;
            return(Result);
        }
        
        r32 AxisDepth = Minimum(B.Max - A.Min, A.Max - B.Min);
        if(AxisDepth < Result.Depth)
        {
            Result.Depth = AxisDepth;
            Result.Normal = Axis;
        }
    }
    
    Result.IsCollides = true;
    
    v2 CenterA = FindSumVector(VertexCountA, VertexArrayA);
    v2 CenterB = FindSumVector(VertexCountB, VertexArrayB);
    v2 Direction = CenterB - CenterA;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    
    return(Result);
}

void ResolveCollision(test_entity *A, test_entity *B, v2 Normal, r32 Depth)
{
    v2 RelVel = B->dP - A->dP;
    
    if(Inner(RelVel, Normal) <= 0.0f)
    {
        r32 e = Minimum(A->Restitution, B->Restitution);
        r32 j = -(1.0f + e) * Inner(RelVel, Normal);
        j /= A->InvMass + B->InvMass;
        
        v2 Impulse = j * Normal;
        A->dP += -(Impulse * A->InvMass);
        B->dP += Impulse * B->InvMass;
    }
}

internal void
UpdateAndRenderPhysics1(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_physics1 *ModePhysics1 = &GameState->ModePhysics1;
    if(!ModePhysics1->IsInitialized)
    {
        // NOTE(ezexff): set controlling entity index
        ModePhysics1->ControlledEntityArray[0].EntityIndex = 0;
        
        // NOTE(ezexff): create entities
        random_series Series = RandomSeed(300);
        for(u32 Index = 0;
            Index < ArrayCount(ModePhysics1->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModePhysics1->EntityArray + Index;
            Entity->Type = RandomBetween(&Series, 0, TestEntityType_Count - 1);
            r32 R = RandomUnilateral(&Series);
            r32 G = RandomUnilateral(&Series);
            r32 B = RandomUnilateral(&Series);
            Entity->Color = V4(R, G, B, 1.0f);
            Entity->OutlineColor = V4(1, 1, 1, 1);
            
            // NOTE(ezexff): rect
            Entity->P = V2((r32)(100 + 60 * Index), (r32)RandomBetween(&Series, 500, 600));
            Entity->Size = (r32)RandomBetween(&Series, 40, 60);
            Entity->VertexCount = ArrayCount(Entity->VertexArray);
            Entity->VertexArray[0] = V2(-0.5f, -0.5f);
            Entity->VertexArray[1] = V2(0.5f, -0.5f);
            Entity->VertexArray[2] = V2(0.5f, 0.5f);
            Entity->VertexArray[3] = V2(-0.5f, 0.5f);
            
            // NOTE(ezexff): circle
            //Entity->Radius = (r32)RandomBetween(&Series, 15, 25);
            Entity->Radius = Entity->Size / 2.0f;
            
            //~ NOTE(ezexff): physics
            Entity->IsStatic = RandomBetween(&Series, 0, 1);
            //Entity->ForceMagnitude = (r32)RandomBetween(&Series, 100, 500);
            Entity->ForceMagnitude = 10.0f;
            Entity->Density = 1.2f;
            // NOTE(ezexff): mass = area * depth * density
            Entity->Mass = Entity->Size * Entity->Density;
            if(Entity->IsStatic)
            {
                Entity->Color = RGBA(40, 40, 40, 255);
                Entity->InvMass = 0.0f;
            }
            else
            {
                Entity->InvMass = 1.0f / Entity->Mass;
            }
            Entity->Restitution = 0.5f;
            Entity->Restitution = Clamp01(Entity->Restitution);
        }
        
        ModePhysics1->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_entity *ConEntity = ModePhysics1->ControlledEntityArray + ControllerIndex;
        
        if(ControllerIndex == 0)
        {
            ConEntity->ddP = {};
            
            if(IsDown(Controller->MoveUp))
            {
                ConEntity->ddP.y += -1.0f;
            }
            if(IsDown(Controller->MoveDown))
            {
                ConEntity->ddP.y += 1.0f;
            }
            if(IsDown(Controller->MoveLeft))
            {
                ConEntity->ddP.x += -1.0f;
            }
            if(IsDown(Controller->MoveRight))
            {
                ConEntity->ddP.x += 1.0f;
            }
            
            if(WasPressed(Controller->ActionUp))
            {
                ConEntity->EntityIndex += 1;
                ConEntity->EntityIndex = Clamp(0, ConEntity->EntityIndex, ArrayCount(ModePhysics1->EntityArray) - 1);
            }
            if(WasPressed(Controller->ActionDown))
            {
                ConEntity->EntityIndex -= 1;
                ConEntity->EntityIndex = Clamp(0, ConEntity->EntityIndex, ArrayCount(ModePhysics1->EntityArray) - 1);
            }
            
        }
    }
    
    // NOTE(ezexff): test transform vertices
    v4 WhiteColor = V4(1, 1, 1, 1);
    v4 RedColor = V4(1, 0, 0, 1);
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModePhysics1->EntityArray);
        ++EntityIndex)
    {
        // NOTE(ezexff): pre transform work
        test_entity *Entity = ModePhysics1->EntityArray + EntityIndex;
        if(EntityIndex == 1)
        {
            Entity->Angle += Input->dtForFrame;
            if(Entity->Angle > 360){Entity->Angle -= 360;}
        }
        
        // NOTE(ezexff): transform vertices
        m4x4 Model =  Translate(V3(Entity->P.x, Entity->P.y, 1.0f)) * Scale(Entity->Size) * ZRotation(Entity->Angle);
        u32 VertexCount = ArrayCount(Entity->VertexArray);
        for(u32 Index = 0;
            Index < ArrayCount(Entity->VertexArray);
            ++Index)
        {
            v2 *OriginalVertex = Entity->VertexArray + Index;
            v2 *TransformedVertex = Entity->TransformedVertexArray + Index;
            *TransformedVertex = (Model * V4(OriginalVertex->x, OriginalVertex->y, 0, 0)).xy;
        }
        
        if(Entity->IsStatic)
        {
            Entity->OutlineColor = RedColor;
        }
        else
        {
            Entity->OutlineColor = WhiteColor;
        }
    }
    
    // NOTE(ezexff): move
    r32 dt = Input->dtForFrame;
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModePhysics1->EntityArray);
        ++EntityIndex)
    {
        test_entity *Entity = ModePhysics1->EntityArray + EntityIndex;
        
        // NOTE(ezexff): hero
        for(u32 ControllerIndex = 0;
            ControllerIndex < ArrayCount(ModePhysics1->ControlledEntityArray);
            ++ControllerIndex)
        {
            controlled_entity *ConEntity = ModePhysics1->ControlledEntityArray + ControllerIndex;
            if(ControllerIndex == 0)
            {
                if(ConEntity->EntityIndex == EntityIndex)
                {
                    if(!Entity->IsStatic)
                    {
                        Entity->ddP = ConEntity->ddP;
                    }
                    /* 
                                        Entity->ddP = ConEntity->ddP;
                                        r32 ddPLength = LengthSq(Entity->ddP);
                                        if(ddPLength > 1.0f)
                                        {
                                            Entity->ddP *= (1.0f / SquareRoot(ddPLength));
                                        }
                                        Entity->dP = Entity->ddP * Entity->ForceMagnitude * Input->dtForFrame;
                                        Entity->P += Entity->dP;
                     */
                }
            }
        }
        
        Entity->Force = Entity->ddP * Entity->ForceMagnitude;
        v2 Delta = Entity->Force / Entity->Mass;
        Entity->dP += Delta  * dt;
        Entity->P += Entity->dP;
        Entity->Force = {};
        
        // NOTE(ezexff): collision detection
        for(u32 TestIndex = 0;
            TestIndex < ArrayCount(ModePhysics1->EntityArray);
            ++TestIndex)
        {
            test_entity *TestEntity = ModePhysics1->EntityArray + TestIndex;
            
            if(EntityIndex != TestIndex)
            {
                if(Entity->IsStatic && TestEntity->IsStatic)
                {
                    continue;
                }
                intersect_result Result = {};
                if(Entity->Type == TestEntityType_Rect)
                {
                    if(TestEntity->Type == TestEntityType_Rect)
                    {
                        Result = IntersectPolygons(Entity->VertexCount, Entity->TransformedVertexArray,
                                                   TestEntity->VertexCount, TestEntity->TransformedVertexArray);
                    }
                    else if(TestEntity->Type == TestEntityType_Circle)
                    {
                        Result = IntersectCirclePolygon(TestEntity->P, TestEntity->Radius, Entity->VertexCount, Entity->TransformedVertexArray);
                        Result.Normal = -Result.Normal;
                    }
                }
                else if(Entity->Type == TestEntityType_Circle)
                {
                    if(TestEntity->Type == TestEntityType_Rect)
                    {
                        Result = IntersectCirclePolygon(Entity->P, Entity->Radius, TestEntity->VertexCount, TestEntity->TransformedVertexArray);
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
                    TestEntity->OutlineColor = RedColor;
                    Entity->OutlineColor = RedColor;
                    ResolveCollision(Entity, TestEntity, Result.Normal, Result.Depth);
                }
            }
        }
    }
    
    // NOTE(ezexff): draw
    for(u32 Index = 0;
        Index < ArrayCount(ModePhysics1->EntityArray);
        ++Index)
    {
        test_entity *Entity = ModePhysics1->EntityArray + Index;
        
        // NOTE(ezexff): only for testing
        {
            if(Entity->P.x < Frame->Dim.x){Entity->P.x += Frame->Dim.x;}
            if(Entity->P.x > Frame->Dim.x){Entity->P.x -= Frame->Dim.x;}
            if(Entity->P.y < Frame->Dim.y){Entity->P.y += Frame->Dim.y;}
            if(Entity->P.y > Frame->Dim.y){Entity->P.y -= Frame->Dim.y;}
        }
        
        switch(Entity->Type)
        {
            case TestEntityType_Rect:
            {
                PushTrianglesOnScreen(&Renderer->PushBufferUI, Entity->VertexCount, Entity->TransformedVertexArray, Entity->Color, 10000);
                PushLinesOnScreen(&Renderer->PushBufferUI, Entity->VertexCount, Entity->TransformedVertexArray, 3, Entity->OutlineColor, 10000);
            } break;
            
            case TestEntityType_Circle:
            {
                PushCircleOnScreen(&Renderer->PushBufferUI, Entity->P, Entity->Radius, Entity->Color, 10000);
                PushCircleOutlineOnScreen(&Renderer->PushBufferUI, Entity->P, Entity->Radius, 3, Entity->OutlineColor, 10000);
            } break;
            
            InvalidDefaultCase;
        }
    }
}