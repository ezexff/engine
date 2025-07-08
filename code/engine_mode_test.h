#include "engine_ui_core.h"
#include "engine_ui_widgets.h"

struct test_entity
{
    v2 P;
    v2 dP;
    v2 ddP;
    
    r32 Density;
    r32 Mass;
    r32 Restitution;
    r32 Radius;
    
    v4 Color;
};

struct controlled_entity
{
    u32 EntityIndex;
    
    v2 ddP;
};

struct test_rect
{
    v2 P;
    r32 Size;
    r32 Angle;
    u32 VertexCount;
    v2 VertexArray[4];
    v2 TransformedVertexArray[4];
    //m4x4 Model;
    v4 Color;
    v4 OutlineColor;
};

struct mode_test
{
    b32 IsInitialized;
    
    controlled_entity ControlledEntityArray[ArrayCount(((game_input *)0)->Controllers)];
    test_entity EntityArray[10];
    
    test_rect RectArray[2];
    /* 
        v4 ClearColor;
        
        opengl_program FrameProgram;
        opengl_shader FrameVert;
        opengl_shader FrameFrag;
     */
};