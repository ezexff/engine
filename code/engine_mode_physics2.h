struct contact_points
{
    v2 P1;
    v2 P2;
    u32 Count;
};

struct test_contact
{
    test_entity *BodyA;
    test_entity *BodyB;
    v2 Normal;
    r32 Depth;
    contact_points ContactPoints;
};

#define ENTITY_COUNT_MAX 128
struct mode_physics2
{
    b32 IsInitialized;
    
    controlled_entity ControlledEntityArray[ArrayCount(((game_input *)0)->Controllers)];
    
    test_entity EntityArray[ENTITY_COUNT_MAX];
    
    u32 ContactCount;
    test_contact ContactArray[ENTITY_COUNT_MAX];
    
    random_series Series;
    
    s32 InitializedEntityCount;
    /* 
        v4 ClearColor;
        
        opengl_program FrameProgram;
        opengl_shader FrameVert;
        opengl_shader FrameFrag;
     */
};