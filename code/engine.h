global_variable platform_api Platform;
global_variable app_log Log;

#include "stdlib.h" // for rand

#include "engine_memory.h"
#include "engine_intrinsics.h"
#include "engine_math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "engine_asset.h"
#include "engine_world.h"
#include "engine_sim_region.h"
#include "engine_entity.h"
#include "engine_render.h"
#include "engine_render_group.h"

struct game_debug
{
    // NOTE(me): ImGui vars
    bool DrawSimRegionBounds;
    bool DrawSimRegionUpdatableBounds;
    bool DrawSimChunks;
    bool DrawChunkWhereCamera;

    bool DrawPlayerHitbox;

    bool LogCycleCounters;

    bool ProcessAnimations;

    s32 GroundBufferIndex;
};

struct low_entity
{
    // TODO(casey): It's kind of busted that P's can be invalid here,
    // AND we store whether they would be invalid in the flags field...
    // Can we do something better here?
    world_position P;
    sim_entity Sim;
};

struct controlled_hero
{
    uint32 EntityIndex;

    // NOTE(casey): These are the controller requests for simulation
    v2 ddP;
    v2 dSword;
    real32 dZ;
};

struct pairwise_collision_rule
{
    bool32 CanCollide;
    uint32 StorageIndexA;
    uint32 StorageIndexB;

    pairwise_collision_rule *NextInHash;
};
struct game_state;
internal void AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide);
internal void ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex);

struct ground_buffer
{
    // NOTE(casey): An invalid P tells us that this ground_buffer has not been filled
    world_position P; // NOTE(casey): This is the center of the bitmap
    // u32 Texture;
    // loaded_bitmap Bitmap;
    loaded_texture DrawBuffer;
    loaded_model *TerrainModel;
};

struct game_state
{
    bool32 IsInitialized;
    memory_arena WorldArena;

    world *World;

    r32 TypicalFloorHeight;

    uint32 CameraFollowingEntityIndex;
    world_position CameraP;
    r32 CameraPitch;
    r32 CameraYaw;
    r32 CameraRenderZ;

    // string TestTexture1Name;
    // u32 TestTexture1;
    loaded_texture TestTexture1;

    // real32 MetersToPixels;
    // real32 PixelsToMeters;

    controlled_hero ControlledHeroes[ArrayCount(((game_input *)0)->Controllers)];

    // TODO(casey): Change the name to "stored entity"
    uint32 LowEntityCount;
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

    // TODO(me): переделать? убрать?
    game_debug *Debug;

    // TODO(me): rework below
    render *Render;
    app_settings *Settings;

    // TODO(me): убрать
    entity_player *Player;
    entity_clip *Clip;

    // TODO(me): поменять на u32 EnvObjectsCount; и entity_envobject *EnvObjects;
#define ENV_OBJECTS_MAX 16
    entity_envobject *EnvObjects[ENV_OBJECTS_MAX];

    // TODO(me): поменять как env_objects
#define GRASS_OBJECTS_MAX 16
    entity_grassobject *GrassObjects[GRASS_OBJECTS_MAX];

    // TODO(me): для тестов, убрать
    bool ShowDemoWindow;
    bool ShowAnotherWindow;
};

struct transient_state
{
    bool32 IsInitialized;
    memory_arena TranArena;

    u32 GroundBufferCount;
    ground_buffer *GroundBuffers;

    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;

    // uint32 GroundBufferCount;
    // loaded_bitmap GroundBitmapTemplate;
    // ground_buffer *GroundBuffers;
};

inline low_entity * //
GetLowEntity(game_state *GameState, uint32 Index)
{
    low_entity *Result = 0;

    if((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }

    return (Result);
}

internal void //
LogCycleCounters(game_memory *Memory)
{
#if ENGINE_INTERNAL
    OutputDebugStringA("DEBUG CYCLE COUNTS:\n");
    for(int CounterIndex = 0;                        //
        CounterIndex < ArrayCount(Memory->Counters); //
        ++CounterIndex)
    {
        debug_cycle_counter *Counter = Memory->Counters + CounterIndex;

        if(Counter->HitCount)
        {
            /*char TextBuffer[256];
            _snprintf_s(TextBuffer, sizeof(TextBuffer), "  %d: %I64ucy %uh %I64ucy/h\n", CounterIndex,
                        Counter->CycleCount, Counter->HitCount, Counter->CycleCount / Counter->HitCount);
            OutputDebugStringA(TextBuffer);*/
            Log.AddLog("[cycle_counter] ind=%d cycles=%d hits=%d cph=%d\n", //
                       CounterIndex, Counter->CycleCount, Counter->HitCount, Counter->CycleCount / Counter->HitCount);
            Counter->HitCount = 0;
            Counter->CycleCount = 0;
        }
    }
#endif
}

// game layer iteration
// internal void EngineUpdateAndRender(GLFWwindow *Window, game_memory *Memory, game_input *Input);