#define ENGINE_IMGUI 0
#define ENGINE_UI    1

//~ NOTE(ezexff): ImGui
#if ENGINE_IMGUI
#include <stdio.h>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#include "implot.h"

/* 
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
 */

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
    
    /* 
        // TODO(ezexff): Test rolling plot
        RollingBuffer RData[32];
     */
};
#endif

//~ NOTE(ezexff): Debug collation
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
};

struct debug_event
{
    u64 Clock;
    char *GUID;
    u16 ThreadID;
    u16 CoreIndex;
    u8 Type;
    
    r32 Value_r32;
    /* 
        union
        {
            debug_id DebugID;
            debug_event *Value_debug_event;
            
            b32 Value_b32;
            s32 Value_s32;
            u32 Value_u32;
            r32 Value_r32;
            v2 Value_v2;
            v3 Value_v3;
            v4 Value_v4;
            rectangle2 Value_rectangle2;
            rectangle3 Value_rectangle3;
            //memory_arena_p Value_memory_arena_p;
        };
     */
};

struct debug_table
{
    //debug_event EditEvent;
    u32 RecordIncrement;
    // TODO(casey): No attempt is currently made to ensure that the final
    // debug records being written to the event array actually complete
    // their output prior to the swap of the event array index.    
    u32 CurrentEventArrayIndex;
    // TODO(casey): This could actually be a u32 atomic now, since we
    // only need 1 bit to store which array we're using...
    u64 volatile EventArrayIndex_EventIndex;
    debug_event Events[2][16*65536];
};

#define UniqueFileCounterString__(A, B, C, D) A "|" #B "|" #C "|" D
#define UniqueFileCounterString_(A, B, C, D) UniqueFileCounterString__(A, B, C, D)
#define DEBUG_NAME(Name) UniqueFileCounterString_(__FILE__, __LINE__, __COUNTER__, Name)

#define DEBUGSetEventRecording(Enabled) (GlobalDebugTable->RecordIncrement = (Enabled) ? 1 : 0)

#define RecordDebugEvent(EventType, GUIDInit)           \
u64 ArrayIndex_EventIndex = AtomicAddU64(&GlobalDebugTable->EventArrayIndex_EventIndex, GlobalDebugTable->RecordIncrement); \
u32 EventIndex = ArrayIndex_EventIndex & 0xFFFFFFFF;            \
Assert(EventIndex < ArrayCount(GlobalDebugTable->Events[0]));   \
debug_event *Event = GlobalDebugTable->Events[ArrayIndex_EventIndex >> 32] + EventIndex; \
Event->Clock = __rdtsc();                       \
Event->Type = (u8)EventType;                                    \
Event->CoreIndex = 0;                                           \
Event->ThreadID = (u16)GetThreadID();                         \
Event->GUID = GUIDInit;

#define FRAME_MARKER(SecondsElapsedInit) \
{RecordDebugEvent(DebugType_FrameMarker, DEBUG_NAME("Frame Marker")); \
Event->Value_r32 = SecondsElapsedInit;}  

#define TIMED_BLOCK__(GUID, Number, ...) timed_block TimedBlock_##Number(GUID, ## __VA_ARGS__)
#define TIMED_BLOCK_(GUID, Number, ...) TIMED_BLOCK__(GUID, Number, ## __VA_ARGS__)
#define TIMED_BLOCK(Name, ...) TIMED_BLOCK_(DEBUG_NAME(Name), __COUNTER__, ## __VA_ARGS__)
#define TIMED_FUNCTION(...) TIMED_BLOCK_(DEBUG_NAME(__FUNCTION__), ## __VA_ARGS__)

#define BEGIN_BLOCK_(GUID) {RecordDebugEvent(DebugType_BeginBlock, GUID);}
#define END_BLOCK_(GUID) {RecordDebugEvent(DebugType_EndBlock, GUID);}

#define BEGIN_BLOCK(Name) BEGIN_BLOCK_(DEBUG_NAME(Name))
#define END_BLOCK() END_BLOCK_(DEBUG_NAME("END_BLOCK_"))

struct timed_block
{
    timed_block(char *GUID, u32 HitCountInit = 1)
    {
        BEGIN_BLOCK_(GUID);
        // TODO(casey): Record the hit count value here?
    }
    
    ~timed_block()
    {
        END_BLOCK();
    }
};