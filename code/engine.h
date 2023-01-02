/*
    TODO(me): переделать OldKeyboardController?
    TODO(me): поддержка джостика?

*/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

#include "engine_memory.h"
#include "engine_intrinsics.h"
#include "engine_math.h"

#include "engine_entity.h"

//#include "ode/ode.h"
#include "engine_world.h"

// TODO(me): на слой платформы?
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"

global_variable platform_api Platform;

struct game_settings
{
    r32 MouseSensitivity;

    // Display Mode radio buttons state
    b32 RBFullscreenIsActive;
    b32 RBWindowedIsActive;
};

struct game_state
{
    b32 IsInitialized;

    memory_arena WorldArena; // permanent память игры без учёта этой структуры game_state

    game_settings *Settings;

    entity_player *Player;

    // TODO: ODE test
    // m4x4 GeomMatrix;
    // MyObject Object;
    // dWorldID World;
    // dSpaceID Space;
    // dJointGroupID contactgroup;

    // TODO(me): для тестов, убрать
    bool ShowDemoWindow;
    bool ShowAnotherWindow;
};