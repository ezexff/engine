#include "engine_base.h"
#include <intrin.h>
#include "immintrin.h"
#include "engine_intrinsics.h"

//~ NOTE(ezexff): Functions
inline u32
SafeTruncateUInt64(u64 Value) // Need it for safety reading files
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

// NOTE(ezexff): Multithreading
#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
inline u32 AtomicCompareExchangeUInt32(u32 volatile *Value, u32 New, u32 Expected)
{
    u32 Result = _InterlockedCompareExchange((long *)Value, New, Expected);
    
    return(Result);
}
inline u64 AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = _InterlockedExchange64((__int64 volatile *)Value, New);
    
    return(Result);
}
inline u64 AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE(casey): Returns the original value _prior_ to adding
    u64 Result = _InterlockedExchangeAdd64((__int64 volatile *)Value, Addend);
    
    return(Result);
}    
inline u32 GetThreadID(void)
{
    u8 *ThreadLocalStorage = (u8 *)__readgsqword(0x30);
    u32 ThreadID = *(u32 *)(ThreadLocalStorage + 0x48);
    
    return(ThreadID);
}

#if ENGINE_INTERNAL
#include "engine_debug_interface.h"
#else
#define TIMED_FUNCTION()
#define BEGIN_BLOCK()
#define END_BLOCK()
#define DEBUGSetEventRecording()
#define FRAME_MARKER()
#endif

//~ NOTE(ezexff): Audio
struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    s16 *Samples;
};

//~ NOTE(ezexff): Read files
struct platform_file_handle
{
    b32 NoErrors;
    void *Platform;
    
    // TODO(ezexff): Mb rework?
    u64 Size;
    string Name;
};

struct platform_file_group
{
    u32 FileCount;
    void *Platform;
};

enum platform_file_type
{
    PlatformFileType_AssetFile,
    PlatformFileType_SavedGameFile,
    PlatformFileType_VertFile,
    PlatformFileType_FragFile,
    
    PlatformFileType_Count,
};

#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group name(platform_file_type Type)
#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
#define PLATFORM_OPEN_FILE(name) platform_file_handle name(platform_file_group *FileGroup)
#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Source, u64 Offset, u64 Size, void *Dest)
#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)

typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);
typedef PLATFORM_OPEN_FILE(platform_open_next_file);
typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);
typedef PLATFORM_FILE_ERROR(platform_file_error);

struct platform_work_queue;
#define PLATFORM_WORK_QUEUE_CALLBACK(name) void name(platform_work_queue *Queue, void *Data)
typedef PLATFORM_WORK_QUEUE_CALLBACK(platform_work_queue_callback);

typedef void platform_add_entry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data);
typedef void platform_complete_all_work(platform_work_queue *Queue);

#define PLATFORM_ALLOCATE_MEMORY(name) void *name(memory_index Size)
#define PLATFORM_DEALLOCATE_MEMORY(name) void name(void *Memory)

typedef PLATFORM_ALLOCATE_MEMORY(platform_allocate_memory);
typedef PLATFORM_DEALLOCATE_MEMORY(platform_deallocate_memory);

//~ NOTE(ezexff): Platform API
struct platform_api
{
    platform_get_all_files_of_type_begin *GetAllFilesOfTypeBegin;
    platform_get_all_files_of_type_end *GetAllFilesOfTypeEnd;
    platform_open_next_file *OpenNextFile;
    platform_read_data_from_file *ReadDataFromFile;
    platform_file_error *FileError;
    
    platform_add_entry *AddEntry;
    platform_complete_all_work *CompleteAllWork;
    
    platform_allocate_memory *AllocateMemory;
    platform_deallocate_memory *DeallocateMemory;
};

// NOTE(ezexff): Opengl function declarations
typedef char GLchar;

