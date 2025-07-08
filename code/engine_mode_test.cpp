#include "engine_ui_core.cpp"
#include "engine_ui_widgets.cpp"

struct vertices_projection
{
    r32 Min;
    r32 Max;
};

vertices_projection
ProjectVertices(u32 VertexCount, v2 *VertexArray, v2 Axis)
{
    vertices_projection Result = {};
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

b32 IsPolygonsCollide(u32 VertexCountA, v2 *VertexArrayA, u32 VertexCountB, v2 *VertexArrayB)
{
    for(u32 Index = 0;
        Index < VertexCountA;
        ++Index)
    {
        v2 *VA = VertexArrayA + Index;
        v2 *VB = VertexArrayA + (Index + 1) % VertexCountA;
        v2 Edge = *VB - *VA;
        v2 Axis = V2(-Edge.y, Edge.x);
        
        vertices_projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        vertices_projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            return(false);
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
        
        vertices_projection A = ProjectVertices(VertexCountA, VertexArrayA, Axis);
        vertices_projection B = ProjectVertices(VertexCountB, VertexArrayB, Axis);
        
        if(A.Min >= B.Max || B.Min >= A.Max)
        {
            return(false);
        }
    }
    
    return(true);
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
        
        // NOTE(ezexff): init entities
        random_series Series = RandomSeed(0);
        for(u32 Index = 0;
            Index < ArrayCount(ModeTest->EntityArray);
            ++Index)
        {
            test_entity *Entity = ModeTest->EntityArray + Index;
            Entity->P.x = 100 * ((r32)Index + 1);
            Entity->P.y = 500;
            Entity->Radius = 25.0f;
            Entity->Density = 10.0f;
            r32 Depth = 1.0f; // for z implement
            Entity->Mass = Entity->Radius * Depth * Entity->Density; // area * depth * density
            r32 R = RandomUnilateral(&Series);
            r32 G = RandomUnilateral(&Series);
            r32 B = RandomUnilateral(&Series);
            Entity->Color = V4(R, G, B, 1.0f);
        }
        
        // NOTE(ezexff): set controlling entity index
        ModeTest->ControlledEntityArray[0].EntityIndex = 0;
        
        // TODO(ezexff): test rectangles
        for(u32 Index = 0;
            Index < ArrayCount(ModeTest->RectArray);
            ++Index)
        {
            test_rect *Rect = ModeTest->RectArray + Index;
            Rect->P = V2((r32)RandomBetween(&Series, 500, 700), 500);
            Rect->Size = 50.0f;
            Rect->VertexCount = ArrayCount(Rect->VertexArray);
            Rect->VertexArray[0] = V2(-0.5f, -0.5f);
            Rect->VertexArray[1] = V2(0.5f, -0.5f);
            Rect->VertexArray[2] = V2(0.5f, 0.5f);
            Rect->VertexArray[3] = V2(-0.5f, 0.5f);
            r32 R = RandomUnilateral(&Series);
            r32 G = RandomUnilateral(&Series);
            r32 B = RandomUnilateral(&Series);
            Rect->Color = V4(R, G, B, 1.0f);
            Rect->OutlineColor = V4(1, 1, 1, 1);
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
        }
    }
    
    // NOTE(ezexff): entities work
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModeTest->EntityArray);
        ++EntityIndex)
    {
        test_entity *Entity = ModeTest->EntityArray + EntityIndex;
        
        // NOTE(ezexff): physics
        {
            // NOTE(ezexff): move controlled entity
            for(u32 ControlIndex = 0;
                ControlIndex < ArrayCount(ModeTest->ControlledEntityArray);
                ++ControlIndex)
            {
                controlled_entity *ConEntity = ModeTest->ControlledEntityArray + ControlIndex;
                r32 Speed = 500.f;
                if(ConEntity->EntityIndex == EntityIndex)
                {
                    v2 Veloctiy = ConEntity->ddP * Speed * Input->dtForFrame;
                    if(Veloctiy.x != 0 || Veloctiy.y != 0)
                    {
                        int Test = 0;
                    }
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
                    r32 Distance = Length(TestEntity->P - Entity->P);
                    r32 Radii = Entity->Radius + TestEntity->Radius;
                    if(Distance < Radii)
                    {
                        v2 Normal = Normalize(TestEntity->P - Entity->P);
                        r32 Depth = Radii - Distance;
                        Depth /= 2.0f;
                        
                        Entity->P += -Normal * Depth;
                        TestEntity->P += Normal * Depth;
                    }
                }
            }
        }
        
        // NOTE(ezexff): draw
        /* 
                    v2 Offset = Entity->P;
                    v2 Dim = 2 * V2(Entity->Radius, Entity->Radius);
                    v2 P = (Offset - 0.5f * Dim);
                    PushRectOnScreen(&Renderer->PushBufferUI, P, P + Dim, Entity->Color, 10000);
         */
        PushCircleOnScreen(&Renderer->PushBufferUI, Entity->P, Entity->Radius, Entity->Color, 10000);
        PushCircleOutlineOnScreen(&Renderer->PushBufferUI, Entity->P, Entity->Radius, 3, V4(1, 1, 1, 1), 10000);
    }
    
    // NOTE(ezexff): test transform vertices
    v4 WhiteColor = V4(1, 1, 1, 1);
    for(u32 RectIndex = 0;
        RectIndex < ArrayCount(ModeTest->RectArray);
        ++RectIndex)
    {
        // NOTE(ezexff): pre transform work
        test_rect *Rect = ModeTest->RectArray + RectIndex;
        Rect->Angle += Input->dtForFrame;
        if(Rect->Angle > 360){Rect->Angle -= 360;}
        
        // NOTE(ezexff): transform vertices
        m4x4 Model =  Translate(V3(Rect->P.x, Rect->P.y, 1.0f)) * Scale(Rect->Size) * ZRotation(Rect->Angle);
        u32 VertexCount = ArrayCount(Rect->VertexArray);
        for(u32 Index = 0;
            Index < ArrayCount(Rect->VertexArray);
            ++Index)
        {
            v2 *OriginalVertex = Rect->VertexArray + Index;
            v2 *TransformedVertex = Rect->TransformedVertexArray + Index;
            *TransformedVertex = (Model * V4(OriginalVertex->x, OriginalVertex->y, 0, 0)).xy;
        }
        // NOTE(ezexff): move controlled entity
        for(u32 ControlIndex = 0;
            ControlIndex < ArrayCount(ModeTest->ControlledEntityArray);
            ++ControlIndex)
        {
            controlled_entity *ConEntity = ModeTest->ControlledEntityArray + ControlIndex;
            r32 Speed = 500.f;
            if(ConEntity->EntityIndex == RectIndex)
            {
                v2 Veloctiy = ConEntity->ddP * Speed * Input->dtForFrame;
                if(Veloctiy.x != 0 || Veloctiy.y != 0)
                {
                    int Test = 0;
                }
                Rect->P += Veloctiy;
            }
        }
        
        Rect->OutlineColor = WhiteColor;
    }
    
    // NOTE(ezexff): collision detection
    v4 RedColor = V4(1, 0, 0, 1);
    for(u32 EntityIndex = 0;
        EntityIndex < ArrayCount(ModeTest->RectArray);
        ++EntityIndex)
    {
        test_rect *Entity = ModeTest->RectArray + EntityIndex;
        for(u32 TestIndex = 0;
            TestIndex < ArrayCount(ModeTest->RectArray);
            ++TestIndex)
        {
            test_rect *TestEntity = ModeTest->RectArray + TestIndex;
            
            if(EntityIndex != TestIndex)
            {
                if(IsPolygonsCollide(Entity->VertexCount, Entity->TransformedVertexArray,
                                     TestEntity->VertexCount, TestEntity->TransformedVertexArray))
                {
                    Entity->OutlineColor = RedColor;
                    TestEntity->OutlineColor = RedColor;
                }
            }
        }
    }
    
    // NOTE(ezexff): draw
    for(u32 RectIndex = 0;
        RectIndex < ArrayCount(ModeTest->RectArray);
        ++RectIndex)
    {
        test_rect *Rect = ModeTest->RectArray + RectIndex;
        
        u32 VertexCount = ArrayCount(Rect->VertexArray);
        PushTrianglesOnScreen(&Renderer->PushBufferUI, VertexCount, Rect->TransformedVertexArray, Rect->Color, 10000);
        PushLinesOnScreen(&Renderer->PushBufferUI, VertexCount, Rect->TransformedVertexArray, 3, Rect->OutlineColor, 10000);
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