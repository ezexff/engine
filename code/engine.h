#include <windows.h>
#include <gl/gl.h>

#include "engine_platform.h"

#include "immintrin.h"
#include "engine_intrinsics.h"
#include "engine_math.h"

#include "engine_memory.h"
#include "engine_asset.h"

#include "engine_renderer.h"

enum game_mode
{
    GameMode_Test,
    GameMode_World,
};

struct world
{
    b32 IsInitialized;
    
    v4 ClearColor;
    
    b32 UseShaderProgram;
    u32 ShaderProgram;
};

struct test
{
    b32 IsInitialized;
    
    v4 ClearColor;
    
    b32 UseShaderProgram;
    u8 ShaderText[10000];
    u32 Shader;
    u32 ShaderProgram;
};

struct game_state
{
    b32 IsInitialized;
    memory_arena ConstArena;
    
    s32 ToneHz;
    s16 ToneVolume;
    r32 tSine;
    u32 SampleIndex;
    
    loaded_sound LoadedSound;
    
    u32 GameMode;
    world World;
    test Test;
};

struct tran_state
{
    b32 IsInitialized;
    memory_arena TranArena;
};

#if ENGINE_INTERNAL
app_log *Log;
#endif