// NOTE(ezexff): WINAPI синоним __stdcall
// Load shader
typedef GLuint WINAPI type_glCreateShader(GLenum type);
typedef void WINAPI type_glShaderSource(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void WINAPI type_glCompileShader(GLuint shader);
typedef void WINAPI type_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef void WINAPI type_glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

// Link shader program
typedef GLuint WINAPI type_glCreateProgram(void);
typedef void WINAPI type_glAttachShader(GLuint program, GLuint shader);
typedef void WINAPI type_glLinkProgram(GLuint program);
typedef void WINAPI type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void WINAPI type_glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);

typedef void WINAPI type_glUseProgram(GLuint program);

typedef void WINAPI type_glDeleteShader (GLuint shader);
typedef void WINAPI type_glDeleteProgram (GLuint program);

// FBO
typedef void WINAPI type_glGenFramebuffers(GLsizei n, GLuint *framebuffers);
typedef void WINAPI type_glBindFramebuffer(GLenum target, GLuint framebuffer);
typedef void WINAPI type_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void WINAPI type_glActiveTexture (GLenum texture);

// VAO, VBO, EBO
typedef ptrdiff_t GLsizeiptr;
typedef void WINAPI type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void WINAPI type_glBindVertexArray(GLuint array);
typedef void WINAPI type_glGenBuffers(GLsizei n, GLuint *buffers);
typedef void WINAPI type_glBindBuffer(GLenum target, GLuint buffer);
typedef void WINAPI type_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void WINAPI type_glEnableVertexAttribArray(GLuint index);
typedef void WINAPI type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);

// Send variable into shader
typedef GLint WINAPI type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef GLenum WINAPI type_glCheckFramebufferStatus(GLenum target);
typedef void WINAPI type_glUniform1i(GLint location, GLint v0);
typedef void WINAPI type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void WINAPI type_glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
typedef void WINAPI type_glUniform1f(GLint location, GLfloat v0);

// Mipmap for texture
typedef void WINAPI type_glGenerateMipmap(GLenum target);

// RBO
typedef void WINAPI type_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void WINAPI type_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void WINAPI type_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void WINAPI type_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);


#define OpenglFunction(Name) type_##Name *Name
struct opengl
{
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
    
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_COMPONENT32 0x81A7
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE1 0x84C1
#define GL_TEXTURE2 0x84C2
#define GL_TEXTURE3 0x84C3
#define GL_TEXTURE4 0x84C4
#define GL_CLAMP_TO_EDGE 0x812F
    
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_ELEMENT_ARRAY_BUFFER 0x8893 
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
    
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_WRAP_R 0x8072
    
#define GL_RENDERBUFFER 0x8D41
#define GL_CLIP_DISTANCE0 0x3000
    
    // Load shader
    OpenglFunction(glCreateShader);
    OpenglFunction(glShaderSource);
    OpenglFunction(glCompileShader);
    OpenglFunction(glGetShaderiv);
    OpenglFunction(glGetShaderInfoLog);
    
    // Link shader program
    OpenglFunction(glCreateProgram);
    OpenglFunction(glAttachShader);
    OpenglFunction(glLinkProgram);
    OpenglFunction(glGetProgramiv);
    OpenglFunction(glGetProgramInfoLog);
    
    OpenglFunction(glUseProgram);
    
    OpenglFunction(glDeleteShader);
    OpenglFunction(glDeleteProgram);
    
    // FBO
    OpenglFunction(glGenFramebuffers);
    OpenglFunction(glBindFramebuffer);
    OpenglFunction(glFramebufferTexture2D);
    OpenglFunction(glActiveTexture);
    
    // VAO, VBO, EBO
    OpenglFunction(glGenVertexArrays);
    OpenglFunction(glBindVertexArray);
    OpenglFunction(glGenBuffers);
    OpenglFunction(glBindBuffer);
    OpenglFunction(glBufferData);
    OpenglFunction(glEnableVertexAttribArray);
    OpenglFunction(glVertexAttribPointer);
    
    // Send variable into shader
    OpenglFunction(glGetUniformLocation);
    OpenglFunction(glCheckFramebufferStatus);
    OpenglFunction(glUniform1i);
    OpenglFunction(glUniformMatrix4fv);
    OpenglFunction(glUniform3fv);
    OpenglFunction(glUniform4fv);
    OpenglFunction(glUniform1f);
    
