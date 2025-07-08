#include "engine_ui_core.cpp"
#include "engine_ui_widgets.cpp"

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
        
        // NOTE(ezexff): 
        ModeTest->ControlledEntityArray[0].EntityIndex = 0;
        
        ModeTest->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    {
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
        
        /* 
            v2 Offset = V2((r32)Frame->Dim.x / 2, (r32)Frame->Dim.y / 2);
            v2 Dim = V2(50, 50);
            v2 P = (Offset - 0.5f * Dim);
                PushRectOnScreen(&Renderer->PushBufferUI, P, P + Dim, V4(0, 0, 1, 1), 100);
         */
    }
    
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