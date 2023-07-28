#include <windows.h>
// #define GLEW_STATIC
// #include <GL/glew.h>
#include <glad.c>
#include <GLFW/glfw3.h>

#include "engine_platform.h"

#include "engine.cpp"
// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context
// creation, etc.) If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "win32_engine.h"

// TODO(me): запаковать все переменные в win32_state структуру?
global_variable b32 GlobalIsWindowMode = true;
global_variable b32 GlobalShowMouseCursor = true;
global_variable b32 MouseCursorInited = true;
global_variable b32 MousePosChanged = false;

game_controller_input *NewKeyboardController;
global_variable r32 MouseOffsetX, MouseOffsetY;
global_variable r32 MouseLastX, MouseLastY;
global_variable r32 DeltaTime = 0.0f;
global_variable r32 LastTime = 0.0f;

global_variable b32 GlobalUncappedFrameRate = false;
global_variable s32 GlobalMaxFrameRate = 60;
global_variable b32 GlobalIsVSyncEnabled = false;
// game_controller_input *KeyboardController = &Input.Controllers[0];

// #pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and
// compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project should not be affected, as you are
// likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
// #if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
// #pragma comment(lib, "legacy_stdio_definitions")
// #endif

PLATFORM_TOGGLE_FRAMERATE_CAP(PlatformToggleFrameRateCap)
{
    GlobalUncappedFrameRate = !GlobalUncappedFrameRate;   
}

PLATFORM_TOGGLE_VSYNC(PlatformToggleVSync)
{
    GlobalIsVSyncEnabled = !GlobalIsVSyncEnabled;
    glfwSwapInterval(GlobalIsVSyncEnabled);
}

PLATFORM_SET_FRAMERATE(PlatformSetFrameRate)
{
    GlobalMaxFrameRate = NewFrameRate;
}

PLATFORM_TOGGLE_FULLSCREEN(PlatformToggleFullscreen)
{
    GlobalIsWindowMode = !GlobalIsWindowMode;
    if(GlobalIsWindowMode)
    {
        glfwSetWindowMonitor(Window, NULL, 100, 100, 1280, 720, 144);
    }
    else
    {
        GLFWmonitor *Monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *Mode = glfwGetVideoMode(Monitor);
        glfwSetWindowMonitor(Window, Monitor, 0, 0, Mode->width, Mode->height, Mode->refreshRate);
    }
}

internal void GlfwProcessKey(int32 Key, int32 KeyCode, int32 Action, game_button_state *NewState)
{
    if((Key == KeyCode) && (Action == GLFW_PRESS))
    {
        NewState->EndedDown = true;
        ++NewState->HalfTransitionCount;
    }
    else if((Key == KeyCode) && (Action == GLFW_RELEASE))
    {
        NewState->EndedDown = false;
    }
}

internal void CursorPositionCallback(GLFWwindow *Window, double XPos, double YPos)
{
    if(MouseCursorInited)
    {
        MouseLastX = (r32)XPos;
        MouseLastY = (r32)YPos;
        MouseCursorInited = false;
    }

    MouseOffsetX = (r32)XPos - MouseLastX;
    MouseOffsetY = MouseLastY - (r32)YPos;

    MouseLastX = (r32)XPos;
    MouseLastY = (r32)YPos;

    MousePosChanged = true;
}