    // Mipmap for texture
    OpenglFunction(glGenerateMipmap);
    
    // RBO
    OpenglFunction(glGenRenderbuffers);
    OpenglFunction(glBindRenderbuffer);
    OpenglFunction(glRenderbufferStorage);
    OpenglFunction(glFramebufferRenderbuffer);
    
#if ENGINE_INTERNAL
    // NOTE(ezexff): OpenGL info
    char *GLVendorStr;
    char *GLRendererStr;
    char *GLVersionStr;
#endif
};

struct vbo_vertex
{
    v3 Position;
    v3 Normal;
    v2 TexCoords;
};

struct base_light
{
    v3 Color;
    r32 AmbientIntensity;
    r32 DiffuseIntensity;
};

struct directional_light
{
    base_light Base;
    v3 WorldDirection;
};

struct attenuation
{
    r32 Constant;
    r32 Linear;
    r32 Exp;
};

struct point_light
{
    base_light Base;
    v3 WorldPosition;
    attenuation Atten;
};

struct spot_light
{
    point_light Base;
    v3 WorldDirection;
    r32 Cutoff;
};

struct material
{
    v4 Ambient;
    v4 Diffuse;
    v4 Specular;
    v4 Emission;
    r32 Shininess;
};

struct world_position
{
    s32 ChunkX;
    s32 ChunkY;
    s32 ChunkZ;
    
    v3 Offset_;
};

struct ground_buffer
{
    b32 IsInitialized;
    b32 IsFilled;
    
    world_position P;
    
    u32 VAO;
    u32 VBO;
    u32 EBO;
    
    v3 OffsetP; // смещение относительно камеры
    vbo_vertex *Vertices;
};

struct ground_buffer_array
{
    u32 Count;
    ground_buffer *Buffers;
    u32 VertexCount;
};

struct renderer_shaders
{
    loaded_shader FrameVert;
    loaded_shader FrameFrag;
    
    loaded_shader SceneVert;
    loaded_shader SceneFrag;
    
    loaded_shader SkyboxVert;
    loaded_shader SkyboxFrag;
    
    loaded_shader ShadowMapVert;
    loaded_shader ShadowMapFrag;
    
    loaded_shader WaterVert;
    loaded_shader WaterFrag;
};

struct renderer_programs
{
    shader_program Frame;
    shader_program Scene;
    shader_program Skybox;
    shader_program ShadowMap;
    shader_program Water ;
};

struct renderer_frame
{
    v2u Dim; // client window render area
    r32 AspectRatio;
    b32 CompileShaders;
    
    //u8 PushBufferMemory[65536];
    u8 *PushBufferMemory;
    u32 MaxPushBufferSize;
    u8 *PushBufferBase;
    u32 PushBufferSize;
    
    // NOTE(ezexff): We output rendered scene through ColorTexture and using shaders for on screen effects
    u32 ColorTexture;
    u32 DepthTexture; // Need for FBO
    u32 FBO;
    u32 VAO;
    u32 VBO;
    
    s32 EffectID; // fragment shader effect - enum frame_effect
    
    void *Renderer; // pointer to renderer struct object
    
    //~ TODO(ezexff): Mb move into asset system?
    renderer_shaders Shaders;
    renderer_programs Programs;
    
    v3 TestSunRelP; // TODO(ezexff):  remove (water shine)
    
    // TODO(ezexff): Mb these variable only for debug in imgui?
    //bool DrawTerrain;
    bool IsTerrainInLinePolygonMode;
    bool FixCameraOnTerrain;
    
    //~ TODO(ezexff): Move into ImGui?
    bool DrawDebugTextLine;
    
