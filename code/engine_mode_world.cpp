void
UpdateAndRenderWorld(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    world *World = &GameState->World;
    
    renderer_frame *Frame = &Memory->Frame;
    
    if(!World->IsInitialized)
    {
        World->ClearColor = {0, 0, 1, 1};
        
        World->UseShaderProgram = false;
        World->ShaderProgram = 0;
        
        World->IsInitialized = true;
    }
    
    camera *Camera = &Memory->Frame.Camera;
    
    // NOTE(ezexff): Keyboard input
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        
        if(Controller->MoveUp.EndedDown && Controller->MoveUp.HalfTransitionCount == 1)
        {
            //Log->Add("[input]: MoveUp was pressed\n");
        }
        if(Controller->MoveDown.EndedDown && Controller->MoveDown.HalfTransitionCount == 1)
        {
            //Log->Add("[input]: MoveDown was pressed\n");
        }
        if(Controller->MoveLeft.EndedDown && Controller->MoveLeft.HalfTransitionCount == 1)
        {
            //Log->Add("[input]: MoveLeft was pressed\n");
        }
        if(Controller->MoveRight.EndedDown && Controller->MoveRight.HalfTransitionCount == 1)
        {
            //Log->Add("[input]: MoveRight was pressed\n");
        }
        
        if(Controller->MoveUp.EndedDown)
        {
            Camera->P.y += 0.1f;
        }
        if(Controller->MoveDown.EndedDown)
        {
            Camera->P.y -= 0.1f;
        }
        if(Controller->MoveLeft.EndedDown)
        {
            Camera->P.x -= 0.1f;
        }
        if(Controller->MoveRight.EndedDown)
        {
            Camera->P.x += 0.1f;
        }
        
    }
    
    // NOTE(ezexff): Mouse input
    if(Input->MouseDelta.z != 0)
    {
        Camera->P.z -= Input->MouseDelta.z;
    }
    if(Input->MouseButtons[PlatformMouseButton_Left].EndedDown && Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount == 1)
    {
        int Test = 0;
        //Log->Add("[input]: VK_LBUTTON was pressed\n");
    }
    if(Input->MouseButtons[PlatformMouseButton_Middle].EndedDown && Input->MouseButtons[PlatformMouseButton_Middle].HalfTransitionCount == 1)
    {
        //Log->Add("[input]: VK_MBUTTON was pressed\n");
    }
    if(Input->MouseButtons[PlatformMouseButton_Right].EndedDown && Input->MouseButtons[PlatformMouseButton_Right].HalfTransitionCount == 1)
    {
        //Log->Add("[input]: VK_RBUTTON was pressed\n");
    }
    if(Input->MouseButtons[PlatformMouseButton_Extended0].EndedDown && Input->MouseButtons[PlatformMouseButton_Extended0].HalfTransitionCount == 1)
    {
        //Log->Add("[input]: VK_XBUTTON1 was pressed\n");
    }
    if(Input->MouseButtons[PlatformMouseButton_Extended1].EndedDown && Input->MouseButtons[PlatformMouseButton_Extended1].HalfTransitionCount == 1)
    {
        //Log->Add("[input]: VK_XBUTTON2 was pressed\n");
    }
    
    Frame->Opengl.UseShaderProgram = World->UseShaderProgram;
    Frame->Opengl.ShaderProgram = World->ShaderProgram;
    
    PushClear(&Memory->Frame, World->ClearColor);
    PushRectOnGround(&Memory->Frame, V3(0, 0, 0), V2(5, 5), V4(0, 1, 0, 1));
    PushRectOutlineOnGround(&Memory->Frame, V3 (0, 7, 0), V2(5, 5), V4(0, 0, 1, 1), 0.5f);
}