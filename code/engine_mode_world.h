struct controlled_hero
{
    u32 EntityIndex;
    
    // NOTE(casey): These are the controller requests for simulation
    v2 ddP;
    v2 dSword;
    r32 dZ;
};

struct low_entity
{
    // TODO(casey): It's kind of busted that P's can be invalid here,
    // AND we store whether they would be invalid in the flags field...
    // Can we do something better here?
    world_position P;
    sim_entity Sim;
};

struct pairwise_collision_rule
{
    b32 CanCollide;
    u32 StorageIndexA;
    u32 StorageIndexB;
    
    pairwise_collision_rule *NextInHash;
};

struct mode_world
{
    //~ NOTE(ezexff): Pointers to allocated memory
    memory_arena *ConstArena;
    memory_arena *TranArena;
    
    //~ NOTE(ezexff): Sim region
    world *World;
    
    u32 GroundBufferWidth;
    u32 GroundBufferHeight;
    r32 TypicalFloorHeight;
    
    controlled_hero ControlledHeroes[ArrayCount(((game_input *)0)->Controllers)];
    
    u32 LowEntityCount;
    low_entity LowEntities[100000];
    
    // TODO(casey): Must be power of two
    pairwise_collision_rule *CollisionRuleHash[256];
    pairwise_collision_rule *FirstFreeCollisionRule;
    
    sim_entity_collision_volume_group *NullCollision;
    sim_entity_collision_volume_group *SwordCollision;
    sim_entity_collision_volume_group *StairCollision;
    sim_entity_collision_volume_group *PlayerCollision;
    sim_entity_collision_volume_group *MonstarCollision;
    sim_entity_collision_volume_group *FamiliarCollision;
    sim_entity_collision_volume_group *WallCollision;
    sim_entity_collision_volume_group *StandardRoomCollision;
    sim_entity_collision_volume_group *WaterCollision;
    
    // NOTE(ezexff): Terrain parameters
    u32 TilesPerChunkRow;
    r32 MaxTerrainHeight;
    
    camera Camera;
    u32 CameraFollowingEntityIndex;
    
    //~ NOTE(ezexff): Render
    u32 RendererFlags;
    
    //~
    b32 IsInitialized;
};

struct game_state;
internal void AddCollisionRule(mode_world *ModeWorld, u32 StorageIndexA, u32 StorageIndexB, b32 CanCollide);
internal void ClearCollisionRulesFor(mode_world *ModeWorld, u32 StorageIndex);
