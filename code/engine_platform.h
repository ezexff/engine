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
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"

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
    ImGuiContext *Context;
    ImGuiIO *IO;
    
    ImGuiMemAllocFunc AllocFunc;
    ImGuiMemFreeFunc FreeFunc;
    void *UserData;
    
    HDC WGL; // NOTE(ezexff): Need for render imgui
    
    // NOTE(ezexff): ImGui all windows visibility
    bool ShowImGuiWindows;
    
    // NOTE(ezexff): ImGui single window visibility
    bool ShowWin32Window;
    bool ShowDemoWindow;
    bool ShowGameWindow;
    bool ShowLogWindow;
    bool ShowFrameShadersEditorWindow;
    bool ShowBitmapPreviewWindow;
    bool ShowSimRegionWindow;
    
    // NOTE(ezexff): Sim Region borders
    bool DrawCameraBounds;
    bool DrawSimBounds;
    bool DrawSimRegionBounds;
    bool DrawSimRegionUpdatableBounds;
    
    // NOTE(ezexff): Sim Region entities
    bool DrawSpaceBounds;
    
    // NOTE(ezexff): Log audio
    bool LogAudio;
    
    // NOTE(ezexff): Log inputs
    bool LogMouseInput;
    bool LogKeyboardInput;
    
    // NOTE(ezexff): Log app
    app_log Log;
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

//~ NOTE(ezexff): Renderer frame with push buffer
struct camera
{
    v3 P;
    v3 Angle; // NOTE(ezexff): Pitch, Yaw, Roll
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
typedef void WINAPI type_glUniform1f(GLint location, GLfloat v0);

// Mipmap for texture
typedef void WINAPI type_glGenerateMipmap(GLenum target);



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
#define GL_CLAMP_TO_EDGE 0x812F
    
#define GL_ARRAY_BUFFER 0x8892
#define GL_STATIC_DRAW 0x88E4
#define GL_ELEMENT_ARRAY_BUFFER 0x8893 
#define GL_FRAMEBUFFER_COMPLETE 0x8CD5
    
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_TEXTURE_WRAP_R 0x8072
    
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
    OpenglFunction(glUniform1f);
    
    // Mipmap for texture
    OpenglFunction(glGenerateMipmap);
    
#if ENGINE_INTERNAL
    // NOTE(ezexff): OpenGL info
    char *GLVendorStr;
    char *GLRendererStr;
    char *GLVersionStr;
#endif
    
#define VERT_POSITION 0
#define VERT_TEXCOORD 1
    //#define VERT_NORMAL    2
    
#define EFFECT_ABBERATION 0
#define EFFECT_BLUR 1
#define EFFECT_EMBOSS 2
#define EFFECT_GRAYSCALE 3
#define EFFECT_INVERSE 4
#define EFFECT_SEPIA 5
#define EFFECT_NORMAL 6
};

struct opengl_shader
{
    u32 ID;
    u8 Text[10000];
};

struct opengl_program
{
    u32 ID;
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
    
    //r32 *PositionsZ;
    vbo_vertex *Vertices;
    //v3*Positions;
    //v3 *Normals;
    //v2 *TexCoords;
};

struct renderer_frame
{
    v3 OffsetP; // TODO(ezexff): World offsetP for entity pos calc
    
    //~ NOTE(ezexff): Frame
    v2s Dim; // NOTE(ezexff): Client render area (window size)
    u8 PushBufferMemory[65536];
    u32 MaxPushBufferSize;
    u8 *PushBufferBase;
    u32 PushBufferSize;
    u32 MissingResourceCount;
    // TODO(ezexff): Mb add FOV?
    
    
    camera Camera;
    r32 CameraZ;
    
    // NOTE(ezexff): We output rendered scene through ColorTexture and using shaders for on screen effects
    u32 ColorTexture;
    u32 DepthTexture; // Need for FBO
    u32 FBO;
    u32 VAO;
    u32 VBO;
    opengl_shader Vert;
    opengl_shader Frag;
    opengl_program Program;
    s32 FragEffect; // NOTE(ezexff): abberation, blur, emboss, grayscale, inverse, sepia, normal
    
    //~ NOTE(ezexff): Need For recompile shaders
    b32 CompileShaders; // TODO(ezexff): Mb replace with IsShadersCompiled
    
    //~ NOTE(ezexff): Clear color
    v3 ClearColor; // TODO(ezexff): Do i need this?
    
    //~ NOTE(ezexff): Skybox
    bool DrawSkybox;
    b32 InitializeSkyboxTexture;
    u32 SkyboxVAO;
    u32 SkyboxVBO;
    loaded_bitmap Skybox[6];
    u32 SkyboxTexture;
    opengl_shader SkyboxVert;
    opengl_shader SkyboxFrag;
    opengl_program SkyboxProgram;
    
    // NOTE(ezexff): Default shaders and program
    opengl_shader DefaultVert;
    opengl_shader DefaultFrag;
    opengl_program DefaultProg;
    
    //~ NOTE(ezexff): Terrain
    // TODO(ezexff): Mb move terrain arrays in GameState
    u32 TerrainVerticesCount;
    vbo_vertex *TerrainVertices;
    u32 TerrainIndicesCount;
    u32 *TerrainIndices;
    
    material TerrainMaterial;
    
    b32 IsTerrainVBOInitialized;
    u32 TerrainVAO;
    u32 TerrainVBO;
    u32 TerrainEBO;
    
    // TODO(ezexff): Mb these variable only for debug in imgui?
    bool DrawTerrain;
    bool IsTerrainInLinePolygonMode;
    bool FixCameraOnTerrain;
    
    //~ NOTE(ezexff): Light
    bool PushBufferWithLight;
    directional_light DirLight;
    u32 PointLightsCount;
    point_light PointLights;
    u32 SpotLightsCount;
    spot_light SpotLights;
    
    //~ NOTE(ezexff): Shadows
    u32 ShadowMapFBO;
    u32 ShadowMap;
    v2s ShadowMapDim;
    r32 ShadowMapSize;
    r32 ShadowMapNearPlane, ShadowMapFarPlane;
    r32 ShadowMapCameraPitch, ShadowMapCameraYaw;
    v3 ShadowMapCameraPos;
    r32 ShadowMapBias;
    opengl_shader ShadowMapVert;
    opengl_shader ShadowMapFrag;
    opengl_program ShadowMapProg;
    
    //~ TODO(ezexff): Move into ImGui?
    bool DrawDebugTextLine;
    
    // NOTE(ezexff): Terrain v2.0
    //b32 GroundBuffersIsInitialized;
    
    u32 GroundBufferCount;
    ground_buffer *GroundBuffers;
    
    u32 TileCount; // NOTE(ezexff): In 1 terrain chunk row
    r32 TileWidth;
    r32 TileHeight;
    
    u32 ChunkVertexCount;
    //v2 *ChunkPositionsXY;
    
    u32 ChunkIndexCount;
    u32 *ChunkIndices;
    
    r32 MaxTerrainHeight;
    
    
    
#if ENGINE_INTERNAL
    // NOTE(ezexff): ImGui bitmap preview window
    loaded_bitmap Preview;
    u32 PreviewTexture;
    imgui ImGuiHandle;
#endif
    
    opengl Opengl;
};

//~ NOTE(ezexff): Memory
struct game_memory
{
    u64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(ezexff): Clear to zero at startup
    
    u64 TransientStorageSize;
    void *TransientStorage; // NOTE(ezexff): Clear to zero at startup
    
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