internal void KeyCallback(GLFWwindow *Window, int Key, int Scancode, int Action, int Mods)
{
    // Game inputs
    GlfwProcessKey(Key, GLFW_KEY_W, Action, &NewKeyboardController->MoveUp);
    GlfwProcessKey(Key, GLFW_KEY_S, Action, &NewKeyboardController->MoveDown);
    GlfwProcessKey(Key, GLFW_KEY_A, Action, &NewKeyboardController->MoveLeft);
    GlfwProcessKey(Key, GLFW_KEY_D, Action, &NewKeyboardController->MoveRight);

    // Dev inputs
#if ENGINE_INTERNAL
    if(Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(Window, GLFW_TRUE);
    }
    if(Key == GLFW_KEY_F1 && Action == GLFW_PRESS)
    {
        PlatformToggleFullscreen(Window);
    }
    if(Key == GLFW_KEY_LEFT_CONTROL && Action == GLFW_PRESS)
    {
        GlobalShowMouseCursor = !GlobalShowMouseCursor;
        if(GlobalShowMouseCursor)
        {
            glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            if(glfwRawMouseMotionSupported())
            {
                glfwSetInputMode(Window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            }
        }
    }
#endif
}

internal void GlfwErrorCallback(int Error, const char *Description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", Error, Description);
}

int main(int, char **)
// int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
    win32_state Win32State = {};

    // Setup window
    glfwSetErrorCallback(GlfwErrorCallback);
    if(!glfwInit())
    {
        InvalidCodePath;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    GLFWwindow *Window = glfwCreateWindow(1280, 720, "Engine window title", NULL, NULL);
    if(Window == NULL)
    {
        InvalidCodePath;
    }
    glfwSetCursorPosCallback(Window, CursorPositionCallback);
    glfwSetKeyCallback(Window, KeyCallback);
    glfwMakeContextCurrent(Window);
    glfwSwapInterval(GlobalIsVSyncEnabled); // Enable vsync

    // Glad
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        // std::cout << "Failed to initialize OpenGL context" << std::endl;
        InvalidCodePath;
    }
    // GLEW
    /*glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if(glewError != GLEW_OK)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }*/

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use
    // ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application
    // (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling
    // ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double
    // backslash \\ !
    // io.Fonts->AddFontDefault();
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL,
    // io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    // Init game memory
#if ENGINE_INTERNAL
    LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
    LPVOID BaseAddress = 0;
#endif

    game_memory GameMemory = {};
    GameMemory.PermanentStorageSize = Megabytes(12);
    GameMemory.TransientStorageSize = Megabytes(1);

#if ENGINE_INTERNAL
    GameMemory.PlatformAPI.ToggleFullscreen = PlatformToggleFullscreen;
    GameMemory.PlatformAPI.SetFrameRate = PlatformSetFrameRate;
    GameMemory.PlatformAPI.ToggleFrameRateCap = PlatformToggleFrameRateCap;
    GameMemory.PlatformAPI.ToggleVSync = PlatformToggleVSync;
#endif

    Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
    Win32State.GameMemoryBlock =
        VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
    GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

    if(GameMemory.PermanentStorage && GameMemory.TransientStorage)
    {
        game_input Input[2] = {}; // KW:INPUT_CONTROLLERS
        game_input *NewInput = &Input[0];
        game_input *OldInput = &Input[1];

        // Main loop
        while(!glfwWindowShouldClose(Window))
        {
            r32 CurrentTime = (r32)glfwGetTime();
            DeltaTime = CurrentTime - LastTime;

            r32 MaximumMS = 1.0f / GlobalMaxFrameRate;
            if(DeltaTime >= MaximumMS || GlobalUncappedFrameRate)
            {
                LastTime = CurrentTime;
                Input->dtForFrame = DeltaTime;

                NewInput->MouseX = MouseLastX;
                NewInput->MouseY = MouseLastY;
                NewInput->MouseZ = 0.0f;
                if(MousePosChanged)
                {
                    NewInput->MouseOffsetX = MouseOffsetX * Input->dtForFrame;
                    NewInput->MouseOffsetY = MouseOffsetY * Input->dtForFrame;
                    MousePosChanged = false;
                }
                else
                {
                    NewInput->MouseOffsetX = 0.0f;
                    NewInput->MouseOffsetY = 0.0f;
                }
                NewInput->ShowMouseCursorMode = GlobalShowMouseCursor;

                game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                NewKeyboardController = GetController(NewInput, 0);
                *NewKeyboardController = {};
                NewKeyboardController->IsConnected = true;
                for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
                {
                    NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                        OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                }

                // Poll and handle events (inputs, window resize, etc.)
                // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use
                // your inputs.
                // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
                // clear/overwrite your copy of the mouse data.
                // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application,
                // or clear/overwrite your copy of the keyboard data. Generally you may always pass all inputs to dear
                // imgui, and hide them from your application based on those two flags.
                glfwPollEvents();

                EngineUpdateAndRender(Window, &GameMemory, NewInput);

                game_input *Temp = NewInput;
                NewInput = OldInput;
                OldInput = Temp;
            }
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(Window);
    glfwTerminate();

    return (0);
}
