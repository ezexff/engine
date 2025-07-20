struct mode_physics1
{
    b32 IsInitialized;
    
    controlled_entity ControlledEntityArray[ArrayCount(((game_input *)0)->Controllers)];
    test_entity EntityArray[30];
    /* 
        v4 ClearColor;
        
        opengl_program FrameProgram;
        opengl_shader FrameVert;
        opengl_shader FrameFrag;
     */
};