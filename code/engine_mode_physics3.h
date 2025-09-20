struct mode_physics3
{
    b32 IsInitialized;
    
    test_entity EntityArray[ENTITY_COUNT_MAX];
    r32 ExplodeRadius;;
    
    //controlled_entity ControlledEntityArray[ArrayCount(((game_input *)0)->Controllers)];
    
    //test_entity EntityArray[ENTITY_COUNT_MAX];
    
    //u32 ContactCount;
    //test_contact ContactArray[ENTITY_COUNT_MAX];
    
    //u32 ContactPairCount;
    //contact_pair ContactPairArray[ENTITY_COUNT_MAX * ENTITY_COUNT_MAX];
    
    //random_series Series;
    
    //s32 InitializedEntityCount;
    /* 
        v4 ClearColor;
        
        opengl_program FrameProgram;
        opengl_shader FrameVert;
        opengl_shader FrameFrag;
     */
};