#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>

#include <mmdeviceapi.h>
#include <audioclient.h>
#include <timeapi.h>

#include "engine_platform.h"

#include "win32_engine_renderer.h"


b32 GlobalRunning = false;
b32 GlobalIsFullscreen = false;
WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

game_controller_input *NewKeyboardController;

IAudioClient* GlobalSoundClient;
IAudioRenderClient* GlobalSoundRenderClient;
#define FramesOfAudioLatency 1
#define MonitorRefreshHz 144
s32 GlobalGameUpdateHz = (MonitorRefreshHz);

u64 GlobalTimerOffset;
u64 GlobalTimerFrequency;

#if ENGINE_INTERNAL
app_log *Log;
b32 GlobalShowImGuiWindows = true; // All ImGui windows visibility

//~ NOTE(ezexff): Message boxes
void
Win32ErrorMessageBox(char *Message)
{
    MessageBoxA(0, Message, 0, MB_OK);
}

//~ NOTE(ezexff): Callbacks and inputs
// Add in beginning Win32MainWindowCallback for ImGui inputs
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void
Win32ToggleFullscreen(HWND Window);

void
ImGuiProcessPendingMessages(HWND Window)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
    
    // - IsKeyDown()
    // - IsKeyPressed()
    // - IsKeyReleased()
    //
    // - bool Repeat = false;
    // - ImGui::IsKeyPressed(ImGuiKey_F1, Repeat
}
#endif

void
Win32ProcessKeyboardMessage(game_button_state *NewState, b32 IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

void
Win32ProcessPendingMessages(game_controller_input *KeyboardController, game_input *Input)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            } break;
            
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYDOWN:
            case WM_KEYUP:
            {
                u32 VKCode = (u32)Message.wParam;
                
                // NOTE(casey): Since we are comparing WasDown to IsDown,
                // we MUST use == and != to convert these bit tests to actual
                // 0 or 1 values.
                // NOTE(ezexff): lParam flags
                // 30 bit - The previous key state. The value is always 1 for a WM_KEYUP message.
                // 31 bit - The transition state. The value is always 1 for a WM_KEYUP message.
                b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
                b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
                if(WasDown != IsDown)
                {
                    if(VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    }
                    if(VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    }
                    if(VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    }
                    if(VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    }
                    if(VKCode == VK_UP)
                    {
                        OutputDebugStringA("VK_UP\n");
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        OutputDebugStringA("VK_DOWN\n");
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        OutputDebugStringA("VK_LEFT\n");
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        OutputDebugStringA("VK_RIGHT\n");
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        OutputDebugStringA("VK_ESCAPE\n");
                        GlobalRunning = false;
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        OutputDebugStringA("VK_SPACE\n");
                    }
                }
                
                if(IsDown)
                {
                    bool AltKeyWasDown = (Message.lParam & (1 << 29));
                    if((VKCode == VK_F4) && AltKeyWasDown)
                    {
                        GlobalRunning = false;
                    }
                    else if((VKCode == VK_RETURN) && AltKeyWasDown)
                    {
                        if(Message.hwnd)
                        {
                            GlobalIsFullscreen = !GlobalIsFullscreen;
                            Win32ToggleFullscreen(Message.hwnd);
                        }
                    }
#if ENGINE_INTERNAL
                    else if(VKCode == VK_F1)
                    {
                        OutputDebugStringA("VK_F1\n");
                        GlobalShowImGuiWindows = !GlobalShowImGuiWindows;
                    }
#endif
                }
            } break;
            
            case WM_MOUSEWHEEL:
            {
                Input->MouseDelta.z = GET_WHEEL_DELTA_WPARAM(Message.wParam) / WHEEL_DELTA;
            } break;
            
            default:
            {
                TranslateMessage(&Message);
                DispatchMessageA(&Message);
            } break;
        }
    }
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    // NOTE(ezexff): ImGui message handler
    {
#if ENGINE_INTERNAL
        if (ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
        {
            return(true);
        }
#endif
    }
    
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CLOSE:
        {
            OutputDebugStringA("WM_CLOSE\n");
            GlobalRunning = false;
        } break;
        
        case WM_DESTROY:
        {
            OutputDebugStringA("WM_DESTROY\n");
            GlobalRunning = false;
        } break;
        
        default:
        {
            Result = DefWindowProcA(Window, Message, WParam, LParam);
        } break;
    }
    
    return(Result);
}

