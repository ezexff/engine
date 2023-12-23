#include <glad.c>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <dsound.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "engine_platform.h"

#include "engine.cpp"
// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context
// creation, etc.) If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "win32_engine.h"

#define DEBUG_AUDIO 0

// TODO(me): запаковать все переменные в win32_state структуру?
global_variable b32 GlobalPause = false;

global_variable b32 GlobalIsWindowMode = true;
global_variable b32 GlobalShowMouseCursor = true;
global_variable b32 MouseCursorInited = true;
global_variable b32 MousePosChanged = false;

global_variable game_controller_input *NewKeyboardController;
global_variable r32 MouseOffsetX, MouseOffsetY;
global_variable r32 MouseLastX, MouseLastY;

// global_variable b32 GlobalUncappedFrameRate = false;
global_variable r32 GlobalGameUpdateHz;
global_variable b32 GlobalIsVSyncEnabled = true;
global_variable b32 GlobalToggleVSync = false;

global_variable GLFWwindow *GlobalWindow;
global_variable game_offscreen_buffer GlobalBuffer = {};
// game_controller_input *KeyboardController = &Input.Controllers[0];

global_variable int64 GlobalPerfCountFrequency;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
// bool32 GlobalIsGameUpdateHzChanged = false;

#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

// #pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and
// compatibility with old VS compilers. To link with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project should not be affected, as you are
// likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
// #if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
// #pragma comment(lib, "legacy_stdio_definitions")
// #endif

//
// NOTE(me): Other
//
internal void //
GlfwErrorCallback(int Error, const char *Description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", Error, Description);
}

//
// NOTE(ezexff): Read files
//

struct win32_platform_file_handle
{
    platform_file_handle H;
    HANDLE Win32Handle;
};

struct win32_platform_file_group
{
    platform_file_group H;
    HANDLE FindHandle;
    WIN32_FIND_DATAA FindData;
};

internal PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(Win32GetAllFilesOfTypeBegin)
{
    // TODO(casey): If we want, someday, make an actual arena used by Win32
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)VirtualAlloc(
                                                                                          0, sizeof(win32_platform_file_group),
                                                                                          MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    char *TypeAt = Type;
    char WildCard[32] = "*.";
    for(u32 WildCardIndex = 2;
        WildCardIndex < sizeof(WildCard);
        ++WildCardIndex)
    {
        WildCard[WildCardIndex] = *TypeAt;
        if(*TypeAt == 0)
        {
            break;
        }
        
        ++TypeAt;
    }
    WildCard[sizeof(WildCard) - 1] = 0;
    
    Win32FileGroup->H.FileCount = 0;
    
    WIN32_FIND_DATAA FindData;
    HANDLE FindHandle = FindFirstFileA(WildCard, &FindData);
    while(FindHandle != INVALID_HANDLE_VALUE)
    {
        ++Win32FileGroup->H.FileCount;
        
        if(!FindNextFileA(FindHandle, &FindData))
        {
            break;
        }
    }
    FindClose(FindHandle);
    
    Win32FileGroup->FindHandle = FindFirstFileA(WildCard, &Win32FileGroup->FindData);
    
    return((platform_file_group *)Win32FileGroup);
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_END(Win32GetAllFilesOfTypeEnd)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup;
    if(Win32FileGroup)
    {
        FindClose(Win32FileGroup->FindHandle);
        
        VirtualFree(Win32FileGroup, 0, MEM_RELEASE);
    }
}

internal PLATFORM_OPEN_FILE(Win32OpenNextFile)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup;
    win32_platform_file_handle *Result = 0;
    
    if(Win32FileGroup->FindHandle != INVALID_HANDLE_VALUE)
    {    
        // TODO(casey): If we want, someday, make an actual arena used by Win32
        Result = (win32_platform_file_handle *)VirtualAlloc(
                                                            0, sizeof(win32_platform_file_handle),
                                                            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        
        if(Result)
        {
            char *FileName = Win32FileGroup->FindData.cFileName;
            Result->Win32Handle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            Result->H.NoErrors = (Result->Win32Handle != INVALID_HANDLE_VALUE);
        }
        
        if(!FindNextFileA(Win32FileGroup->FindHandle, &Win32FileGroup->FindData))
        {
            FindClose(Win32FileGroup->FindHandle);
            Win32FileGroup->FindHandle = INVALID_HANDLE_VALUE;
        }
    }
    
    return((platform_file_handle *)Result);
}

