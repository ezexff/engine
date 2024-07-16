struct mode_test
{
    b32 IsInitialized;
    
    v4 ClearColor;
    
    opengl_program FrameProgram;
    opengl_shader FrameVert;
    opengl_shader FrameFrag;
};