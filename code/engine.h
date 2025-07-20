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

#include "engine_ui_core.h"
#include "engine_ui_widgets.h"

#include "engine_physics.h"
#include "engine_mode_physics1.h"
#include "engine_mode_physics2.h"
#include "engine_mode_world.h"

enum game_mode
{
    GameMode_Physics1,
    GameMode_Physics2,
    GameMode_World,
};

struct game_state
{
    b32 IsInitialized;
    memory_arena ConstArena;
    
    u32 GameModeID;
    mode_world ModeWorld;
    mode_physics1 ModePhysics1;
    mode_physics2 ModePhysics2;
    
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
    
    // NOTE(ezexff): World gen
    u32 GroundBufferWidth;
    u32 GroundBufferHeight;
    r32 TypicalFloorHeight;
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
    
    u32 MainGenerationID;
    
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

platform_api Platform;

#if ENGINE_INTERNAL
#if ENGINE_IMGUI
app_log *Log;
#endif
#endif