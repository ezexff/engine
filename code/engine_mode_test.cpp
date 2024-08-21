void
UpdateAndRenderTest(game_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    /* 
        renderer_frame *Frame = &Memory->Frame;
        
        mode_test *ModeTest = &GameState->ModeTest;
        
        memory_arena *ConstArena = &GameState->ConstArena;
        
        if(!ModeTest->IsInitialized)
        {
            ModeTest->ClearColor = {0, 0, 0, 1};
            
            ModeTest->IsInitialized = true;
        }
        
        Clear(Frame, ModeTest->ClearColor);
        PushRectOnScreen(Frame, V2(0, 0), V2(1000, 50), V4(1, 0, 0, 1));
        
    #if ENGINE_INTERNAL
        imgui *ImGuiHandle = &Memory->ImGuiHandle;
    #endif
     */
}