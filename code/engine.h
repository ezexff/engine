#include <windows.h>
#include <gl/gl.h>

#include "engine_platform.h"

#include "engine_math.h"

#include "engine_memory.h"


#include "engine_asset_file_formats.h"
#include "engine_asset.h"

#include "engine_audio.h"
#include "engine_renderer.h"

#include "engine_world.h"
#include "engine_sim_region.h"
#include "engine_entity.h"

#include "engine_random.h"

#include "engine_mode_test.h"
#include "engine_mode_world.h"

enum game_mode
{
    GameMode_Test,
    GameMode_World,
};

struct game_state
{
    b32 IsInitialized;
    memory_arena ConstArena;
    
    u32 GameMode;
    mode_world ModeWorld;
    mode_test ModeTest;
    
    //~ NOTE(ezexff): Audio
    audio_state AudioState;
    playing_sound *PlayingSound;
#if ENGINE_INTERNAL
    bool IsTestSineWave;
    s32 ToneHz;
    s16 ToneVolume;
    r32 tSine;
    u32 SampleIndex;
#endif
    
    //~ NOTE(ezexff): Sim region
    world *World;
    
    u32 GroundBufferWidth;
    u32 GroundBufferHeight;
    r32 TypicalFloorHeight;
    
    u32 CameraFollowingEntityIndex;
    world_position CameraP;
    r32 CameraPitch;
    r32 CameraYaw;
    r32 CameraRoll;
    
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
    
    // NOTE(ezexff): Terrain parameters
    u32 TilesPerChunkRow;
    r32 MaxTerrainHeight;
};

struct task_with_memory
{
    b32 BeingUsed;
    memory_arena Arena;
    
    temporary_memory MemoryFlush;
};

struct tran_state
{
    b32 IsInitialized;
    memory_arena TranArena;
    
    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;
    task_with_memory Tasks[4];
    
    game_assets *Assets;
    
    // NOTE(ezexff): Terrain
    // NOTE(ezexff): Тиррейн состоит из чанков, а чанки из тайлов
    /*u32 GroundBufferCount;
    ground_buffer *GroundBuffers;
    
    u32 TileCount; // NOTE(ezexff): In 1 terrain chunk row
    r32 TileWidth;
    r32 TileHeight;
    
    u32 ChunkVertexCount;
    v2 *ChunkPositionsXY;
    
    u32 ChunkIndexCount;
    u32 *ChunkIndices;
    
    r32 MaxTerrainHeight;*/
};

internal task_with_memory *
BeginTaskWithMemory(tran_state *TranState)
{
    task_with_memory *FoundTask = 0;
    
    for(u32 TaskIndex = 0;
        TaskIndex < ArrayCount(TranState->Tasks);
        ++TaskIndex)
    {
        task_with_memory *Task = TranState->Tasks + TaskIndex;
        if(!Task->BeingUsed)
        {
            FoundTask = Task;
            Task->BeingUsed = true;
            Task->MemoryFlush = BeginTemporaryMemory(&Task->Arena);
            break;
        }
    }
    
    return(FoundTask);
}

internal void
EndTaskWithMemory(task_with_memory *Task)
{
    EndTemporaryMemory(Task->MemoryFlush);
    
    CompletePreviousWritesBeforeFutureWrites;
    Task->BeingUsed = false;
}

global_variable platform_api Platform;
#if ENGINE_INTERNAL
app_log *Log;
#endif