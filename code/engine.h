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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#include "ode/ode.h"
#include "engine_world.h"
#include "engine_asset.h"
#include "engine_entity.h"
#include "engine_render.h"
// TODO(me): на слой платформы?

global_variable platform_api Platform;

struct app_settings
{
    r32 MouseSensitivity;

    // Display Mode radio buttons state
    b32 RBFullscreenIsActive;
    b32 RBWindowedIsActive;
};

struct game_state
{
    b32 IsInitialized;

    memory_arena WorldArena; // постоянная память

    render *Render;

    app_settings *Settings;

    // TODO(me): объединить все сущности и поместить структуру world?
    entity_player *Player;

    entity_envobject *EnvObjects[10];

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