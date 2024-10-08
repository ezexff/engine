void
UpdateAndRenderTest(game_memory *Memory)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    renderer_frame *Frame = &Memory->Frame;
    
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    Renderer->PushBufferUI.SortEntryArray = PushArray(&TranState->TranArena, 10, tile_sort_entry);
    
    v2 Dim1 = V2(1000, 1000);
    v2 Pos1 = V2((r32)Frame->Dim.x - Dim1.x / 2, -Dim1.y / 2);
    PushRectOnScreen(&Renderer->PushBufferUI, Pos1, V2(300, 300), V4(0, 0, 1, 1), 3);
    PushRectOnScreen(&Renderer->PushBufferUI, Pos1, V2(500, 500), V4(0, 1, 0, 1), 2);
    PushRectOnScreen(&Renderer->PushBufferUI, Pos1, Dim1, V4(1, 0, 0, 1), 1);
    
    /* 
        bitmap_id BitmapID = GetFirstBitmapFrom(TranState->Assets, Asset_DuDvMap);
        v2 Dim = V2(1000, 1000);
        v2 Pos = V2((r32)Frame->Dim.x - Dim.x / 2, -Dim.y / 2);
        
        PushBitmapOnScreen(Frame, TranState->Assets, BitmapID, Pos, Dim, 1.0f);
        
        if(Frame->DrawDebugTextLine)
        {
            char *TestString = "The quick brown fox jumps over a lazy dog.";
            DEBUGTextLine(Frame, TranState->Assets, TestString);
        }
     */
    
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