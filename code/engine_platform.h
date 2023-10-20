/*
  NOTE(casey):

  ENGINE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

  ENGINE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#ifdef __cplusplus
extern "C"
{
#endif

#include "engine_types.h"

    struct app_settings
    {
        r32 MouseSensitivity;

        // Display mode radio buttons state
        b32 RBFullscreenIsActive;
        b32 RBWindowedIsActive;

        s32 NewFrameRate;

        b32 RBCappedIsActive;
        b32 RBUncappedIsActive;

        b32 RBVSyncIsActive;
    };

    //
    // NOTE(me): Buffers
    //
    typedef struct game_offscreen_buffer
    {
        int Width;
        int Height;
        u32 FBO;
        u32 DrawTexture;
        u32 DepthTexture;
        u32 GroundFBO;
    } game_offscreen_buffer;

    //
    // NOTE(me): Inputs
    //
    typedef struct game_button_state
    {
        int HalfTransitionCount;
        bool32 EndedDown;
    } game_button_state;

    typedef struct game_controller_input
    {
        bool32 IsConnected;
        bool32 IsAnalog;
        real32 StickAverageX;
        real32 StickAverageY;

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

                // NOTE(casey): All buttons must be added above this line

                game_button_state Terminator;
            };
        };
    } game_controller_input;

    enum game_input_mouse_button
    {
        PlatformMouseButton_Left,
        PlatformMouseButton_Middle,
        PlatformMouseButton_Right,
        PlatformMouseButton_Extended0,
        PlatformMouseButton_Extended1,

        PlatformMouseButton_Count,
    };
    typedef struct game_input
    {
        real32 dtForFrame;

        game_controller_input Controllers[5];

        // TODO(me): переделать?
        game_button_state MouseButtons[PlatformMouseButton_Count];
        r32 MouseX, MouseY, MouseZ;
        // b32 ShiftDown, AltDown, ControlDown;

        r32 MouseOffsetX, MouseOffsetY;
        // r32 MouseCenterDiffX, MouseCenterDiffY;
        // r32 MouseCenterX, MouseCenterY;
        b32 ShowMouseCursorMode;

        bool32 ExecutableReloaded;

    } game_input;

    inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
    {
        Assert(ControllerIndex < ArrayCount(Input->Controllers));

        game_controller_input *Result = &Input->Controllers[ControllerIndex];
        return (Result);
    }

    //
    // NOTE(me): Platform API
    //
#define PLATFORM_TOGGLE_FULLSCREEN(name) void name(void)
    typedef PLATFORM_TOGGLE_FULLSCREEN(platform_toggle_fullscreen);

#define PLATFORM_SET_FRAMERATE(name) void name(s32 NewFrameRate)
    typedef PLATFORM_SET_FRAMERATE(platform_set_framerate);

#define PLATFORM_TOGGLE_FRAMERATE_CAP(name) void name(void)
    typedef PLATFORM_TOGGLE_FRAMERATE_CAP(platform_toggle_framerate_cap);

#define PLATFORM_TOGGLE_VSYNC(name) void name(void)
    typedef PLATFORM_TOGGLE_VSYNC(platform_toggle_vsync);

    typedef struct platform_api
    {
        platform_toggle_fullscreen *ToggleFullscreen;

        platform_set_framerate *SetFrameRate;

        platform_toggle_framerate_cap *ToggleFrameRateCap;

        platform_toggle_vsync *ToggleVSync;
    } platform_api;

//
// NOTE(me): Performance Counters
//
#if ENGINE_INTERNAL
    enum
    {
        DebugCycleCounter_EngineUpdateAndRender,
        DebugCycleCounter_RenderGroupToOutput,

        DebugCycleCounter_Count,
    };
    typedef struct debug_cycle_counter
    {
        uint64 CycleCount;
        uint32 HitCount;
    } debug_cycle_counter;

    extern struct game_memory *DebugGlobalMemory;

#define BEGIN_TIMED_BLOCK(ID) uint64 StartCycleCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID)                                                                                            \
    DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID;                 \
    ++DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount;
    debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif

    //
    // NOTE(me): Game Memory
    //
    typedef struct game_memory
    {
        uint64 PermanentStorageSize;
        void *PermanentStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup

        uint64 TransientStorageSize;
        void *TransientStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup

        platform_api PlatformAPI;

#if ENGINE_INTERNAL
        debug_cycle_counter Counters[DebugCycleCounter_Count];
#endif

    } game_memory;
    /*
    #define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_offscreen_buffer
    *Buffer) typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
    */

    //
    // NOTE(me): ImGui Log App
    //
    struct app_log
    {
        // Usage:
        // static ExampleAppLog my_log;
        // my_log.AddLog("Hello %d world\n", 123);
        // my_log.Draw("title");
        ImGuiTextBuffer Buf;
        ImGuiTextFilter Filter;
        ImVector<int> LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
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

        void AddLog(const char *fmt, ...) IM_FMTARGS(2)
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

#ifdef __cplusplus
}
#endif
