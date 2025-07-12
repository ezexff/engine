#include "engine_ui_core.cpp"
#include "engine_ui_widgets.cpp"

struct projection
{
    r32 Min;
    r32 Max;
};

struct intersect_result
{
    b32 IsCollides;
    v2 Normal;
    r32 Depth;
};

intersect_result IntersectCircles(v2 P, r32 Radius, v2 TestP, r32 TestRadius)
{
    intersect_result Result = {};
    r32 Distance = Length(TestP - P);
    r32 Radii = Radius + TestRadius;
    if(Distance < Radii)
    {
        Result.IsCollides = true;;
        Result.Normal = Normalize(TestP - P);
        Result.Depth = Radii - Distance;
        
    }
    return(Result);
}

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

s32 FindClosestPointOnPolygion(v2 CircleCenter, u32 VertexCount, v2 *VertexArray)
{
    s32 Result = -1;
    r32 MinDistance = F32Max;
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        r32 Distance = Length(VertexArray[Index] - CircleCenter);
        if(Distance < MinDistance)
        {
            MinDistance = Distance;
            Result = Index;
        }
    }
    return(Result);
}

projection
ProjectCircle(v2 Center, r32 Radius, v2 Axis)
{
    projection Result = {};
    
    v2 Direction = Normalize(Axis);
    v2 DirectionAndRadius = Direction * Radius;
    
    v2 P1 = Center - DirectionAndRadius;
    v2 P2 = Center + DirectionAndRadius;
    
    Result.Min = Inner(P1, Axis);
    Result.Max = Inner(P2, Axis);
    
    if(Result.Min > Result.Min)
    {
        r32 Temp = Result.Min;
        Result.Min = Result.Max;
        Result.Max = Temp;
    }
    
    return(Result);
}

projection
ProjectVertices(u32 VertexCount, v2 *VertexArray, v2 Axis)
{
    projection Result = {};
    Result.Min = F32Max;
    Result.Max = F32Min;
    
    for(u32 Index = 0;
        Index < VertexCount;
        ++Index)
    {
        r32 Projection = Inner(VertexArray[Index], Axis);
        if(Projection < Result.Min){ Result.Min = Projection;}
        if(Projection > Result.Max){ Result.Max = Projection;}
    }
    
    return(Result);
}

intersect_result
IntersectCirclePolygon(v2 CircleCenter, r32 CircleRadius, u32 VertexCount, v2 *VertexArray)
{
    intersect_result Result = {};
    //Result.Normal = {};
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
    Result.Depth /= Length(Result.Normal);
    Result.Normal = Normalize(Result.Normal);
    
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
    Result.Depth /= Length(Result.Normal);
    Result.Normal = Normalize(Result.Normal);
    
    v2 CenterA = FindSumVector(VertexCountA, VertexArrayA);
    v2 CenterB = FindSumVector(VertexCountB, VertexArrayB);
    v2 Direction = CenterB - CenterA;
    if(Inner(Direction, Result.Normal) < 0.0f)
    {
        Result.Normal = -Result.Normal;
    }
    
    return(Result);
}