internal PLATFORM_FILE_ERROR(Win32FileError)
{
#if HANDMADE_INTERNAL
    OutputDebugString("WIN32 FILE ERROR: ");
    OutputDebugString(Message);
    OutputDebugString("\n");
#endif
    
    Handle->NoErrors = false;
}

internal PLATFORM_READ_DATA_FROM_FILE(Win32ReadDataFromFile)
{
    if(PlatformNoFileErrors(Source))
    {
        win32_platform_file_handle *Handle = (win32_platform_file_handle *)Source;
        OVERLAPPED Overlapped = {};
        Overlapped.Offset = (u32)((Offset >> 0) & 0xFFFFFFFF);
        Overlapped.OffsetHigh = (u32)((Offset >> 32) & 0xFFFFFFFF);
        
        u32 FileSize32 = SafeTruncateUInt64(Size);
        
        DWORD BytesRead;
        if(ReadFile(Handle->Win32Handle, Dest, FileSize32, &BytesRead, &Overlapped) &&
           (FileSize32 == BytesRead))
        {
            // NOTE(casey): File read succeeded!
        }
        else
        {
            Win32FileError(&Handle->H, "Read file failed.");
        }
    }
}

/*

internal PLATFORM_FILE_ERROR(Win32CloseFile)
{
    CloseHandle(FileHandle);
}

*/

internal r32 //
Win32GetMonitorRefreshHz(HWND Window)
{
    r32 Result;
    
    int MonitorRefreshHz = 60;
    HDC RefreshDC = GetDC(Window);
    int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
    ReleaseDC(Window, RefreshDC);
    if(Win32RefreshRate > 1)
    {
        MonitorRefreshHz = Win32RefreshRate;
    }
    Result = (r32)MonitorRefreshHz;
    
    return (Result);
}

//
// NOTE(me): Timers
//
inline LARGE_INTEGER //
Win32GetWallClock(void)
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return (Result);
}

inline real32 //
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    real32 Result = ((real32)(End.QuadPart - Start.QuadPart) / //
                     (real32)GlobalPerfCountFrequency);
    return (Result);
}

//
// NOTE(me): Sound
//
internal void //
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    // NOTE(casey): Load the library
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    if(DSoundLibrary)
    {
        // NOTE(casey): Get a DirectSound object! - cooperative
        direct_sound_create *DirectSoundCreate =
        (direct_sound_create *)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        
        // TODO(casey): Double-check that this works on XP - DirectSound8 or 7??
        LPDIRECTSOUND DirectSound;
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
            
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                // NOTE(casey): "Create" a primary buffer
                // TODO(casey): DSBCAPS_GLOBALFOCUS?
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    HRESULT Error = PrimaryBuffer->SetFormat(&WaveFormat);
                    if(SUCCEEDED(Error))
                    {
                        // NOTE(casey): We have finally set the format!
                        OutputDebugStringA("Primary buffer format was set.\n");
                    }
                    else
                    {
                        // TODO(casey): Diagnostic
                    }
                }
                else
                {
                    // TODO(casey): Diagnostic
                }
            }
            else
            {
                // TODO(casey): Diagnostic
            }
            
            // TODO(casey): DSBCAPS_GETCURRENTPOSITION2
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error = DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary buffer created successfully.\n");
            }
        }
        else
        {
            // TODO(casey): Diagnostic
        }
    }
    else
    {
        // TODO(casey): Diagnostic
    }
}

