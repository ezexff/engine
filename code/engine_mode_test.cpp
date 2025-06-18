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
        
        ModeTest->IsInitialized = true;
    }
    
    // NOTE(ezexff): UI
    BEGIN_BLOCK("UI_TEST");
    {
#if 0
        UI_BeginFrame(GameState, TranState, Frame, Input);
        
        local b32 IsWindowVisible = true;
        if(IsWindowVisible)
        {
            UI_BeginWindow("DebugTest", &IsWindowVisible);
            
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
            
            UI_Label("TestLongStringTestLongStringTestLongStringTestLongStringTestLongStringTestLongString");
            
            UI_EndWindow();
        }
        
        UI_EndFrame();
#endif
    }
    END_BLOCK();
}