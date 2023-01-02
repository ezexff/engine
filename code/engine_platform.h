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
        //b32 ShiftDown, AltDown, ControlDown;

        r32 MouseOffsetX, MouseOffsetY;
        //r32 MouseCenterDiffX, MouseCenterDiffY;
        //r32 MouseCenterX, MouseCenterY;
        b32 ShowMouseCursorMode;

        bool32 ExecutableReloaded;

    } game_input;

#define PLATFORM_TOGGLE_FULLSCREEN(name) void name(GLFWwindow *Window)
    typedef PLATFORM_TOGGLE_FULLSCREEN(platform_toggle_fullscreen);

    typedef struct platform_api
    {
        platform_toggle_fullscreen *ToggleFullscreen;
    } platform_api;

    typedef struct game_memory
    {
        uint64 PermanentStorageSize;
        void *PermanentStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup

        uint64 TransientStorageSize;
        void *TransientStorage; // NOTE(casey): REQUIRED to be cleared to zero at startup

        platform_api PlatformAPI;

    } game_memory;
    /*
    #define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_offscreen_buffer
    *Buffer) typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
    */

    inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
    {
        Assert(ControllerIndex < ArrayCount(Input->Controllers));

        game_controller_input *Result = &Input->Controllers[ControllerIndex];
        return (Result);
    }

#ifdef __cplusplus
}
#endif