internal void //
Win32ClearBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, //
                                             &Region1, &Region1Size,              //
                                             &Region2, &Region2Size,              //
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        uint8 *DestSample = (uint8 *)Region1;
        for(DWORD ByteIndex = 0;     //
            ByteIndex < Region1Size; //
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8 *)Region2;
        for(DWORD ByteIndex = 0;     //
            ByteIndex < Region2Size; //
            ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void //
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD ByteToLock, DWORD BytesToWrite,
                     game_sound_output_buffer *SourceBuffer)
{
    // TODO(casey): More strenuous test!
    VOID *Region1;
    DWORD Region1Size;
    VOID *Region2;
    DWORD Region2Size;
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, //
                                             &Region1, &Region1Size,   //
                                             &Region2, &Region2Size,   //
                                             0)))
    {
        // TODO(casey): assert that Region1Size/Region2Size is valid
        
        // TODO(casey): Collapse these two loops
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *)Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0;            //
            SampleIndex < Region1SampleCount; //
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        DestSample = (int16 *)Region2;
        for(DWORD SampleIndex = 0;            //
            SampleIndex < Region2SampleCount; //
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

#if DEBUG_AUDIO
internal void //
Win32DebugDrawVertical(int X, int Top, int Bottom, v3 Color)
{
    //
    if(Top <= 0)
    {
        Top = 0;
    }
    
    if(Bottom > GlobalBuffer.Height)
    {
        Bottom = GlobalBuffer.Height;
    }
    
    if((X >= 0) && (X < GlobalBuffer.Width))
    {
        r32 MinX = (r32)X;
        r32 MaxX = (r32)(X + 1);
        r32 MinY = (r32)Top;
        r32 MaxY = (r32)(Bottom);
        r32 VRect[] = {
            MinX, MinY, // 0
            MaxX, MinY, // 1
            MaxX, MaxY, // 2
            MinX, MaxY  // 3
        };
        
        r32 Alpha = 0.5f;
        r32 VColor[] = {
            Color.x, Color.y, Color.z, Alpha, // 0
            Color.x, Color.y, Color.z, Alpha, // 1
            Color.x, Color.y, Color.z, Alpha, // 2
            Color.x, Color.y, Color.z, Alpha, // 3
        };
        
        glViewport(0, 0, GlobalBuffer.Width, GlobalBuffer.Height);
        
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, GlobalBuffer.Width, 0, GlobalBuffer.Height, 0, 1);
        glMatrixMode(GL_MODELVIEW);
        
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        
        glVertexPointer(2, GL_FLOAT, 0, VRect);
        glColorPointer(4, GL_FLOAT, 0, VColor);
        glDrawArrays(GL_QUADS, 0, 4);
        
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
    }
}

inline void                                                         //
Win32DrawSoundBufferMarker(win32_sound_output *SoundOutput,         //
                           real32 C, int PadX, int Top, int Bottom, //
                           DWORD Value, v3 Color)
{
    real32 XReal32 = (C * (real32)Value);
    int X = PadX + (int)XReal32;
    Win32DebugDrawVertical(X, Top, Bottom, Color);
}

internal void                                                            //
Win32DebugSyncDisplay(int MarkerCount, win32_debug_time_marker *Markers, //
                      int CurrentMarkerIndex,                            //
                      win32_sound_output *SoundOutput)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    
    s32 PadX = 16;
    s32 PadY = 16;
    
    int LineHeight = 32;
    
    r32 C = (r32)(GlobalBuffer.Width - 2 * PadX) / (r32)SoundOutput->SecondaryBufferSize;
    for(int MarkerIndex = 0;       //
        MarkerIndex < MarkerCount; //
        ++MarkerIndex)
    {
        win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
        Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
        Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);
        
        v3 PlayColor = V3(1, 1, 1);         // white
        v3 WriteColor = V3(1, 0, 0);        // red
        v3 ExpectedFlipColor = V3(1, 1, 0); // yellow
        v3 PlayWindowColor = V3(1, 0, 1);   // purple
        
        int Top = GlobalBuffer.Height - PadY;                 // - MarkerIndex * 5;
        int Bottom = GlobalBuffer.Height - PadY - LineHeight; // - MarkerIndex * 5;
        
        if(MarkerIndex == CurrentMarkerIndex)
        {
            Top -= LineHeight + PadY;
            Bottom -= LineHeight + PadY;
            
            int FirstTop = Top;
            
            // prepare audio for writing
            Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColor);
            Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputWriteCursor, WriteColor);
            
            Top -= LineHeight + PadY;
            Bottom -= LineHeight + PadY;
            
            // ByteToLock & BTL+BytesToWrite
            Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColor);
            Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom,
                                       ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);
            
            Top -= LineHeight + PadY;
            Bottom -= LineHeight + PadY;
            
            // ExpectedFlipPlayCursor
            Win32DrawSoundBufferMarker(SoundOutput, C, PadX, FirstTop, Bottom, ThisMarker->ExpectedFlipPlayCursor,
                                       ExpectedFlipColor);
        }
        
        // previous frames and current frame if(MarkerIndex == CurrentMarkerIndex)
        Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColor);
        Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom,
                                   ThisMarker->FlipPlayCursor + 480 * SoundOutput->BytesPerSample, PlayWindowColor);
        Win32DrawSoundBufferMarker(SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipWriteCursor, WriteColor);
    }
    glDisable(GL_BLEND);
}
#endif