    // TODO(ezexff): Test
    u8 WaterPushBufferMemory[1024];
    u32 WaterMaxPushBufferSize;
    u8 *WaterPushBufferBase;
    u32 WaterPushBufferSize;
    
#if ENGINE_INTERNAL
    debug_table *DebugTable; // for opengl renderer collation
#if ENGINE_IMGUI
    b32 IsOpenglImGuiInitialized;
    loaded_bitmap Preview;
    imgui *ImGuiHandle;
#endif
#endif
    
    opengl Opengl;
};

//~ NOTE(ezexff): Memory
struct game_memory
{
    u64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(ezexff): REQUIRED to be cleared to zero at startup
    
    u64 TransientStorageSize;
    void *TransientStorage; // NOTE(ezexff): REQUIRED to be cleared to zero at startup
    
#if ENGINE_INTERNAL
    struct debug_table *DebugTable;
#if ENGINE_IMGUI
    imgui ImGuiHandle;
#endif
    
    u64 DebugStorageSize;
    void *DebugStorage; // NOTE(ezexff): REQUIRED to be cleared to zero at startup
    
    //struct debug_state *DebugState;
    //struct memory_arena *ConstArena;
#endif
    
    b32 Paused;
    
    renderer_frame Frame;
    
    platform_work_queue *HighPriorityQueue;
    platform_work_queue *LowPriorityQueue;
    
    platform_api PlatformAPI;
};

//~ NOTE(ezexff): Input
struct game_button_state
{
    s32 HalfTransitionCount;
    b32 EndedDown;
};

struct game_controller_input
{
    b32 IsConnected;
    
    union {
        game_button_state Buttons[22];
        struct
        {
            game_button_state Key0;
            game_button_state Key1;
            game_button_state Key2;
            game_button_state Key3;
            game_button_state Key4;
            game_button_state Key5;
            game_button_state Key6;
            game_button_state Key7;
            game_button_state Key8;
            game_button_state Key9;
            
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;
            
            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;
            
            game_button_state LeftShoulder;
            game_button_state RightShoulder;
            
            game_button_state Back;
            game_button_state Start;
            
            // NOTE(ezexff): All buttons must be added above this line
            game_button_state Terminator;
        };
    };
};

enum game_input_mouse_button
{
    PlatformMouseButton_Left,
    PlatformMouseButton_Middle,
    PlatformMouseButton_Right,
    PlatformMouseButton_Extended0,
    PlatformMouseButton_Extended1,
    
    PlatformMouseButton_Count,
};
struct game_input
{
    r32 dtForFrame;
    
    game_controller_input Controllers[5];
    
    game_button_state MouseButtons[PlatformMouseButton_Count];
    v2 MouseP;
    v3 dMouseP; // x and y - pos delta, z - scroll delta
    b32 CenteringMouseCursor;
};

b32 WasPressed(game_button_state State)
{
    b32 Result = ((State.HalfTransitionCount > 1) ||
                  ((State.HalfTransitionCount == 1) && (State.EndedDown)));
    
    return(Result);
}

b32 IsDown(game_button_state State)
{
    b32 Result = (State.EndedDown);
    
    return(Result);
}

inline game_controller_input *
GetController(game_input *Input, u32 ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return (Result);
}

//~ NOTE(ezexff): Export/import engine functions
#define UPDATE_AND_RENDER_FUNC(name) void name(game_memory *Memory, game_input *Input)
#define GET_SOUND_SAMPLES_FUNC(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)

typedef UPDATE_AND_RENDER_FUNC(update_and_render);
typedef GET_SOUND_SAMPLES_FUNC(get_sound_samples);

//~ NOTE(ezexff): Export/import renderer functions
#define RENDERER_BEGIN_FRAME(name) void name(renderer_frame *Frame)
#define RENDERER_END_FRAME(name) void name(renderer_frame *Frame)

typedef RENDERER_BEGIN_FRAME(renderer_begin_frame);
typedef RENDERER_END_FRAME(renderer_end_frame);

//~ NOTE(ezexff): Debug collation at frame end
#define DEBUG_GAME_FRAME_END(name) void name(game_memory *Memory, game_input *Input)
typedef DEBUG_GAME_FRAME_END(debug_game_frame_end);