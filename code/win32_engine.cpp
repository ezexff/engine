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
b32 GlobalPause = false;
b32 GlobalIsFullscreen = false;
b32 GlobalIsWindowActive = false;
WINDOWPLACEMENT GlobalWindowPosition = {sizeof(GlobalWindowPosition)};

game_controller_input *NewKeyboardController;

IAudioClient* GlobalAudioClient;
IAudioRenderClient* GlobalAudioRenderClient;
#define FramesOfAudioLatency 1
#define MonitorRefreshHz 10000
s32 GlobalGameUpdateHz = (MonitorRefreshHz);

u64 GlobalTimerOffset;
u64 GlobalTimerFrequency;

#if ENGINE_INTERNAL
global debug_table GlobalDebugTable_;
debug_table *GlobalDebugTable = &GlobalDebugTable_;
#if ENGINE_IMGUI
b32 GlobalShowImGuiWindows = true; // All ImGui windows visibility
app_log *Log;
#endif
//~ NOTE(ezexff): Callbacks and inputs
// Add in beginning Win32MainWindowCallback for ImGui inputs
#if ENGINE_IMGUI
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

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

void Win32ToggleFullscreen(HWND Window);

//~ NOTE(ezexff): Message boxes
void
Win32ErrorMessageBox(char *Message)
{
    MessageBoxA(0, Message, 0, MB_OK);
}

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
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
                    }
                    else if(VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
                    }
                    else if(VKCode == VK_SPACE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }
                    else if(VKCode == VK_ESCAPE)
                    {
                        GlobalRunning = false;
                    }
