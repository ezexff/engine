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
        UI_BeginFrame(GameState, TranState, Frame, Input);
        
        local b32 IsWindowVisible = true;
        if(IsWindowVisible)
        {
            UI_BeginWindow("DebugTest", &IsWindowVisible);
            
            for(u32 Index = 0;
                Index < 20;
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
        
        /*local b32 IsDebugVarsWindowVisible = true;
        if(IsDebugVarsWindowVisible)
        {
            UI_BeginWindow("DebugVars", &IsDebugVarsWindowVisible);
            u32 FPS = (u32)(1 / UI_State->Input->dtForFrame);
            UI_Label("FPS = %d", FPS);
            r32 MS = UI_State->Input->dtForFrame * 1000.0f;
            UI_Label("MS = %fms/frame", MS);
            UI_Label("FrameCount = %llu", UI_State->FrameCount);
            
            UI_EndWindow();
        }
 */
        
        UI_EndFrame();
    }
    END_BLOCK();
}