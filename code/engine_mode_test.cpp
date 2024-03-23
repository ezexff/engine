void
UpdateAndRenderTest(game_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    renderer_frame *Frame = &Memory->Frame;
    
    mode_test *ModeTest = &GameState->ModeTest;
    camera *Camera = &ModeTest->Camera;
    
    
    
    memory_arena *ConstArena = &GameState->ConstArena;
    
    if(!ModeTest->IsInitialized)
    {
        Camera->P.z = 10.0f;
        
        ModeTest->ClearColor = {0, 0, 0, 1};
        
        ModeTest->IsInitialized = true;
    }
    
    MoveCamera(Frame, *Camera);
    
    //UseProgram(Frame, &Test->FrameProgram);
    Clear(Frame, ModeTest->ClearColor);
    //Clear2(Frame, Test->ClearColor);
    PushRectOnGround(Frame, V3(0, 0, 0), V2(5, 5), V4(1, 0, 0, 1));
    
#if ENGINE_INTERNAL
    imgui *ImGuiHandle = &Frame->ImGuiHandle;
    
#endif
}