#if ENGINE_INTERNAL
                    else if(VKCode == 'P')
                    {
                        if(IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    }
                    else if(VKCode == VK_F1)
                    {
                        if(IsDown)
                        {
#if ENGINE_IMGUI
                            GlobalShowImGuiWindows = !GlobalShowImGuiWindows;
                            //ShowCursor(GlobalShowImGuiWindows);
#endif
                        }
                    }
#endif
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
                }
            } break;
            
            case WM_MOUSEWHEEL:
            {
                Input->dMouseP.z = (r32)(GET_WHEEL_DELTA_WPARAM(Message.wParam) / WHEEL_DELTA);
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
#if ENGINE_IMGUI
        if(ImGui_ImplWin32_WndProcHandler(Window, Message, WParam, LParam))
        {
            return(true);
        }
#endif
#endif
    }
    
    LRESULT Result = 0;
    
    switch(Message)
    {
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_DESTROY:
        {
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
    HANDLE Win32Handle;
};

struct win32_platform_file_group
{
    HANDLE FindHandle;
    WIN32_FIND_DATAW FindData;
};

internal PLATFORM_GET_ALL_FILE_OF_TYPE_BEGIN(Win32GetAllFilesOfTypeBegin)
{
    platform_file_group Result = {};
    win32_platform_file_group *Win32FileGroup = 
    (win32_platform_file_group *)VirtualAlloc(0, sizeof(win32_platform_file_group),
                                              MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Result.Platform = Win32FileGroup;
    Result.FileCount = 0;
    
    wchar_t *WildCard = L"*.*";
    switch(Type)
    {
        case PlatformFileType_AssetFile:
        {
            WildCard = L"*.eab";
        } break;
        
        case PlatformFileType_SavedGameFile:
        {
            WildCard = L"*.es";
        } break;
        
        case PlatformFileType_VertFile:
        {
            WildCard = L"*.vert";
        } break;
        
        case PlatformFileType_FragFile:
        {
            WildCard = L"*.frag";
        } break;
        
        InvalidDefaultCase;
    }
    
    WIN32_FIND_DATAW FindData;
    HANDLE FindHandle = FindFirstFileW(WildCard, &FindData);
    while(FindHandle != INVALID_HANDLE_VALUE)
    {
        ++Result.FileCount;
        
        if(!FindNextFileW(FindHandle, &FindData))
        {
            break;
        }
    }
    FindClose(FindHandle);
    
    Win32FileGroup->FindHandle = FindFirstFileW(WildCard, &Win32FileGroup->FindData);
    
    return(Result);
}

internal PLATFORM_GET_ALL_FILE_OF_TYPE_END(Win32GetAllFilesOfTypeEnd)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    if(Win32FileGroup)
    {
        FindClose(Win32FileGroup->FindHandle);
        
        VirtualFree(Win32FileGroup, 0, MEM_RELEASE);
    }
}

// TODO(ezexff): mb enable optimization
#pragma optimize( "", off )
internal PLATFORM_OPEN_FILE(Win32OpenNextFile)
{
    win32_platform_file_group *Win32FileGroup = (win32_platform_file_group *)FileGroup->Platform;
    platform_file_handle Result = {};
    
    if(Win32FileGroup->FindHandle != INVALID_HANDLE_VALUE)
    {    
        win32_platform_file_handle *Win32Handle = (win32_platform_file_handle *)VirtualAlloc(0, sizeof(win32_platform_file_handle), 
                                                                                             MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        Result.Platform = Win32Handle;
        
        if(Win32Handle)
        {
            wchar_t *FileName = Win32FileGroup->FindData.cFileName;
            Win32Handle->Win32Handle = CreateFileW(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
            Result.NoErrors = (Win32Handle->Win32Handle != INVALID_HANDLE_VALUE);
            
            Result.Size = (Win32FileGroup->FindData.nFileSizeHigh * (MAXDWORD + 1)) + 
                Win32FileGroup->FindData.nFileSizeLow;
            
            Result.Name.Count = StringLength(FileName);
            Result.Name.Count++;
            u64 NameSizeInBytes = sizeof(wchar_t) * Result.Name.Count;
            Result.Name.Data = (u8 *)VirtualAlloc(0, NameSizeInBytes, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            u8 *FileNameU8 = (u8 *)FileName;
            for(u32 Index = 0;
                Index < NameSizeInBytes;
                Index++)
            {
                Result.Name.Data[Index] = FileNameU8[Index];
            }
            wchar_t *DataU16 = (wchar_t *)Result.Name.Data;
            DataU16[Result.Name.Count - 1] = L'\0';
        }
        
        if(!FindNextFileW(Win32FileGroup->FindHandle, &Win32FileGroup->FindData))
        {
            FindClose(Win32FileGroup->FindHandle);
            Win32FileGroup->FindHandle = INVALID_HANDLE_VALUE;
        }
    }
    
    return(Result);
}
#pragma optimize( "", on )

internal PLATFORM_FILE_ERROR(Win32FileError)
{
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
    Log->Add("[win32file] error: %s\n", Message);
#endif
#endif
    Handle->NoErrors = false;
}

internal PLATFORM_READ_DATA_FROM_FILE(Win32ReadDataFromFile)
{
    if(PlatformNoFileErrors(Source))
    {
        win32_platform_file_handle *Handle = (win32_platform_file_handle *)Source->Platform;
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
            Win32FileError(Source, "Read file failed!");
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
    if(FAILED(Device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (LPVOID *)&GlobalAudioClient)))
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
    if(FAILED(GlobalAudioClient->GetDevicePeriod(&phnsDefaultDevicePeriod, &phnsMinimumDevicePeriod)))
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
    if(FAILED(GlobalAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_NOPERSIST,
                                            BufferDuration, 0, &WaveFormat.Format, nullptr)))
    {
        InvalidCodePath;
    }
    
    // IID_IAudioRenderClient
    // Метод GetService обращается к дополнительным службам из объекта аудиоконферентного клиента
    if(FAILED(GlobalAudioClient->GetService(IID_PPV_ARGS(&GlobalAudioRenderClient))))
    {
        InvalidCodePath;
    }
    
    // Метод GetBufferSize извлекает размер (максимальную емкость) буфера конечной точки.
    UINT32 SoundFrameCount;
    if(FAILED(GlobalAudioClient->GetBufferSize(&SoundFrameCount)))
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
    if (SUCCEEDED(GlobalAudioRenderClient->GetBuffer((UINT32)SamplesToWrite, &SoundBufferData)))
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
        
        GlobalAudioRenderClient->ReleaseBuffer((UINT32)SamplesToWrite, 0);
        //GlobalAudioRenderClient->ReleaseBuffer((UINT32)SamplesToWrite, AUDCLNT_BUFFERFLAGS_SILENT);
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

//~ NOTE(ezexff): Thread work queue
struct platform_work_queue_entry
{
    platform_work_queue_callback *Callback;
    void *Data;
};

struct platform_work_queue
{
    u32 volatile CompletionGoal;
    u32 volatile CompletionCount;
    
    u32 volatile NextEntryToWrite;
    u32 volatile NextEntryToRead;
    HANDLE SemaphoreHandle;
    
    platform_work_queue_entry Entries[256];
};

internal void
Win32AddEntry(platform_work_queue *Queue, platform_work_queue_callback *Callback, void *Data)
{
    // TODO(casey): Switch to InterlockedCompareExchange eventually
    // so that any thread can add?
    u32 NewNextEntryToWrite = (Queue->NextEntryToWrite + 1) % ArrayCount(Queue->Entries);
    Assert(NewNextEntryToWrite != Queue->NextEntryToRead);
    platform_work_queue_entry *Entry = Queue->Entries + Queue->NextEntryToWrite;
    Entry->Callback = Callback;
    Entry->Data = Data;
    ++Queue->CompletionGoal;
    _WriteBarrier();
    Queue->NextEntryToWrite = NewNextEntryToWrite;
    ReleaseSemaphore(Queue->SemaphoreHandle, 1, 0);
}

internal b32
Win32DoNextWorkQueueEntry(platform_work_queue *Queue)
{
    b32 WeShouldSleep = false;
    
    u32 OriginalNextEntryToRead = Queue->NextEntryToRead;
    u32 NewNextEntryToRead = (OriginalNextEntryToRead + 1) % ArrayCount(Queue->Entries);
    if(OriginalNextEntryToRead != Queue->NextEntryToWrite)
    {
        u32 Index = InterlockedCompareExchange((LONG volatile *)&Queue->NextEntryToRead,
                                               NewNextEntryToRead,
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
    
    return(WeShouldSleep);
}

internal void
Win32CompleteAllWork(platform_work_queue *Queue)
{
    while(Queue->CompletionGoal != Queue->CompletionCount)
    {
        Win32DoNextWorkQueueEntry(Queue);
    }
    
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
}

DWORD WINAPI
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
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
    Log->Add("[win32workqueue] thread %u: %s\n", GetCurrentThreadId(), (char *)Data);
#endif
#endif
}

internal void
Win32MakeQueue(platform_work_queue *Queue, u32 ThreadCount)
{
    Queue->CompletionGoal = 0;
    Queue->CompletionCount = 0;
    
    Queue->NextEntryToWrite = 0;
    Queue->NextEntryToRead = 0;
    
    u32 InitialCount = 0;
    Queue->SemaphoreHandle = CreateSemaphoreEx(0,
                                               InitialCount,
                                               ThreadCount,
                                               0, 0, SEMAPHORE_ALL_ACCESS);
    for(u32 ThreadIndex = 0;
        ThreadIndex < ThreadCount;
        ++ThreadIndex)
    {
        DWORD ThreadID;
        HANDLE ThreadHandle = CreateThread(0, 0, ThreadProc, Queue, 0, &ThreadID);
        CloseHandle(ThreadHandle);
    }
}

PLATFORM_ALLOCATE_MEMORY(Win32AllocateMemory)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    return(Result);
}

PLATFORM_DEALLOCATE_MEMORY(Win32DeallocateMemory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

//~ NOTE(ezexff): Entry point
#if ENGINE_INTERNAL
int main(int, char**)
#else
extern "C" void __stdcall WinMainCRTStartup(void)
#endif
{
    DEBUGSetEventRecording(true);
    
    HINSTANCE Instance = GetModuleHandle(0);
    
    // NOTE(ezexff): Init thread queues
    platform_work_queue HighPriorityQueue = {};
    Win32MakeQueue(&HighPriorityQueue, 6);
    platform_work_queue LowPriorityQueue = {};
    Win32MakeQueue(&LowPriorityQueue, 2);
    
    // NOTE(ezexff): Import engine.dll functions
    void *TestLibrary = LoadLibraryA("engine.dll");
    update_and_render *UpdateAndRender = 0;
    get_sound_samples *GetSoundSamples = 0;
    debug_game_frame_end *DEBUGGameFrameEnd = 0;
    if(TestLibrary)
    {
        UpdateAndRender = (update_and_render *)GetProcAddress((HMODULE)TestLibrary, "UpdateAndRender");
        GetSoundSamples = (get_sound_samples *)GetProcAddress((HMODULE)TestLibrary, "GetSoundSamples");
        DEBUGGameFrameEnd = (debug_game_frame_end *)GetProcAddress((HMODULE)TestLibrary, "DEBUGGameFrameEnd");
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
    
#if !ENGINE_INTERNAL
    ShowCursor(1);
#endif
    
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
            GameMemory.TransientStorageSize = Megabytes(256);
#if ENGINE_INTERNAL
            GameMemory.DebugStorageSize = Megabytes(64);
            u64 TotalSize = (GameMemory.PermanentStorageSize + 
                             GameMemory.TransientStorageSize + 
                             GameMemory.DebugStorageSize);
#else
            u64 TotalSize = (GameMemory.PermanentStorageSize + 
                             GameMemory.TransientStorageSize);
#endif
            
            void *GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t)TotalSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            GameMemory.PermanentStorage = GameMemoryBlock;
            GameMemory.TransientStorage = ((u8 *)GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);
#if ENGINE_INTERNAL
            GameMemory.DebugStorage = ((u8 *)GameMemory.TransientStorage + GameMemory.TransientStorageSize);
#endif
            
            // NOTE(ezexff): Work queues
            GameMemory.HighPriorityQueue = &HighPriorityQueue;
            GameMemory.LowPriorityQueue = &LowPriorityQueue;
            GameMemory.PlatformAPI.AddEntry = Win32AddEntry;
            GameMemory.PlatformAPI.CompleteAllWork = Win32CompleteAllWork;
            GameMemory.PlatformAPI.AllocateMemory = Win32AllocateMemory;
            GameMemory.PlatformAPI.DeallocateMemory = Win32DeallocateMemory;
            
            // NOTE(ezexff): Pointers to platform API functions
            GameMemory.PlatformAPI.GetAllFilesOfTypeBegin = Win32GetAllFilesOfTypeBegin;
            GameMemory.PlatformAPI.GetAllFilesOfTypeEnd = Win32GetAllFilesOfTypeEnd;
            GameMemory.PlatformAPI.OpenNextFile = Win32OpenNextFile;
            GameMemory.PlatformAPI.ReadDataFromFile = Win32ReadDataFromFile;
            GameMemory.PlatformAPI.FileError = Win32FileError;
            
            
            // NOTE(ezexff): Init and load renderer
            renderer_frame *Frame = &GameMemory.Frame;
            LoadRenderer(Frame, GetDC(Window));
            
            // NOTE(ezexff): Init imgui for opengl
#if ENGINE_INTERNAL
            GameMemory.DebugTable = GlobalDebugTable;
            Frame->DebugTable = GlobalDebugTable;
#if ENGINE_IMGUI
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            ImPlot::CreateContext();
            
            imgui *ImGuiHandle = &GameMemory.ImGuiHandle;
            ImGuiHandle->ContextImGui = ImGui::GetCurrentContext();
            ImGuiHandle->ContextImPlot = ImPlot::GetCurrentContext();
            ImGui::GetAllocatorFunctions(&ImGuiHandle->AllocFunc, &ImGuiHandle->FreeFunc, &ImGuiHandle->UserData);
            
            GameMemory.ImGuiHandle.IO = &ImGui::GetIO();
            GameMemory.ImGuiHandle.IO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            ImGui_ImplWin32_InitForOpenGL(Window);
            ImGui_ImplOpenGL3_Init();
            
            ImGuiHandle->ShowWin32Window = true;
            ImGuiHandle->ShowGameWindow = true;
            ImGuiHandle->ShowDebugCollationWindow = true;
            ImGuiHandle->ShowLogWindow = true;
            Frame->ImGuiHandle = ImGuiHandle;
            Log = &ImGuiHandle->Log;
#endif
#endif
            
            // NOTE(ezexff): Init Audio
            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.BytesPerSample = sizeof(s16) * 2;
            SoundOutput.BufferSize = SoundOutput.SamplesPerSecond;
            SoundOutput.LatencySampleCount = FramesOfAudioLatency * (SoundOutput.SamplesPerSecond / GlobalGameUpdateHz);
            SoundOutput.LatencySampleCount += 1; // TODO(ezexff): нужно округление после деления?
            Win32InitWASAPI(SoundOutput.SamplesPerSecond, SoundOutput.BufferSize);
            GlobalAudioClient->Start();
            s16 *Samples = (s16 *)VirtualAlloc(0, SoundOutput.BufferSize,
                                               MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
            
            // NOTE(ezexff): Main Game Cycle
            GlobalRunning = true;
            if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {
                b32 IsCenteringMouseCursorInitialized = false;
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
                        NewInput->dtForFrame = (r32)DeltaFrameTime;
                        
                        if(GetActiveWindow() == Window)
                        {
                            GlobalIsWindowActive = true;
                        }
                        else
                        {
                            IsCenteringMouseCursorInitialized = false;
                            GlobalIsWindowActive = false;
                        }
                        
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
                            Frame->Dim.x = CWidth;
                            Frame->Dim.y = CHeight;
                            Frame->AspectRatio = (r32)Frame->Dim.x / (r32)Frame->Dim.y;
                        }
                        
                        // NOTE(ezexff): ImGui new frame
                        {
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                            ImGui_ImplOpenGL3_NewFrame();
                            ImGui_ImplWin32_NewFrame();
                            ImGui::NewFrame();
#endif
#endif
                        }
                        
                        // NOTE(ezexff): BEGIN FRAME
                        BEGIN_BLOCK("BeginFrame");
                        {
                            BeginFrame(Frame);
                        }
                        END_BLOCK();
                        
                        // NOTE(ezexff): Input processing
                        BEGIN_BLOCK("InputProcessing");
                        {
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
                            NewInput->dMouseP.z = 0;
#if ENGINE_IMGUI
                            // NOTE(ezexff): Win32 inputs with ImGui works only when ImGui windows is unselected
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
#else
                            Win32ProcessPendingMessages(NewKeyboardController, NewInput);
#endif
                            
                            if(GlobalIsWindowActive)
                            {
                                if(!IsCenteringMouseCursorInitialized && Input->CenteringMouseCursor)
                                {
                                    RECT WRectTmp;
                                    int CenterXTmp = 0;
                                    int CenterYTmp = 0;
                                    if(GetWindowRect(Window, &WRectTmp))
                                    {
                                        CenterXTmp = WRectTmp.left + Frame->Dim.x / 2;
                                        CenterYTmp = WRectTmp.top + Frame->Dim.y / 2;
                                    }
                                    SetCursorPos(CenterXTmp, CenterYTmp);
                                    
                                    NewInput->MouseP.x = 0;
                                    NewInput->MouseP.y = 0;
                                    NewInput->dMouseP.x = 0;
                                    NewInput->dMouseP.y = 0;
                                    
                                    IsCenteringMouseCursorInitialized = true;
                                }
                                
                                POINT MouseP;
                                GetCursorPos(&MouseP);
                                ScreenToClient(Window, &MouseP);
                                NewInput->MouseP.x = (r32)MouseP.x;
                                NewInput->MouseP.y = (r32)((Frame->Dim.y - 1) - MouseP.y);
                                
                                NewInput->dMouseP.x = NewInput->MouseP.x - OldInput->MouseP.x;
                                NewInput->dMouseP.y = NewInput->MouseP.y - OldInput->MouseP.y;
                                
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                                if(!ImGuiHandle->ShowImGuiWindows)
#endif
#endif
                                {
                                    if(Input->CenteringMouseCursor)
                                    {
                                        // Set cursor pos to window center
                                        RECT WRect;
                                        int CenterX = 0;
                                        int CenterY = 0;
                                        if(GetWindowRect(Window, &WRect))
                                        {
                                            CenterX = WRect.left + Frame->Dim.x / 2;
                                            CenterY = WRect.top + Frame->Dim.y / 2;
                                        }
                                        SetCursorPos(CenterX, CenterY);
                                        
                                        MouseP;
                                        GetCursorPos(&MouseP);
                                        ScreenToClient(Window, &MouseP);
                                        NewInput->MouseP.x = (r32)MouseP.x;
                                        NewInput->MouseP.y = (r32)((Frame->Dim.y - 1) - MouseP.y);
                                        //NewInput->MouseP.y = (r32)(MouseP.y); // TODO(ezexff): mb start use flipped y?
                                    }
                                }
                            }
#if 0
                            if(NewInput->MouseDelta.x != 0 || NewInput->MouseDelta.y != 0)
                            {
                                Log->Add("[win32input] mouse delta = %d %d\n",
                                         NewInput->MouseDelta.x, NewInput->MouseDelta.y);
                                
                                IsCenteringMouseCursorInitialized = true;
                            }
#endif
                            
                            // NOTE(ezexff): Mouse buttons
                            DWORD WinButtonID[PlatformMouseButton_Count] =
                            {
                                VK_LBUTTON,
                                VK_MBUTTON,
                                VK_RBUTTON,
                                VK_XBUTTON1,
                                VK_XBUTTON2,
                            };
                            if(GlobalIsWindowActive)
                            {
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
                            else
                            {
                                for(u32 ButtonIndex = 0;
                                    ButtonIndex < PlatformMouseButton_Count;
                                    ++ButtonIndex)
                                {
                                    NewInput->MouseButtons[ButtonIndex].EndedDown = false;
                                    NewInput->MouseButtons[ButtonIndex].HalfTransitionCount = 0;
                                }
                            }
                        }
                        END_BLOCK();
                        
                        // NOTE(ezexff): Init audio update
                        BEGIN_BLOCK("InitAudioUpdate");
                        int SamplesToWrite = 0;
                        UINT32 SoundPaddingSize = 0;
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
                            if(SUCCEEDED(GlobalAudioClient->GetCurrentPadding(&SoundPaddingSize)))
                            {
                                SamplesToWrite = (int)(SoundOutput.BufferSize - SoundPaddingSize);
                                {
                                    SoundOutput.LatencySampleCount = FramesOfAudioLatency * (SoundOutput.SamplesPerSecond / GlobalGameUpdateHz);
                                    // TODO(ezexff): нужно округление после деления?
                                    //SoundOutput.LatencySampleCount -= 5;
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                                    if(ImGuiHandle->LogAudio)
                                    {
                                        Log->Add("[win32audio] SamplesToWrite = %d LatencySampleCount = %d Aligned8LatencySampleCount = %d\n", 
                                                 SamplesToWrite, SoundOutput.LatencySampleCount, Align8(SoundOutput.LatencySampleCount));
                                    }
#endif
#endif
                                    SoundOutput.LatencySampleCount = Align8(SoundOutput.LatencySampleCount);
                                }
                                
                                if(SamplesToWrite > SoundOutput.LatencySampleCount)
                                {
                                    SamplesToWrite = SoundOutput.LatencySampleCount;
                                }
                            }
                            else
                            {
                                InvalidCodePath;
                            }
                            //SamplesToWrite = Align8(SamplesToWrite);
                        }
                        game_sound_output_buffer SoundBuffer = {};
                        SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                        SoundBuffer.SampleCount = SamplesToWrite;
                        SoundBuffer.Samples = Samples;
                        END_BLOCK();
                        
                        // NOTE(ezexff): Game update
                        BEGIN_BLOCK("GameUpdate");
                        if(!GlobalPause)
                        {
                            UpdateAndRender(&GameMemory, NewInput);
                        }
                        END_BLOCK();
                        
                        // NOTE(ezexff): Audio update
                        BEGIN_BLOCK("AudioUpdate");
                        if(!GlobalPause)
                        {
                            GetSoundSamples(&GameMemory, &SoundBuffer);
                            Win32FillSoundBuffer(&SoundOutput, SamplesToWrite, &SoundBuffer);
                        }
                        END_BLOCK();
                        
                        BEGIN_BLOCK("DebugCollation");
                        {
                            GameMemory.Paused = GlobalPause;
                            DEBUGGameFrameEnd(&GameMemory, NewInput);
                        }
                        END_BLOCK();
                        
                        // NOTE(ezexff): ImGui demo, win32 and renderer windows
                        {
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                            if(ImGuiHandle->ShowImGuiWindows)
                            {
                                if(ImGuiHandle->ShowWin32Window)
                                {
                                    ImGui::Begin("Win32");
                                    
                                    ImGui::Text("Debug window for win32 layer...");
                                    ImGui::BulletText("Is window active: %s", (GetActiveWindow() == Window) ? "true" : "false");
                                    ImGui::BulletText("Pause = %s", GlobalPause ? "true" : "false");
                                    
                                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGuiHandle->IO->Framerate, ImGuiHandle->IO->Framerate);
                                    
                                    
                                    //ImGui::SeparatorText("Windows visibility");
                                    ImGui::Checkbox("ImGui demo window", &ImGuiHandle->ShowImGuiDemoWindow);
                                    ImGui::Checkbox("ImPlot demo window", &ImGuiHandle->ShowImPlotDemoWindow);
                                    ImGui::Checkbox("Game window", &ImGuiHandle->ShowGameWindow);
                                    ImGui::Checkbox("Debug collation window", &ImGuiHandle->ShowDebugCollationWindow);
                                    ImGui::Checkbox("Log window", &ImGuiHandle->ShowLogWindow);
                                    
                                    ImGui::SeparatorText("HighPriorityQueue");
                                    ImGui::BulletText("CompletionGoal = %lu", GameMemory.HighPriorityQueue->CompletionGoal);
                                    ImGui::BulletText("CompletionCount = %lu", GameMemory.HighPriorityQueue->CompletionCount);
                                    ImGui::BulletText("NextEntryToWrite = %lu", GameMemory.HighPriorityQueue->NextEntryToWrite);
                                    ImGui::BulletText("NextEntryToRead = %lu", GameMemory.HighPriorityQueue->NextEntryToRead);
                                    
                                    ImGui::SeparatorText("LowPriorityQueue");
                                    ImGui::BulletText("CompletionGoal = %lu", GameMemory.LowPriorityQueue->CompletionGoal);
                                    ImGui::BulletText("CompletionCount = %lu", GameMemory.LowPriorityQueue->CompletionCount);
                                    ImGui::BulletText("NextEntryToWrite = %lu", GameMemory.LowPriorityQueue->NextEntryToWrite);
                                    ImGui::BulletText("NextEntryToRead = %lu", GameMemory.LowPriorityQueue->NextEntryToRead);
                                    
                                    ImGui::SeparatorText("Settings");
                                    bool IsFullscreen = GlobalIsFullscreen;
                                    if(ImGui::Checkbox("IsFullscreen", &IsFullscreen))
                                    {
                                        GlobalIsFullscreen = IsFullscreen;
                                        Win32ToggleFullscreen(Window);
                                    }
                                    //if(ImGui::SliderInt("fps", &GlobalGameUpdateHz, 30, 4096))
                                    if(ImGui::InputInt("fps lock", &GlobalGameUpdateHz))
                                    {
                                        if(GlobalGameUpdateHz < 5)
                                        {
                                            GlobalGameUpdateHz = 5;
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
                                        ImGui::BulletText("Dim = %dx%d", Frame->Dim.x, Frame->Dim.y);
                                        
                                        ImGui::BulletText("LoadedRendererName =");
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
                                            ImGui::BulletText("VENDOR = %s", Frame->Opengl.GLVendorStr);
                                            ImGui::BulletText("RENDERER = %s", Frame->Opengl.GLRendererStr);
                                            ImGui::BulletText("VERSION = %s", Frame->Opengl.GLVersionStr);
                                        }
                                    }
                                    
                                    if(ImGui::CollapsingHeader("Audio"))
                                    {
                                        ImGui::SeparatorText("WASAPI");
                                        ImGui::BulletText("BufferSize = %d", SoundOutput.BufferSize);
                                        //ImGui::Text("LatencySampleCount = %d", SoundOutput.LatencySampleCount);
                                        ImGui::BulletText("SamplesToWrite = %d", SamplesToWrite);
                                        ImGui::BulletText("SoundPaddingSize = %d", SoundPaddingSize);
                                        /*r64 AudioLatencyMS = 1000.0f *
                                        (r64)SoundOutput.LatencySampleCount / (r64)SoundOutput.BufferSize;
                                        ImGui::Text("AudioLatencyMS = %.10f (this value + 1 frame)", AudioLatencyMS);*/
                                        
                                        // Get playback cursor position
                                        IAudioClock* AudioClock;
                                        GlobalAudioClient->GetService(__uuidof(IAudioClock), (LPVOID*)(&AudioClock));
                                        u64 AudioPlaybackFreq;
                                        u64 AudioPlaybackPos;
                                        AudioClock->GetFrequency(&AudioPlaybackFreq);
                                        AudioClock->GetPosition(&AudioPlaybackPos, 0);
                                        AudioClock->Release();
                                        
                                        u64 AudioPlaybackPosInSeconds = AudioPlaybackPos / AudioPlaybackFreq;
                                        u64 AudioPlaybackPosInSamples = AudioPlaybackPosInSeconds*SoundOutput.SamplesPerSecond;
                                        
                                        ImGui::BulletText("AudioPlaybackFreq = %lu", AudioPlaybackFreq);
                                        ImGui::BulletText("AudioPlaybackPos = %lu", AudioPlaybackPos);
                                        ImGui::BulletText("AudioPlaybackPosInSeconds = %lu", AudioPlaybackPosInSeconds);
                                        ImGui::BulletText("AudioPlaybackPosInSamples = %lu", AudioPlaybackPosInSamples);
                                        
                                        ImGui::Checkbox("Log audio debug info", &ImGuiHandle->LogAudio);
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
                                            ImGui::Text("Attack");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Down arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Attack");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Left arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Attack");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Right arrow");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Attack");
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("Space");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Add hero");
                                            
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
                                            
                                            ImGui::TableNextRow();
                                            ImGui::TableSetColumnIndex(0);
                                            ImGui::Text("P");
                                            ImGui::TableSetColumnIndex(1);
                                            ImGui::Text("Pause");
                                            
                                            ImGui::EndTable();
                                        }
                                        ImGui::Text("NOTE: Win32 inputs with ImGui works\n");
                                        ImGui::Text("only when ImGui windows is unselected");
                                    }
                                    
                                    ImGui::End();
                                }
                                
                                if(ImGuiHandle->ShowImGuiDemoWindow)
                                {
                                    ImGui::ShowDemoWindow(&ImGuiHandle->ShowImGuiDemoWindow);
                                }
                                
                                if(ImGuiHandle->ShowImPlotDemoWindow)
                                {
                                    ImPlot::ShowDemoWindow(&ImGuiHandle->ShowImPlotDemoWindow);
                                }
                                
                                // NOTE(ezexff): Draw Log
                                if(ImGuiHandle->ShowLogWindow)
                                {
                                    Log->Draw("Log", &ImGuiHandle->ShowLogWindow);
                                }
                            }
#endif
#endif
                        }
                        
                        // NOTE(ezexff): END FRAME
                        BEGIN_BLOCK("EndFrame");
                        {
                            EndFrame(Frame);
                        }
                        END_BLOCK();
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                        // NOTE(ezexff): ImGui end frame
                        BEGIN_BLOCK("ImGuiRender");
                        {
                            ImGui::Render();
                            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                            SwapBuffers(GameMemory.ImGuiHandle.WGL);
                        }
                        END_BLOCK();
#endif
#endif
                        
                        // NOTE(ezexff): Save input state for next after flip frame
                        {
                            game_input *Temp = NewInput;
                            NewInput = OldInput;
                            OldInput = Temp;
                        }
                        
                        r64 SecondsElapsed = Win32GetTime() - StartFrameTime;
                        FRAME_MARKER((r32)SecondsElapsed);
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
#if ENGINE_IMGUI
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImPlot::DestroyContext();
        ImGui::DestroyContext();
#endif
#endif
    }
    
    GlobalAudioClient->Stop();
    GlobalAudioClient->Reset();
    GlobalAudioClient->Release();
    CoUninitialize();
    
    ExitProcess(0);
}