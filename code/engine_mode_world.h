
struct mode_world
{
    b32 IsInitialized;
};

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
struct game_state;
internal void AddCollisionRule(game_state *GameState, u32 StorageIndexA, u32 StorageIndexB, b32 ShouldCollide);
internal void ClearCollisionRulesFor(game_state *GameState, u32 StorageIndex);

struct ground_buffer
{
    // NOTE(casey): An invalid P tells us that this ground_buffer has not been filled
    world_position P; // NOTE(casey): This is the center of the bitmap
    b32 UpdateVertices;
    //r32 RandomZ;
    // u32 Texture;
    // loaded_bitmap Bitmap;
    //loaded_texture DrawBuffer;
    //loaded_model *TerrainModel;
};