//
// NOTE(me): Settings
//

PLATFORM_TOGGLE_VSYNC(PlatformToggleVSync)
{
    GlobalIsVSyncEnabled = NewIsVSyncEnabled;
    GlobalGameUpdateHz = NewGameUpdateHz;
    GlobalToggleVSync = GlobalIsVSyncEnabled;
    glfwSwapInterval(GlobalIsVSyncEnabled);
}

PLATFORM_TOGGLE_FULLSCREEN(PlatformToggleFullscreen)
{
    GlobalIsWindowMode = !GlobalIsWindowMode;
    if(GlobalIsWindowMode)
    {
        glfwSetWindowMonitor(GlobalWindow, NULL, 100, 100, 1280, 720, GLFW_DONT_CARE);
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
    if(Key == GLFW_KEY_P && Action == GLFW_PRESS)
    {
        GlobalPause = !GlobalPause;
    }
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

DWORD WINAPI //
ThreadProc(LPVOID lpParameter)
{
    platform_work_queue *Queue = (platform_work_queue *)lpParameter;
    
    for(;;)
    {
        if(Win32DoNextWorkQueueEntry(Queue))
        {
            WaitForSingleObjectEx(Queue->SemaphoreHandle, INFINITE, FALSE);
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

internal void //
Win32MakeQueue(platform_work_queue *Queue, u32 ThreadCount)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;
    
    uint32 InitialCount = 0;
    Queue->SemaphoreHandle = CreateSemaphoreEx(0, InitialCount, ThreadCount, 0, 0, SEMAPHORE_ALL_ACCESS);
    for(uint32 ThreadIndex = 0;    //
        ThreadIndex < ThreadCount; //
        ++ThreadIndex)
    {
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Queue, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

//
// NOTE(me): Main
//
int main(int, char **)
// int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
    win32_state Win32State = {};
    
    platform_work_queue HighPriorityQueue = {};
    Win32MakeQueue(&HighPriorityQueue, 6);
    
    platform_work_queue LowPriorityQueue = {};
    Win32MakeQueue(&LowPriorityQueue, 2);
    
#if 0
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
#endif
    
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
    if(Window)
    {
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
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among
        // multiple.
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
        
        // NOTE(me): Init DirectSound
        HWND HWNDWindow = glfwGetWin32Window(Window);
        
        LARGE_INTEGER PerfCountFrequencyResult;
        QueryPerformanceFrequency(&PerfCountFrequencyResult);
        GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
        
        GlobalGameUpdateHz = Win32GetMonitorRefreshHz(HWNDWindow);
        r32 TargetSecondsPerFrame = 1.0f / GlobalGameUpdateHz;
        
        win32_sound_output SoundOutput = {};
        // TODO(casey): Make this like sixty seconds?
        SoundOutput.SamplesPerSecond = 48000;
        SoundOutput.BytesPerSample = sizeof(int16) * 2;
        SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
        // TODO(casey): Actually compute this variance and see what the lowest reasonable value is.
        SoundOutput.SafetyBytes = (int)(((real32)SoundOutput.SamplesPerSecond * //
                                         (real32)SoundOutput.BytesPerSample / GlobalGameUpdateHz) /
                                        3.0f);
        Win32InitDSound(HWNDWindow, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
        Win32ClearBuffer(&SoundOutput);
        GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
        
        int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, //
                                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        
        // NOTE(me): Init game memory
#if ENGINE_INTERNAL
        LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
        LPVOID BaseAddress = 0;
#endif
        
        game_memory GameMemory = {};
        GameMemory.PermanentStorageSize = Megabytes(32);
        GameMemory.TransientStorageSize = Megabytes(128);
        GameMemory.HighPriorityQueue = &HighPriorityQueue;
        GameMemory.LowPriorityQueue = &LowPriorityQueue;
        
        GameMemory.PlatformAPI.ToggleFullscreen = PlatformToggleFullscreen;
        GameMemory.PlatformAPI.ToggleVSync = PlatformToggleVSync;
        
        GameMemory.PlatformAPI.AddEntry = Win32AddEntry;
        GameMemory.PlatformAPI.CompleteAllWork = Win32CompleteAllWork;
        
        GameMemory.PlatformAPI.GetAllFilesOfTypeBegin = Win32GetAllFilesOfTypeBegin;
        GameMemory.PlatformAPI.GetAllFilesOfTypeEnd = Win32GetAllFilesOfTypeEnd;
        GameMemory.PlatformAPI.OpenNextFile = Win32OpenNextFile;
        GameMemory.PlatformAPI.ReadDataFromFile = Win32ReadDataFromFile;
        GameMemory.PlatformAPI.FileError = Win32FileError;
        
        // NOTE(ezexff): Init big memory chunk
        Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
        Win32State.GameMemoryBlock =
            VirtualAlloc(BaseAddress, (size_t)Win32State.TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
        GameMemory.TransientStorage = ((uint8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);
        
        if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
        {
            game_input Input[2] = {}; // KW:INPUT_CONTROLLERS
            game_input *NewInput = &Input[0];
            game_input *OldInput = &Input[1];
            
            // LARGE_INTEGER LastCounter = Win32GetWallClock();
            // LARGE_INTEGER FlipWallClock = Win32GetWallClock();
            r64 FlipWallClock = glfwGetTime();
            
#if DEBUG_AUDIO
            int DebugTimeMarkerIndex = 0;
            win32_debug_time_marker DebugTimeMarkers[30] = {0};
#endif
            
            bool32 SoundIsValid = false;
            
            // NOTE(me): Main loop
            r64 DeltaFrameTime = 0.0f;
            r64 EndFrameTime = 0.0f;
            while(!glfwWindowShouldClose(Window))
            {
                r64 StartFrameTime = glfwGetTime(); // ms
                DeltaFrameTime = StartFrameTime - EndFrameTime;
                
                TargetSecondsPerFrame = 1.0f / GlobalGameUpdateHz;
                
                if(DeltaFrameTime >= TargetSecondsPerFrame)
                {
                    EndFrameTime = StartFrameTime;
                    Input->dtForFrame = (r32)DeltaFrameTime;
                    
                    // NOTE(me): Inputs
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
                    for(int ButtonIndex = 0;                                      //
                        ButtonIndex < ArrayCount(NewKeyboardController->Buttons); //
                        ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                            OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }
                    
                    // Poll and handle events (inputs, window resize, etc.)
                    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to
                    // use your inputs.
                    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or
                    // clear/overwrite your copy of the mouse data.
                    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main
                    // application, or clear/overwrite your copy of the keyboard data. Generally you may always pass all
                    // inputs to dear imgui, and hide them from your application based on those two flags.
                    glfwPollEvents();
                    
                    if(!GlobalPause)
                    {
                        glfwGetFramebufferSize(Window, &GlobalBuffer.Width, &GlobalBuffer.Height);
                        
                        EngineUpdateAndRender(&GameMemory, NewInput, &GlobalBuffer);
                        
                        // LARGE_INTEGER AudioWallClock = Win32GetWallClock();
                        // real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);
                        r64 AudioWallClock = glfwGetTime();
                        r32 FromBeginToAudioSeconds = (r32)(AudioWallClock - FlipWallClock);
                        
                        DWORD PlayCursor;
                        DWORD WriteCursor;
                        if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
                        {
                            /* NOTE(casey):

                               Here is how sound output computation works.

                               We define a safety value that is the number
                               of samples we think our game update loop
                               may vary by (let's say up to 2ms)

                               When we wake up to write audio, we will look
                               and see what the play cursor position is and we
                               will forecast ahead where we think the play
                               cursor will be on the next frame boundary.

                               We will then look to see if the write cursor is
                               before that by at least our safety value.  If
                               it is, the target fill position is that frame
                               boundary plus one frame.  This gives us perfect
                               audio sync in the case of a card that has low
                               enough latency.

                               If the write cursor is _after_ that safety
                               margin, then we assume we can never sync the
                               audio perfectly, so we will write one frame's
                               worth of audio plus the safety margin's worth
                               of guard samples.
                            */
                            if(!SoundIsValid)
                            {
                                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                SoundIsValid = true;
                            }
                            
                            DWORD ByteToLock = ((SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) %
                                                SoundOutput.SecondaryBufferSize);
                            
                            DWORD ExpectedSoundBytesPerFrame =
                            (int)((real32)(SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample) /
                                  GlobalGameUpdateHz);
                            real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
                            DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip / TargetSecondsPerFrame) *
                                                                   (real32)ExpectedSoundBytesPerFrame);
                            
                            DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;
                            
                            DWORD SafeWriteCursor = WriteCursor;
                            if(SafeWriteCursor < PlayCursor)
                            {
                                SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            Assert(SafeWriteCursor >= PlayCursor);
                            SafeWriteCursor += SoundOutput.SafetyBytes;
                            
                            bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);
                            
                            DWORD TargetCursor = 0;
                            if(AudioCardIsLowLatency)
                            {
                                TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
                            }
                            else
                            {
                                TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes);
                            }
                            TargetCursor = (TargetCursor % SoundOutput.SecondaryBufferSize);
                            
                            DWORD BytesToWrite = 0;
                            if(ByteToLock > TargetCursor)
                            {
                                BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                                BytesToWrite += TargetCursor;
                            }
                            else
                            {
                                BytesToWrite = TargetCursor - ByteToLock;
                            }
                            
                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                            SoundBuffer.Samples = Samples;
                            GameGetSoundSamples(&GameMemory, &SoundBuffer);
                            
#if DEBUG_AUDIO
                            win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                            Marker->OutputPlayCursor = PlayCursor;
                            Marker->OutputWriteCursor = WriteCursor;
                            Marker->OutputLocation = ByteToLock;
                            Marker->OutputByteCount = BytesToWrite;
                            Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;
                            
                            DWORD UnwrappedWriteCursor = WriteCursor;
                            if(UnwrappedWriteCursor < PlayCursor)
                            {
                                UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            DWORD AudioLatencyBytes = 0;
                            real32 AudioLatencySeconds = 0;
                            AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;
                            AudioLatencySeconds = (((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) /
                                                   (real32)SoundOutput.SamplesPerSecond);
                            
                            char TextBuffer[256];
                            _snprintf_s(TextBuffer, sizeof(TextBuffer),
                                        "BTL:%u TC:%u BTW:%u - PC:%u WC:%u DELTA:%u (%fs)\n", ByteToLock, TargetCursor,
                                        BytesToWrite, PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
                            OutputDebugStringA(TextBuffer);
#endif
                            
                            Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                        }
                        else
                        {
                            SoundIsValid = false;
                        }
                        
#if DEBUG_AUDIO
                        Win32DebugSyncDisplay(ArrayCount(DebugTimeMarkers), DebugTimeMarkers, //
                                              DebugTimeMarkerIndex - 1,                       //
                                              &SoundOutput);
#endif
                        
                        // FlipWallClock = Win32GetWallClock();
                        FlipWallClock = glfwGetTime();
                        
#if DEBUG_AUDIO
                        // NOTE(casey): This is debug code
                        {
                            DWORD PlayCursor1;
                            DWORD WriteCursor1;
                            if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor1, &WriteCursor1) == DS_OK)
                            {
                                Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers));
                                win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
                                Marker->FlipPlayCursor = PlayCursor1;
                                Marker->FlipWriteCursor = WriteCursor1;
                            }
                        }
#endif
                        
                        game_input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;
                        
                        if(GlobalToggleVSync)
                        {
                            GlobalGameUpdateHz = Win32GetMonitorRefreshHz(HWNDWindow);
                            TargetSecondsPerFrame = 1.0f / GlobalGameUpdateHz;
                            SoundOutput.SafetyBytes = (int)(((real32)SoundOutput.SamplesPerSecond * //
                                                             (real32)SoundOutput.BytesPerSample / GlobalGameUpdateHz) /
                                                            3.0f);
                            GlobalToggleVSync = false;
                        }
                        
#if DEBUG_AUDIO
                        ++DebugTimeMarkerIndex;
                        if(DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers))
                        {
                            DebugTimeMarkerIndex = 0;
                        }
#endif
                        
                        glfwSwapBuffers(Window);
                    }
                }
            }
        }
    }
    else
    {
        InvalidCodePath;
    }
    
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    glfwDestroyWindow(Window);
    glfwTerminate();
    
    return (0);
}