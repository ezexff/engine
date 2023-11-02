#include <glad.c>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

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

global_variable game_controller_input *NewKeyboardController;
global_variable r32 MouseOffsetX, MouseOffsetY;
global_variable r32 MouseLastX, MouseLastY;
global_variable r64 DeltaTime = 0.0f;
global_variable r64 LastTime = 0.0f;

global_variable b32 GlobalUncappedFrameRate = false;
global_variable s32 GlobalMaxFrameRate = 60;
global_variable b32 GlobalIsVSyncEnabled = false;

global_variable GLFWwindow *GlobalWindow;
global_variable game_offscreen_buffer GlobalBuffer = {};
// game_controller_input *KeyboardController = &Input.Controllers[0];

// #pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and
// compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project should not be affected, as you are
// likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
// #if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
// #pragma comment(lib, "legacy_stdio_definitions")
// #endif

//
// NOTE(me): Settings
//
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
        glfwSetWindowMonitor(GlobalWindow, NULL, 100, 100, 1280, 720, 144);
    }
    else
    {
        GLFWmonitor *Monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode *Mode = glfwGetVideoMode(Monitor);
        glfwSetWindowMonitor(GlobalWindow, Monitor, 0, 0, Mode->width, Mode->height, Mode->refreshRate);
    }
}

//
// NOTE(me): Inputs
//
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
    GlfwProcessKey(Key, GLFW_KEY_SPACE, Action, &NewKeyboardController->Start);
    GlfwProcessKey(Key, GLFW_KEY_UP, Action, &NewKeyboardController->ActionUp);
    GlfwProcessKey(Key, GLFW_KEY_DOWN, Action, &NewKeyboardController->ActionDown);
    GlfwProcessKey(Key, GLFW_KEY_LEFT, Action, &NewKeyboardController->ActionLeft);
    GlfwProcessKey(Key, GLFW_KEY_RIGHT, Action, &NewKeyboardController->ActionRight);

    // Dev inputs
#if ENGINE_INTERNAL
    if(Key == GLFW_KEY_ESCAPE && Action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(Window, GLFW_TRUE);
    }
    if(Key == GLFW_KEY_F1 && Action == GLFW_PRESS)
    {
        PlatformToggleFullscreen();
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

//
// NOTE(me): Threads
//
struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    uint32 volatile CompletionGoal;
    uint32 volatile CompletionCount;

    uint32 volatile NextEntryToWrite;
    uint32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;

    platform_work_queue_entry Entries[256];
};

internal void //
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    uint32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal bool32 //
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    bool32 WeShouldSleep = false;

    uint32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    uint32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        uint32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead, NewNextEntryToRead,
                                                  OriginalNextEntryToRead);
        if(Index == OriginalNextEntryToRead)
        {
            platform_work_queue_entry Entry = Queue->Entries[Index];
            Entry.Callback(Queue, Entry.Data);
            InterlockedIncrement((LONG volatile *)&Queue->CompletionCount);
        }
    }
    else
    {
        WeShouldSleep = true;
    }

    return (WeShouldSleep);
}

internal void //
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }

    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

struct win32_thread_info
{
    int LogicalThreadIndex;
    platform_work_queue *Queue;
};
DWORD WINAPI //
ThreadProc(LPVOID lpParameter)
{
    win32_thread_info *ThreadInfo = (win32_thread_info *)lpParameter;

    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(ThreadInfo->Queue))
        {
            WaitForSingleObjectEx(ThreadInfo->Queue->SemaphoreHandle, INFINITE, FALSE);
        }
    }

    //    return(0);
}

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork)
{
    char Buffer[256];
    wsprintf(Buffer, "Thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
    OutputDebugStringA(Buffer);
}

//
// NOTE(me): Debug
//
internal void //
GlfwErrorCallback(int Error, const char *Description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", Error, Description);
}

//
// NOTE(me): Main
//
int main(int, char **)
// int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
    win32_state Win32State = {};

    win32_thread_info ThreadInfo[7];

    platform_work_queue Queue = {};

    uint32 InitialCount = 0;
    uint32 ThreadCount = ArrayCount(ThreadInfo);
    Queue.SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0;    //
        ThreadIndex < ThreadCount; //
        ++ThreadIndex)
    {
        win32_thread_info *Info = ThreadInfo + ThreadIndex;
        Info->Queue = &Queue;
        Info->LogicalThreadIndex = ThreadIndex;

        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Info, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }

    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A0");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A1");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A2");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A3");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A4");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A5");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A6");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A7");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A8");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work A9");

    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B0");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B1");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B2");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B3");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B4");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B5");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B6");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B7");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B8");
    Win32AddEntry(&Queue, DoWorkerWork, "[thread] test work B9");

    Win32CompleteAllWork(&Queue);

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
    GlobalWindow = Window;
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
    GameMemory.PermanentStorageSize = Megabytes(32);
    GameMemory.TransientStorageSize = Megabytes(32);

    GameMemory.PlatformAPI.ToggleFullscreen = PlatformToggleFullscreen;
    GameMemory.PlatformAPI.SetFrameRate = PlatformSetFrameRate;
    GameMemory.PlatformAPI.ToggleFrameRateCap = PlatformToggleFrameRateCap;
    GameMemory.PlatformAPI.ToggleVSync = PlatformToggleVSync;

    GameMemory.PlatformAPI.HighPriorityQueue = &Queue;
    GameMemory.PlatformAPI.AddEntry = Win32AddEntry;
    GameMemory.PlatformAPI.CompleteAllWork = Win32CompleteAllWork;

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
            r64 CurrentTime = glfwGetTime();
            DeltaTime = CurrentTime - LastTime;

            r64 MaximumMS = 1.0f / GlobalMaxFrameRate;
            if(DeltaTime >= MaximumMS || GlobalUncappedFrameRate)
            {
                LastTime = CurrentTime;
                Input->dtForFrame = (r32)DeltaTime; // TODO(me): r64 Input->dtForFrame?

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

                glfwGetFramebufferSize(Window, &GlobalBuffer.Width, &GlobalBuffer.Height);

                EngineUpdateAndRender(&GameMemory, NewInput, &GlobalBuffer);

                game_input *Temp = NewInput;
                NewInput = OldInput;
                OldInput = Temp;

                glfwSwapBuffers(Window);
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