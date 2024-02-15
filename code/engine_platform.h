#include "engine_types.h"

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
    
    // NOTE(ezexff): ImGui windows visibility
    bool ShowWin32Window;
    bool ShowGameWindow;
    bool ShowDemoWindow;
    
    // NOTE(ezexff): ImGui all windows visibility
    bool ShowImGuiWindows;
    
    // NOTE(ezexff): Log inputs
    bool LogMouseInput;
    bool LogKeyboardInput;
    
    app_log Log;
};
#endif

//~ NOTE(ezexff): Sound
struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;
    s16 *Samples;
};

struct loaded_sound
{
    u32 SampleCount;
    u32 ChannelCount;
    s16 *Samples[2];
    
    void *Free;
};

//~ NOTE(ezexff): Read files
struct platform_file_handle
{
    b32 NoErrors;
};

struct platform_file_group
{
    u32 FileCount;
};

#define PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(name) platform_file_group *name(char *Type)
#define PLATFORM_GET_ALL_FILE_OF_TYPE_END(name) void name(platform_file_group *FileGroup)
#define PLATFORM_OPEN_FILE(name) platform_file_handle *name(platform_file_group *FileGroup)
#define PLATFORM_READ_DATA_FROM_FILE(name) void name(platform_file_handle *Source, u64 Offset, u64 Size, void *Dest)
#define PLATFORM_FILE_ERROR(name) void name(platform_file_handle *Handle, char *Message)
#define PlatformNoFileErrors(Handle) ((Handle)->NoErrors)

typedef PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(platform_get_all_files_of_type_begin);
typedef PLATFORM_GET_ALL_FILE_OF_TYPE_END(platform_get_all_files_of_type_end);
typedef PLATFORM_OPEN_FILE(platform_open_next_file);
typedef PLATFORM_READ_DATA_FROM_FILE(platform_read_data_from_file);
typedef PLATFORM_FILE_ERROR(platform_file_error);

//~ NOTE(ezexff): Platform API
struct platform_api
{
    platform_get_all_files_of_type_begin *GetAllFilesOfTypeBegin;
    platform_get_all_files_of_type_end *GetAllFilesOfTypeEnd;
    platform_open_next_file *OpenNextFile;
    platform_read_data_from_file *ReadDataFromFile;
    platform_file_error *FileError;
};

//~ NOTE(ezexff): Renderer frame with push buffer
struct camera
{
    v3 P;
    v3 Angle; // NOTE(ezexff): Pitch, Yaw, Roll
};

struct renderer_frame
{
    // NOTE(ezexff): Client render area
    s32 Width;
    s32 Height;
    
    u8 PushBufferMemory[65536];
    u32 MaxPushBufferSize;
    u8 *PushBufferBase;
    u32 PushBufferSize;
    
    camera Camera;
    
    u32 MissingResourceCount;
    
#if ENGINE_INTERNAL
    // NOTE(ezexff): OpenGL info
    char *GLVendorStr;
    char *GLRendererStr;
    char *GLVersionStr;
    
    imgui ImGuiHandle;
#endif
};

//~ NOTE(ezexff): Memory
struct game_memory
{
    u64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(ezexff): Clear to zero at startup
    
    u64 TransientStorageSize;
    void *TransientStorage; // NOTE(ezexff): Clear to zero at startup
    
    renderer_frame Frame;
    
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
    
    // TODO(me): переделать?
    game_button_state MouseButtons[PlatformMouseButton_Count];
    //r32 MouseX, MouseY, MouseZ;
    // b32 ShiftDown, AltDown, ControlDown;
    v2s MouseP;
    v3s MouseDelta; // x and y - pos delta, z - scroll delta
    
    //r32 MouseOffsetX, MouseOffsetY;
    // r32 MouseCenterDiffX, MouseCenterDiffY;
    // r32 MouseCenterX, MouseCenterY;
    // b32 ShowMouseCursorMode;
};

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