#include "engine_ui_core.h"
#include "engine_ui_widgets.h"

/* 
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
 */

struct controlled_entity
{
    u32 EntityIndex;
    v2 ddP;
    //v2 ForceDirection;
};

enum test_entity_type
{
    TestEntityType_Rect,
    TestEntityType_Circle,
    
    TestEntityType_Count,
};

struct test_entity
{
    s32 Type;
    v4 Color;
    v4 OutlineColor;
    //m4x4 Model;
    
    // NOTE(ezexff): move spec
    b32 IsStatic;
    r32 ForceMagnitude;
    v2 Force;
    v2 P;
    v2 dP;
    v2 ddP;
    r32 Density;
    r32 Mass;
    r32 InvMass;
    r32 Restitution;
    
    //~ NOTE(ezexff): entity types
    // NOTE(ezexff): rect
    r32 Size;
    r32 Angle;
    u32 VertexCount;
    v2 VertexArray[4];
    v2 TransformedVertexArray[4];
    
    // NOTE(ezexff): circle
    r32 Radius;
};

struct mode_test
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