void
UpdateAndRenderTest(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_test *ModeTest = &GameState->ModeTest;
    if(!ModeTest->IsInitialized)
    {
        UI_Init(&GameState->ConstArena, &TranState->TranArena);
        
        // NOTE(ezexff): set controlling entity index
        ModeTest->ControlledEntityArray[0].EntityIndex = 0;
        
        // NOTE(ezexff): create entities
        random_series Series = RandomSeed(0);
        for(u32 Index = 0;
            Index < ArrayCount(ModeTest->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModeTest->EntityArray + Index;
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
            Entity->Radius = (r32)RandomBetween(&Series, 15, 25);
        }
        
        ModeTest->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_entity *ConEntity = ModeTest->ControlledEntityArray + ControllerIndex;
        
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
            }
            if(WasPressed(Controller->ActionDown))
            {
                ConEntity->EntityIndex -= 1;
            }
            
            ConEntity->EntityIndex = Clamp(0, ConEntity->EntityIndex, ArrayCount(ModeTest->EntityArray) - 1);
        }
    }
    
    // NOTE(ezexff): test transform vertices
    v4 WhiteColor = V4(1, 1, 1, 1);
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModeTest->EntityArray);
        ++EntityIndex)
    {
        // NOTE(ezexff): pre transform work
        test_entity *Entity = ModeTest->EntityArray + EntityIndex;
        if(EntityIndex == 5)
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
        
        Entity->OutlineColor = WhiteColor;
    }
    
    // NOTE(ezexff): move
    v4 RedColor = V4(1, 0, 0, 1);
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModeTest->EntityArray);
        ++EntityIndex)
    {
        test_entity *Entity = ModeTest->EntityArray + EntityIndex;
        
        // NOTE(ezexff): hero
        for(u32 ControlIndex = 0;
            ControlIndex < ArrayCount(ModeTest->ControlledEntityArray);
            ++ControlIndex)
        {
            controlled_entity *ConEntity = ModeTest->ControlledEntityArray + ControlIndex;
            r32 Speed = 500.f;
            if(ConEntity->EntityIndex == EntityIndex)
            {
                v2 Veloctiy = ConEntity->ddP * Speed * Input->dtForFrame;
                Entity->P += Veloctiy;
            }
        }
        
        // NOTE(ezexff): collision detection
        for(u32 TestIndex = 0;
            TestIndex < ArrayCount(ModeTest->EntityArray);
            ++TestIndex)
        {
            test_entity *TestEntity = ModeTest->EntityArray + TestIndex;
            
            if(EntityIndex != TestIndex)
            {
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
                    Entity->OutlineColor = RedColor;
                    TestEntity->OutlineColor = RedColor;
                    Result.Depth /= 2.0f;
                    Entity->P += -Result.Normal * Result.Depth;
                    TestEntity->P += Result.Normal * Result.Depth;
                }
            }
        }
    }
    
    // NOTE(ezexff): draw
    for(u32 Index = 0;
        Index < ArrayCount(ModeTest->EntityArray);
        ++Index)
    {
        test_entity *Entity = ModeTest->EntityArray + Index;
        
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
    
    /* 
        v2 Offset = V2((r32)Frame->Dim.x / 2, (r32)Frame->Dim.y / 2);
        v2 Dim = V2(50, 50);
        v2 P = (Offset - 0.5f * Dim);
            PushRectOnScreen(&Renderer->PushBufferUI, P, P + Dim, V4(0, 0, 1, 1), 100);
     */
    
    
    // NOTE(ezexff): UI
    {
#if 0
        BEGIN_BLOCK("UI_TEST");
        UI_BeginFrame(GameState, TranState, Frame, Input);
        
        local b32 IsWindowVisible = true;
        if(IsWindowVisible)
        {
            UI_BeginWindow("DebugTest", &IsWindowVisible);
            
            r32 FPS = 1 / UI_State->Input->dtForFrame;
            UI_Label("FPS = %.2f", FPS);
            
            for(u32 Index = 1;
                Index <= 30;
                ++Index)
            {
                UI_Label("Text%d", Index);
            }
            
            UI_EndWindow();
        }
        
        local b32 IsWindowVisible3 = true;
        if(IsWindowVisible3)
        {
            UI_BeginWindow("DebugTest3", &IsWindowVisible3);
            
            r32 FPS = 1 / UI_State->Input->dtForFrame;
            UI_Label("FPS = %.2f", FPS);
            
            UI_Label("TestLongStringTestLongStringTestLongStringTestLongStringTestLongStringTestLongString");
            
            UI_EndWindow();
        }
        
        UI_EndFrame();
        END_BLOCK();
#endif
    }
}