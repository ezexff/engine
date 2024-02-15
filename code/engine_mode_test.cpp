void
UpdateAndRenderTest(game_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    test *Test = &GameState->Test;
    
    if(!Test->IsInitialized)
    {
        Test->ClearColor = {1, 0, 0, 1};
        
        Test->IsInitialized = true;
    }
    
    PushClear(&Memory->Frame, Test->ClearColor);
}