//~ NOTE(ezexff): File i/o
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
    win32_platform_file_group *Win32FileGroup = 
    (win32_platform_file_group *)VirtualAlloc(0, sizeof(win32_platform_file_group),
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
        Result = (win32_platform_file_handle *)VirtualAlloc(0, sizeof(win32_platform_file_handle),
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

//~ NOTE(ezexff): Screen mode: fullscreen or windowed
void
Win32ToggleFullscreen(HWND Window)
{
    // NOTE(casey): This follows Raymond Chen's prescription
    // for fullscreen toggling, see:
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPosition) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPosition);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

//~ NOTE(ezexff): Audio
void
Win32InitWASAPI(s32 SamplesPerSecond, s32 BufferSizeInSamples)
{
    // Инициализация COM библиотеки для вызывающего потока, задание модели параллелизма
    if(FAILED(CoInitializeEx(0, COINIT_SPEED_OVER_MEMORY)))
    {
        InvalidCodePath;
    }
    
    // IMMDeviceEnumerator предоставляет методы для перечисления звуковых устройств
    // Функция CoCreateInstance создает и инициализирует по умолчанию один объект класса,
    // связанный с указанным идентификатором CLSID
    IMMDeviceEnumerator *Enumerator;
    if(FAILED(CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&Enumerator))))
    {
        InvalidCodePath;
    }
    
    // IMMDevice представляет звуковое устройство
    // Метод GetDefaultAudioEndpoint извлекает конечную точку звука по умолчанию
    // для указанного направления и роли потока данных
    IMMDevice *Device;
    if(FAILED(Enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &Device)))
    {
        InvalidCodePath;
    }
    
    // Интерфейс IAudioClient позволяет клиенту создавать и инициализировать аудиопоток между
    // вуковым приложением и обработчиком звука (для потока в общем режиме) или аппаратным 
    // буфером устройства конечной точки аудио (для потока в монопольном режиме).
    // Метод Activate создает COM-объект с указанным интерфейсом
    if(FAILED(Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (LPVOID *)&GlobalSoundClient)))
    {
        InvalidCodePath;
    }
    
    // https://learn.microsoft.com/ru-ru/previous-versions/dd757713(v=vs.85)
    WAVEFORMATEXTENSIBLE WaveFormat;
    WaveFormat.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    WaveFormat.Format.cbSize = sizeof(WaveFormat);
    WaveFormat.Format.wBitsPerSample = 16;
    WaveFormat.Format.nChannels = 2;
    WaveFormat.Format.nSamplesPerSec = (DWORD)SamplesPerSecond;
    WaveFormat.Format.nBlockAlign = (WORD)(WaveFormat.Format.nChannels * WaveFormat.Format.wBitsPerSample / 8);
    WaveFormat.Format.nAvgBytesPerSec = WaveFormat.Format.nSamplesPerSec * WaveFormat.Format.nBlockAlign;
    
    WaveFormat.Samples.wValidBitsPerSample = 16;
    WaveFormat.dwChannelMask = KSAUDIO_SPEAKER_STEREO;
    WaveFormat.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
    
    // Размер буфера в 100 nanoseconds 
    // где 10000000ULL unsigned long long (u64) это 1000ms или 1 second
#if 1
    REFERENCE_TIME BufferDuration = 10000000ULL * BufferSizeInSamples / SamplesPerSecond;
