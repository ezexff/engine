#include "engine_types.h"
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

//~ NOTE(ezexff): ImGui
#if ENGINE_INTERNAL
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "implot.h"

struct RollingBuffer
{
    float Span;
    ImVector<ImVec2> Data;
    RollingBuffer()
    {
        Span = 10.0f;
        Data.reserve(2000);
    }
    void AddPoint(float x, float y)
    {
        float xmod = fmodf(x, Span);
        if (!Data.empty() && xmod < Data.back().x)
            Data.shrink(0);
        Data.push_back(ImVec2(xmod, y));
    }
};

struct app_log
{
    // Usage:
    // app_log Log;
    // Log.Add("Hello %d world\n", 123);
    // Log.Draw("title");
    ImGuiTextBuffer Buf;
    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets; // Index to lines offset. We maintain this with Add() calls.
    bool AutoScroll;           // Keep scrolling if already at the bottom.
    
    app_log()
    {
        AutoScroll = true;
        Clear();
    }
    
    void Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }
    
    void Add(const char *fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for(int new_size = Buf.size(); old_size < new_size; old_size++)
            if(Buf[old_size] == '\n')
            LineOffsets.push_back(old_size + 1);
    }
    
    void Draw(const char *title, bool *p_open = NULL)
    {
        if(!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }
        
        // Options menu
        if(ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }
        
        // Main window
        if(ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
        
        if(clear)
            Clear();
        if(copy)
            ImGui::LogToClipboard();
        
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        const char *buf = Buf.begin();
        const char *buf_end = Buf.end();
        if(Filter.IsActive())
        {
            // In this example we don't use the clipper when Filter is enabled.
            // This is because we don't have a random access on the result on our filter.
            // A real application processing logs with ten of thousands of entries may want to store the result of
            // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
            for(int line_no = 0; line_no < LineOffsets.Size; line_no++)
            {
                const char *line_start = buf + LineOffsets[line_no];
                const char *line_end =
                (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                if(Filter.PassFilter(line_start, line_end))
                    ImGui::TextUnformatted(line_start, line_end);
            }
        }
        else
        {
            // The simplest and easy way to display the entire buffer:
            //   ImGui::TextUnformatted(buf_begin, buf_end);
            // And it'll just work. TextUnformatted() has specialization for large blob of text and will
            // fast-forward to skip non-visible lines. Here we instead demonstrate using the clipper to only process
            // lines that are within the visible area. If you have tens of thousands of items and their processing
            // cost is non-negligible, coarse clipping them on your side is recommended. Using ImGuiListClipper
            // requires
            // - A) random access into your data
            // - B) items all being the  same height,
            // both of which we can handle since we an array pointing to the beginning of each line of text.
            // When using the filter (in the block of code above) we don't have random access into the data to
            // display anymore, which is why we don't use the clipper. Storing or skimming through the search result
            // would make it possible (and would be recommended if you want to search through tens of thousands of
            // entries).
            ImGuiListClipper clipper;
            clipper.Begin(LineOffsets.Size);
            while(clipper.Step())
            {
                for(int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                {
                    const char *line_start = buf + LineOffsets[line_no];
                    const char *line_end =
                    (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    ImGui::TextUnformatted(line_start, line_end);
                }
            }
            clipper.End();
        }
        ImGui::PopStyleVar();
        
        if(AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);
        
        ImGui::EndChild();
        ImGui::End();
    }
};


struct imgui
{
    ImGuiContext *ContextImGui;
    ImPlotContext *ContextImPlot;
    ImGuiIO *IO;
    
    ImGuiMemAllocFunc AllocFunc;
    ImGuiMemFreeFunc FreeFunc;
    void *UserData;
    
    HDC WGL; // NOTE(ezexff): Need for render imgui
    
    // NOTE(ezexff): ImGui all windows visibility
    bool ShowImGuiWindows;
    
    // NOTE(ezexff): ImGui single window visibility
    bool ShowImGuiDemoWindow;
    bool ShowImPlotDemoWindow;
    bool ShowWin32Window;
    bool ShowGameWindow;
    bool ShowDebugCollationWindow;
    bool ShowLogWindow;
    bool ShowFrameShadersEditorWindow;
    bool ShowBitmapPreviewWindow;
    bool ShowSimRegionWindow;
    
    // NOTE(ezexff): Sim Region borders
    bool DrawCameraBounds;
    bool DrawSimBounds;
    bool DrawSimRegionBounds;
    bool DrawSimRegionUpdatableBounds;
    
    // NOTE(ezexff): Ground
    bool DrawGroundBufferBounds;
    
    // NOTE(ezexff): Sim Region entities
    bool DrawSpaceBounds;
    
    // NOTE(ezexff): Log audio
    bool LogAudio;
    
    // NOTE(ezexff): Log inputs
    bool LogMouseInput;
    bool LogKeyboardInput;
    
    // NOTE(ezexff): Log app
    app_log Log;
    
    // TODO(ezexff): Test rolling plot
    RollingBuffer RData[32];
};
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

//~ TODO(ezexff): Debug collation test
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
struct game_memory;
struct game_input;
struct debug_table;
game_memory *GlobalDebugMemory;
game_input *GlobalDebugInput;
//debug_table *GlobalDebugTable;
extern debug_table *GlobalDebugTable;

struct debug_id
{
    void *Value[2];
};

enum debug_type
{
    DebugType_Unknown,
    
    DebugType_FrameMarker,
    DebugType_BeginBlock,
    DebugType_EndBlock,
    
    DebugType_CounterThreadList,
    //    DebugVariableType_CounterFunctionList,
};

struct debug_event
{
    u64 Clock;
    char *GUID;
    char *BlockName; // TODO(casey): Should we remove BlockName altogether?
    u16 ThreadID;
    u16 CoreIndex;
    u8 Type;
    union
    {
        debug_id DebugID;
        debug_event *Value_debug_event;
        
        r32 Value_r32;
    };
};

/* 
#define MAX_DEBUG_THREAD_COUNT 256
#define MAX_DEBUG_EVENT_ARRAY_COUNT 8
#define MAX_DEBUG_TRANSLATION_UNITS 3
#define MAX_DEBUG_EVENT_COUNT (16 * 65536)
#define MAX_DEBUG_RECORD_COUNT (65536)
 */
struct debug_table
{
    // TODO(casey): No attempt is currently made to ensure that the final
    // debug records being written to the event array actually complete
    // their output prior to the swap of the event array index.    
    u32 CurrentEventArrayIndex;
    // TODO(casey): This could actually be a u32 atomic now, since we
    // only need 1 bit to store which array we're using...
    u64 volatile EventArrayIndex_EventIndex;
    debug_event Events[3][16*65536];
};

#define UniqueFileCounterString__(A, B, C) A "(" #B ")." #C
#define UniqueFileCounterString_(A, B, C) UniqueFileCounterString__(A, B, C)
#define UniqueFileCounterString() UniqueFileCounterString_(__FILE__, __LINE__, __COUNTER__)

#define RecordDebugEvent(EventType, Block)           \
u64 ArrayIndex_EventIndex = AtomicAddU64(&GlobalDebugTable->EventArrayIndex_EventIndex, 1); \
u32 EventIndex = ArrayIndex_EventIndex & 0xFFFFFFFF;            \
Assert(EventIndex < ArrayCount(GlobalDebugTable->Events[0]));   \
debug_event *Event = GlobalDebugTable->Events[ArrayIndex_EventIndex >> 32] + EventIndex; \
Event->Clock = __rdtsc();                       \
Event->Type = (u8)EventType;                                    \
Event->CoreIndex = 0;                                           \
Event->ThreadID = (u16)GetThreadID();                         \
Event->GUID = UniqueFileCounterString(); \
Event->BlockName = Block;                              \

#define FRAME_MARKER(SecondsElapsedInit) \
{ \
int Counter = __COUNTER__; \
RecordDebugEvent(DebugType_FrameMarker, "Frame Marker"); \
Event->Value_r32 = SecondsElapsedInit; \
} 

#define TIMED_BLOCK__(BlockName, Number, ...) timed_block TimedBlock_##Number(__COUNTER__, __FILE__, __LINE__, BlockName, ## __VA_ARGS__)
#define TIMED_BLOCK_(BlockName, Number, ...) TIMED_BLOCK__(BlockName, Number, ## __VA_ARGS__)
#define TIMED_BLOCK(BlockName, ...) TIMED_BLOCK_(#BlockName, __LINE__, ## __VA_ARGS__)
#define TIMED_FUNCTION(...) TIMED_BLOCK_((char *)__FUNCTION__, __LINE__, ## __VA_ARGS__)

#define BEGIN_BLOCK_(Counter, FileNameInit, LineNumberInit, BlockNameInit)          \
{RecordDebugEvent(DebugType_BeginBlock, BlockNameInit);}
#define END_BLOCK_(Counter) \
{ \
RecordDebugEvent(DebugType_EndBlock, "End Block"); \
}

#define BEGIN_BLOCK(Name) \
int Counter_##Name = __COUNTER__;                       \
BEGIN_BLOCK_(Counter_##Name, __FILE__, __LINE__, #Name);

#define END_BLOCK(Name) \
END_BLOCK_(Counter_##Name);

struct timed_block
{
    int Counter;
    
    timed_block(int CounterInit, char *FileName, int LineNumber, char *BlockName, u32 HitCountInit = 1)
    {
        // TODO(casey): Record the hit count value here?
        Counter = CounterInit;
        BEGIN_BLOCK_(Counter, FileName, LineNumber, BlockName);
    }
    
    ~timed_block()
    {
        END_BLOCK_(Counter);
    }
};

#endif

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
    
    u8 PushBufferMemory[65536];
    u32 MaxPushBufferSize;
    u8 *PushBufferBase;
    u32 PushBufferSize;
    
    // NOTE(ezexff): We output rendered scene through ColorTexture and using shaders f or on screen effects
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
    
#if ENGINE_INTERNAL
    b32 IsOpenglImGuiInitialized;
    loaded_bitmap Preview;
    imgui *ImGuiHandle;
    debug_table *DebugTable; // for opengl renderer collation
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
    imgui ImGuiHandle;
    debug_table *DebugTable;
    
    u64 DebugStorageSize;
    void *DebugStorage; // NOTE(ezexff): REQUIRED to be cleared to zero at startup
    //struct debug_state *DebugState;
    //struct memory_arena *ConstArena;
#endif
    
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
    v2s MouseP;
    v3s MouseDelta; // x and y - pos delta, z - scroll delta
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