#else
    // NOTE(ezexff): Low latency test
    
    // In 100 nanoseconds
    REFERENCE_TIME phnsDefaultDevicePeriod;
    REFERENCE_TIME phnsMinimumDevicePeriod;
    if(FAILED(GlobalSoundClient->GetDevicePeriod(&phnsDefaultDevicePeriod, &phnsMinimumDevicePeriod)))
    {
        InvalidCodePath;
    }
    r64 HundredNSToMs = 1000000 / 100;
    r64 DefaultDevicePeriod = (r64)phnsDefaultDevicePeriod / HundredNSToMs;
    r64 MinimumDevicePeriod = (r64)phnsMinimumDevicePeriod / HundredNSToMs;
    
    //REFERENCE_TIME BufferDuration = 0;
    REFERENCE_TIME BufferDuration = 2 * phnsMinimumDevicePeriod;
#endif
    
    // Метод Initialize инициализирует аудиопоток
    // 1. ShareMode: AUDCLNT_SHAREMODE_EXCLUSIVE или AUDCLNT_SHAREMODE_SHARED
    // 2. StreamFlags: AUDCLNT_STREAMFLAGS_NOPERSIST (не сохранять параметры громкости и отключения звука
    // при перезапуске приложения)
    // 3. Ёмкость буфера
    // 4. Период устройства
    // 5. Указатель на дескриптор формата
    if(FAILED(GlobalSoundClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST,
                                            BufferDuration, 0, &WaveFormat.Format, nullptr)))
    {
        InvalidCodePath;
    }
    
    // IID_IAudioRenderClient
    // Метод GetService обращается к дополнительным службам из объекта аудиоконферентного клиента
    if(FAILED(GlobalSoundClient->GetService(IID_PPV_ARGS(&GlobalSoundRenderClient))))
    {
        InvalidCodePath;
    }
    
    // Метод GetBufferSize извлекает размер (максимальную емкость) буфера конечной точки.
    UINT32 SoundFrameCount;
    if(FAILED(GlobalSoundClient->GetBufferSize(&SoundFrameCount)))
    {
        InvalidCodePath;
    }
    
    Assert(BufferSizeInSamples <= (s32)SoundFrameCount);
    
    Enumerator->Release();
    
    
    // NOTE(ezexff): Detect lowest possible latency
    {
#if 0
        IAudioClient3* SoundClient3;
        // IID_IAudioClient3
        if(FAILED(Device->Activate(__uuidof(SoundClient3), CLSCTX_ALL, NULL, (LPVOID *)&SoundClient3)))
        {
            InvalidCodePath;
        }
        
        WAVEFORMATEX *WaveFormat2;
        if(FAILED(SoundClient3->GetMixFormat(&WaveFormat2)))
        {
            InvalidCodePath;
        }
        
        UINT32 DefaultBufferSize;
        UINT32 FundamentalBufferSize;
        UINT32 MinimumBufferSize;
        UINT32 MaximumBufferSize;
        if(FAILED(SoundClient3->GetSharedModeEnginePeriod(WaveFormat2, 
                                                          &DefaultBufferSize,
                                                          &FundamentalBufferSize,
                                                          &MinimumBufferSize,
                                                          &MaximumBufferSize)))
        {
            InvalidCodePath;
        }
        r64 SampleRate = WaveFormat2->nSamplesPerSec;
        r64 BufferSizeMin = 1000.0f * MinimumBufferSize / SampleRate;
        r64 BufferSizeMax = 1000.0f * MaximumBufferSize / SampleRate;
        r64 BufferSizeDefault = 1000.0f * DefaultBufferSize;
#endif
    }
}

struct win32_sound_output
{
    s32 SamplesPerSecond;
    u32 RunningSampleIndex;
    s32 BytesPerSample;
    u32 BufferSize;
    s32 LatencySampleCount;
};

void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, int SamplesToWrite,
                     game_sound_output_buffer *SourceBuffer)
{
    // NOTE(ezexff): Для успешного выполнения метода GetBuffer() нужно запрашивать
    // число семплов не превышающее доступное пространство в буфере
    // Метод GetBuffer извлекает указатель на следующее доступное пространство в буфере 
    // конечной точки отрисовки, в которое вызывающий объект может записать пакет данных
    BYTE *SoundBufferData;
    if (SUCCEEDED(GlobalSoundRenderClient->GetBuffer((UINT32)SamplesToWrite, &SoundBufferData)))
    {
        s16* SourceSample = SourceBuffer->Samples;
        s16* DestSample = (s16*)SoundBufferData;
        for(int SampleIndex = 0;
            SampleIndex < SamplesToWrite;
            ++SampleIndex)
        {
            *DestSample++ = *SourceSample++; 
            *DestSample++ = *SourceSample++; 
            ++SoundOutput->RunningSampleIndex;
        }
        
        GlobalSoundRenderClient->ReleaseBuffer((UINT32)SamplesToWrite, 0);
        //GlobalSoundRenderClient->ReleaseBuffer((UINT32)SamplesToWrite, AUDCLNT_BUFFERFLAGS_SILENT);
    }
    else
    {
        InvalidCodePath;
    }
}

//~ NOTE(ezexff): Timers
u64
Win32GetTimerFrequency(void)
{
    u64 Result;
    QueryPerformanceFrequency((LARGE_INTEGER *)&Result);
    return(Result);
}

u64
Win32GetTimerValue(void)
{    
    u64 Result;
    QueryPerformanceCounter((LARGE_INTEGER *)&Result);
    return(Result);
}

r64
Win32GetTime(void)
{
    r64 Result;
    Result = (r64)(Win32GetTimerValue() - GlobalTimerOffset) / GlobalTimerFrequency;
    return(Result);
}

//~ NOTE(ezexff): Entry point
#if ENGINE_INTERNAL
int main(int, char**)
#else
extern "C" void __stdcall WinMainCRTStartup(void)
#endif
{
    HINSTANCE Instance = GetModuleHandle(0);
    
    // NOTE(ezexff): Import engine.dll functions
    void *TestLibrary = LoadLibraryA("engine.dll");
    update_and_render *UpdateAndRender = 0;
    get_sound_samples *GetSoundSamples = 0;
    if(TestLibrary)
    {
        UpdateAndRender = (update_and_render *)GetProcAddress((HMODULE)TestLibrary, "UpdateAndRender");
        GetSoundSamples = (get_sound_samples *)GetProcAddress((HMODULE)TestLibrary, "GetSoundSamples");
    }
    else
    {
        Win32ErrorMessageBox("Can't open engine.dll");
        ExitProcess(0);
    }
    
    // NOTE(ezexff): Import win32_engine_opengl.dll functions
    loaded_renderer_name LoadedRendererName = LoadedRenderer_empty;
    TestLibrary = LoadLibraryA("win32_engine_opengl.dll");
    win32_load_renderer *LoadRenderer = 0;
    renderer_begin_frame *BeginFrame = 0;
    renderer_end_frame *EndFrame = 0;
    if(TestLibrary)
    {
        LoadRenderer = (win32_load_renderer *)GetProcAddress((HMODULE)TestLibrary, "Win32LoadRenderer");
        BeginFrame = (renderer_begin_frame *)GetProcAddress((HMODULE)TestLibrary, "Win32BeginFrame");
        EndFrame = (renderer_end_frame *)GetProcAddress((HMODULE)TestLibrary, "Win32EndFrame");
        if(LoadRenderer && BeginFrame && EndFrame)
        {
            LoadedRendererName = LoadedRenderer_opengl;
        }
        else
        {
            Win32ErrorMessageBox("Can't load some function(s) from win32_engine_opengl.dll");
            ExitProcess(0);
        }
    }
    else
    {
        Win32ErrorMessageBox("Can't open win32_engine_opengl.dll");
        ExitProcess(0);
    }
    
    
    GlobalTimerFrequency = Win32GetTimerFrequency();
    GlobalTimerOffset = Win32GetTimerValue();
    
    WNDCLASSA WindowClass = {};
    WindowClass.style = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    // WindowClass.hIcon = ;
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    // WindowsClass.lpszMenuName = ;
    WindowClass.lpszClassName = "EngineWindowClass";
    
    if(RegisterClassA(&WindowClass))
    {
        HWND Window =
            CreateWindowExA(
                            0, // WS_EX_TOPMOST|WS_EX_LAYERED,
                            WindowClass.lpszClassName,
                            "C++ Game Engine",
                            WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            CW_USEDEFAULT,
                            0,
                            0,
                            Instance,
                            0);
        if(Window)
        {
            if(GlobalIsFullscreen)
            {
                Win32ToggleFullscreen(Window);
            }
            
            // NOTE(ezexff): Init big memory chunk
#if ENGINE_INTERNAL
            LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else
            LPVOID BaseAddress = 0;
#endif
            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes(32);
            GameMemory.TransientStorageSize = Megabytes(128);
            u64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            void *GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            GameMemory.PermanentStorage = GameMemoryBlock;
            GameMemory.TransientStorage = ((u8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);
            
            // NOTE(ezexff): Pointers to platform API functions
            GameMemory.PlatformAPI.GetAllFilesOfTypeBegin = Win32GetAllFilesOfTypeBegin;
            GameMemory.PlatformAPI.GetAllFilesOfTypeEnd = Win32GetAllFilesOfTypeEnd;
            GameMemory.PlatformAPI.OpenNextFile = Win32OpenNextFile;
            GameMemory.PlatformAPI.ReadDataFromFile = Win32ReadDataFromFile;
            GameMemory.PlatformAPI.FileError = Win32FileError;
            
            
            // NOTE(ezexff): Load and init renderer
            renderer_frame *Frame = &GameMemory.Frame;
            LoadRenderer(Frame, GetDC(Window));
            
            // NOTE(ezexff): Init imgui for opengl
#if ENGINE_INTERNAL
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            
            imgui *ImGuiHandle = &GameMemory.Frame.ImGuiHandle;
            ImGuiHandle->Context = ImGui::GetCurrentContext();
            ImGui::GetAllocatorFunctions(&ImGuiHandle->AllocFunc, &ImGuiHandle->FreeFunc, &ImGuiHandle->UserData);
            
            Frame->ImGuiHandle.IO = &ImGui::GetIO();
            Frame->ImGuiHandle.IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui_ImplWin32_InitForOpenGL(Window);
            ImGui_ImplOpenGL3_Init();
            
            ImGuiHandle->ShowWin32Window = true;
            ImGuiHandle->ShowDemoWindow = false;
            ImGuiHandle->ShowGameWindow = true;
            
            Log = &ImGuiHandle->Log;
#endif
            
            // NOTE(ezexff): Init Audio
            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(s16) * 2;
            SoundOutput.BufferSize = SoundOutput.SamplesPerSecond;
            SoundOutput.LatencySampleCount = FramesOfAudioLatency * (SoundOutput.SamplesPerSecond / GlobalGameUpdateHz);
            SoundOutput.LatencySampleCount += 1; // TODO(ezexff): нужно округление после деления?
            Win32InitWASAPI(SoundOutput.SamplesPerSecond, SoundOutput.BufferSize);
            GlobalSoundClient->Start();
            s16 *Samples = (s16 *)VirtualAlloc(0, SoundOutput.BufferSize,
                                               MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            // NOTE(ezexff): Main Game Cycle
            GlobalRunning = true;
            if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {
                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];
                
                r64 TargetSecondsPerFrame = 1.0f / (r64)GlobalGameUpdateHz;
                r64 DeltaFrameTime = 0.0f;
                r64 EndFrameTime = 0.0f;
                while(GlobalRunning)
                {
                    r64 StartFrameTime = Win32GetTime(); // ms
                    DeltaFrameTime = StartFrameTime - EndFrameTime;
                    if(DeltaFrameTime >= TargetSecondsPerFrame)
                    {
                        EndFrameTime = StartFrameTime;
                        
                        // NOTE(ezexff): Setting up display area
                        {
                            RECT CRect;
                            int CWidth = 960;
                            int CHeight = 540;
                            if(GetClientRect(Window, &CRect))
                            {
                                CWidth = CRect.right - CRect.left;
                                CHeight = CRect.bottom - CRect.top;
                            }
                            Frame->Width = CWidth;
                            Frame->Height = CHeight;
                        }
                        
                        // NOTE(ezexff): ImGui new frame
                        {
#if ENGINE_INTERNAL
                            ImGui_ImplOpenGL3_NewFrame();
                            ImGui_ImplWin32_NewFrame();
                            ImGui::NewFrame();
#endif
                        }
                        // NOTE(ezexff): BEGIN FRAME
                        {
                            BeginFrame(Frame);
                        }
                        
                        // NOTE(ezexff): Input processing
                        {
                            Input->dtForFrame = (r32)TargetSecondsPerFrame;
                            
                            // NOTE(ezexff): Load prev frame keyboard buttons state
                            game_controller_input *OldKeyboardController = GetController(OldInput, 0);
                            NewKeyboardController = GetController(NewInput, 0);
                            *NewKeyboardController = {};
                            NewKeyboardController->IsConnected = true;
                            for(int ButtonIndex = 0;
                                ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
                                ++ButtonIndex)
                            {
                                NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                                    OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                            }
                            
#if ENGINE_INTERNAL
                            // NOTE(ezexff): Win32 inputs with ImGui works only when ImGui windows is unselected
                            NewInput->MouseDelta.z = 0;
                            if(ImGuiHandle->IO->WantCaptureKeyboard)
                            {
                                ImGuiProcessPendingMessages(Window);
                            }
                            else
                            {
                                Win32ProcessPendingMessages(NewKeyboardController, NewInput);
                            }
                            ImGuiHandle->ShowImGuiWindows = GlobalShowImGuiWindows;
#else
                            Win32ProcessPendingMessages(NewKeyboardController, NewInput);
#endif
                            if(GetActiveWindow() == Window)
                            {
                                // NOTE(ezexff): Mouse cursor
                                POINT MouseP;
                                GetCursorPos(&MouseP);
                                ScreenToClient(Window, &MouseP);
                                NewInput->MouseP.x = (s32)MouseP.x;
                                NewInput->MouseP.y = (s32)((Frame->Height - 1) - MouseP.y);
                                
                                NewInput->MouseDelta.x = NewInput->MouseP.x - OldInput->MouseP.x;
                                NewInput->MouseDelta.y = NewInput->MouseP.y - OldInput->MouseP.y;
                                
                                // NOTE(ezexff): Mouse buttons
                                DWORD WinButtonID[PlatformMouseButton_Count] =
                                {
                                    VK_LBUTTON,
                                    VK_MBUTTON,
                                    VK_RBUTTON,
                                    VK_XBUTTON1,
                                    VK_XBUTTON2,
                                };
                                for(u32 ButtonIndex = 0;
                                    ButtonIndex < PlatformMouseButton_Count;
                                    ++ButtonIndex)
                                {
                                    NewInput->MouseButtons[ButtonIndex] = OldInput->MouseButtons[ButtonIndex];
                                    NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
                                    // NOTE(ezexff): GetKeyState возвращает значение размером 16 bit
                                    // если старший бит равен 1, то клавиша нажата
                                    // если младший бит равен 1, то ключ клавиши переключен (Caps Lock, Num Lock)
                                    // старший бит числа размером 16 бит = 1 это (1 << 15) или (1000 0000 0000 0000)
                                    // младший бит числа размером 16 бит = 1 это 1
                                    Win32ProcessKeyboardMessage(&NewInput->MouseButtons[ButtonIndex],
                                                                GetKeyState(WinButtonID[ButtonIndex]) & (1 << 15));
                                }
                            }
                        }
                        
                        // NOTE(ezexff): Game update
                        {
                            UpdateAndRender(&GameMemory, NewInput);
                        }
                        
                        // NOTE(ezexff): Audio update
                        int SamplesToWrite = 0;
                        UINT32 SoundPaddingSize;
                        {
                            // NOTE(ezexff): Определяем сколько данных звука и куда можно записать
                            /* 
Звуковой буфер:
- Сэмплы это кусочки звука размером 2 * 16 бит, где 2 это каналы
 - Весь размер буфера звука 48000 семплов или же 1 секунда
- Буфер цикличен

Звуковой движок WASAPI:
- Работает в двух режимах: общий и эксклюзивный
- В общем режиме, вызов метода GetCurrentPadding() покажет число семплов,
поставленных в очередь на воспроизведение
- 
*/
                            // SamplesToWrite - сколько данных можно безопасно записать, не перезаписав
                            // данные, которые будут воспроизводиться
                            
                            // Метод GetCurrentPadding получает значение заполнения, указывающее
                            // объем действительных непрочитанных данных, которые в данный момент
                            // содержатся в буфере конечной точки
                            
                            // Если SamplesToWrite = BufferSize - SoundPaddingSize, это число семплов,
                            // которое можем безопасно записать, не задев непрочитанные данные
                            if(SUCCEEDED(GlobalSoundClient->GetCurrentPadding(&SoundPaddingSize)))
                            {
                                SamplesToWrite = (int)(SoundOutput.BufferSize - SoundPaddingSize);
                                
                                {
                                    SoundOutput.LatencySampleCount = FramesOfAudioLatency * (SoundOutput.SamplesPerSecond / GlobalGameUpdateHz);
                                    // TODO(ezexff): нужно округление после деления?
                                    SoundOutput.LatencySampleCount += 1;
                                }
                                
                                if(SamplesToWrite > SoundOutput.LatencySampleCount)
                                {
                                    SamplesToWrite = SoundOutput.LatencySampleCount;
                                }
                            }
                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = SamplesToWrite;
                            SoundBuffer.Samples = Samples;
                            
                            GetSoundSamples(&GameMemory, &SoundBuffer);
                            
                            Win32FillSoundBuffer(&SoundOutput, SamplesToWrite, &SoundBuffer);
                        }
                        
                        // NOTE(ezexff): ImGui demo, win32 and renderer windows
                        {
#if ENGINE_INTERNAL
                            if(ImGuiHandle->ShowImGuiWindows)
                            {
                                if(ImGuiHandle->ShowWin32Window)
                                {
                                    ImGui::Begin("Win32");
                                    
                                    ImGui::Text("Debug window for win32 layer...");
                                    ImGui::Text("Is window active: %s", (GetActiveWindow() == Window) ? "true" : "false");
                                    
                                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGuiHandle->IO->Framerate, ImGuiHandle->IO->Framerate);
                                    
                                    ImGui::SeparatorText("Settings");
                                    ImGui::Checkbox("Show demo window", &ImGuiHandle->ShowDemoWindow);
                                    bool IsFullscreen = GlobalIsFullscreen;
                                    if(ImGui::Checkbox("Is fullscreen?", &IsFullscreen))
                                    {
                                        GlobalIsFullscreen = IsFullscreen;
                                        Win32ToggleFullscreen(Window);
                                        Log->Add("[win32]: IsFullscreen = %d\n", IsFullscreen);
                                    }
                                    //if(ImGui::SliderInt("fps", &GlobalGameUpdateHz, 30, 4096))
                                    if(ImGui::InputInt("fps lock", &GlobalGameUpdateHz))
                                    {
                                        if(GlobalGameUpdateHz < 30)
                                        {
                                            GlobalGameUpdateHz = 30;
                                        } 
                                        else if(GlobalGameUpdateHz > 4096)
                                        {
                                            GlobalGameUpdateHz = 4096;
                                        }
                                        TargetSecondsPerFrame = 1.0f / (r64)GlobalGameUpdateHz;
                                    }
                                    
                                    ImGui::SeparatorText("Subsystems");
                                    if(ImGui::CollapsingHeader("Renderer"))
                                    {
                                        ImGui::SeparatorText("Frame");
                                        ImGui::Text("Dim = %dx%d", Frame->Width, Frame->Height);
                                        
                                        ImGui::Text("Loaded renderer:");
                                        switch(LoadedRendererName)
                                        {
                                            case LoadedRenderer_opengl:
                                            {
                                                ImGui::SameLine();
                                                ImGui::Text("OpenGL");
                                            } break;
                                            case LoadedRenderer_vulkan:
                                            {
                                                ImGui::SameLine();
                                                ImGui::Text("Vulkan");
                                            } break;
                                            case LoadedRenderer_direct3d:
                                            {
                                                ImGui::SameLine();
                                                ImGui::Text("Direct3D");
                                            } break;
                                            
                                            InvalidDefaultCase;
                                        }
                                        
                                        if(LoadedRendererName == LoadedRenderer_opengl)
                                        {
                                            ImGui::SeparatorText("OpenGL");
                                            ImGui::Text("VENDOR = %s", Frame->GLVendorStr);
                                            ImGui::Text("RENDERER = %s", Frame->GLRendererStr);
                                            ImGui::Text("VERSION = %s", Frame->GLVersionStr);
                                        }
                                    }
                                    
                                    if(ImGui::CollapsingHeader("Audio"))
                                    {
                                        ImGui::SeparatorText("WASAPI");
                                        ImGui::Text("BufferSize = %d", SoundOutput.BufferSize);
                                        //ImGui::Text("LatencySampleCount = %d", SoundOutput.LatencySampleCount);
                                        ImGui::Text("SamplesToWrite = %d", SamplesToWrite);
                                        ImGui::Text("SoundPaddingSize = %d", SoundPaddingSize);
                                        /*r64 AudioLatencyMS = 1000.0f *
                                        (r64)SoundOutput.LatencySampleCount / (r64)SoundOutput.BufferSize;
                                        ImGui::Text("AudioLatencyMS = %.10f (this value + 1 frame)", AudioLatencyMS);*/
                                    }
                                    
                                    if(ImGui::CollapsingHeader("Input"))
                                    {
                                        ImGui::SeparatorText("Cheatsheet of implemented inputs");
                                        
                                        if(ImGui::BeginTable("KeysCheatsheetTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
                                        {
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Input");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Description");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("LButton");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("MButton");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("RButton");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("XButton1");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("XButton2");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Up/Down mouse wheel");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("W");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("MoveUp");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("S");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("MoveDown");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("A");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("MoveLeft");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("D");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("MoveRight");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Up arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Down arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Left arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Right arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Escape");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Close game");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Alt+F4");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Close game");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Alt+Enter");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Change window mode");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("F1");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("ImGui windows visibility");
                                            
                                            ImGui::EndTable();
                                        }
                                        ImGui::Text("NOTE: Win32 inputs with ImGui works\n");
                                        ImGui::Text("only when ImGui windows is unselected");
                                    }
                                    
                                    ImGui::End();
                                }
                                
                                if(ImGuiHandle->ShowDemoWindow)
                                {
                                    ImGui::ShowDemoWindow(&ImGuiHandle->ShowDemoWindow);
                                }
                                
                                // NOTE(ezexff): Draw Log
                                Log->Draw("Log");
                            }
#endif
                        }
                        
                        // NOTE(ezexff): END FRAME
                        {
                            EndFrame(Frame);
                        }
#if ENGINE_INTERNAL
                        // NOTE(ezexff): ImGui end frame
                        {
                            ImGui::Render();
                            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                            SwapBuffers(Frame->ImGuiHandle.WGL);
                        }
#endif
                        
                        // NOTE(ezexff): Save input state for next after flip frame
                        {
                            game_input *Temp = NewInput;
                            NewInput = OldInput;
                            OldInput = Temp;
                        }
                    }
                }
            }
            else
            {
                Win32ErrorMessageBox("Can't alloc memory for samples");
            }
        }
        else
        {
            Win32ErrorMessageBox("Can't create window");
        }
    }
    else
    {
        Win32ErrorMessageBox("Can't register window class");
    }
    
    // NOTE(ezexff): ImGui destroy
    {
#if ENGINE_INTERNAL
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
#endif
    }
    
    GlobalSoundClient->Stop();
    GlobalSoundClient->Reset();
    GlobalSoundClient->Release();
    CoUninitialize();
    
    ExitProcess(0);
}