#include "engine.h"

#include "engine_asset.cpp"

#include "engine_audio.cpp"
#include "engine_renderer.cpp"

#include "engine_entity.cpp"
#include "engine_world.cpp"
#include "engine_sim_region.cpp"

internal void
DEBUGTextLine(renderer_frame *Frame, game_assets *Assets, char *String)
{
    font_id FontID = GetFirstFontFrom(Assets, Asset_Font);
    loaded_font *Font = PushFont(Frame, Assets, FontID);
    if(Font)
    {
        eab_font *FontInfo = GetFontInfo(Assets, FontID);
        
        r32 FontScale = FontInfo->Scale;
        r32 LeftEdge = 0.0f;
        
        u32 PrevCodePoint = 0;
        r32 CharScale = FontScale;
        //CharScale = 1.0f;
        v4 Color = V4(1, 1, 1, 1);
        
        //r32 AtX = LeftEdge;
        //r32 AtY = 0.0f;
        s32 AtX = 0;
        s32 AtY = 0;
        
        for(char *At = String;
            *At;
            )
        {
            u32 CodePoint = *At;
            
            //r32 AdvanceX = CharScale * GetHorizontalAdvanceForPair(FontInfo, Font, PrevCodePoint, CodePoint);
            
            //r32 AdvanceX = ((r32)Font->Advances[PrevCodePoint] / (r32)Font->Advances[0]) * 150.0f;
            //AdvanceX = 70.0f;
            
            s32 Ascent = RoundR32ToS32(FontInfo->Ascent * FontScale);
            //AtY = Ascent;
            
            s32 LSB = Font->LSBs[CodePoint];
            s32 XOffset = s32(Font->GlyphOffsets[CodePoint].x);
            s32 YOffset = s32(Font->GlyphOffsets[CodePoint].y);
            
            if(CodePoint != ' ')
            {
                
                bitmap_id BitmapID = GetBitmapForGlyph(Assets, FontInfo, Font, CodePoint);
                eab_bitmap *GlyphInfo = GetBitmapInfo(Assets, BitmapID);
                
                //PushBitmap(RenderGroup, BitmapID, CharScale*(r32)Info->Dim[1], V3(AtX, AtY, 0), Color);
                v2 GlyphDim;
                GlyphDim.x = (r32)GlyphInfo->Dim[0];
                GlyphDim.y = (r32)GlyphInfo->Dim[1];
                v2 Pos = V2(0, 0);
                Pos.x = (r32)(AtX + XOffset);
                Pos.y = (r32)AtY + YOffset;
                PushBitmapOnScreen(Frame, Assets, BitmapID, Pos, GlyphDim, 1.0f);
            }
            
            s32 AdvanceX = RoundR32ToS32(Font->Advances[CodePoint] * CharScale);
            //s32 AdvanceX = RoundR32ToS32(70 * CharScale);
            //s32 AdvanceX = RoundR32ToS32(70);
            AtX += AdvanceX;
            
            //PrevCodePoint = CodePoint;
            
            ++At;
        }
    }
}

#include "engine_mode_test.cpp"
#include "engine_mode_world.cpp"

struct test_work
{
    task_with_memory *Task;
};
internal PLATFORM_WORK_QUEUE_CALLBACK(TestWork)
{
    test_work *Work = (test_work *)Data;
    
#if ENGINE_INTERNAL
    Log->Add("[enginework] TestWorkWithMemory\n");
#endif
    
    EndTaskWithMemory(Work->Task);
}

extern "C" UPDATE_AND_RENDER_FUNC(UpdateAndRender)
{
    renderer_frame *Frame = &Memory->Frame;
    opengl *Opengl = &Frame->Opengl;
    
#if ENGINE_INTERNAL
    GlobalDebugTable = Memory->DebugTable;
    DebugGlobalMemory = Memory;
    DebugGlobalInput = Input;
#endif
    
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!GameState->IsInitialized)
    {
        InitializeArena(&GameState->ConstArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        memory_arena *ConstArena = &GameState->ConstArena;
        
        // NOTE(ezexff): Init game sound output
        InitializeAudioState(&GameState->AudioState, &GameState->ConstArena);
        
        Platform = Memory->PlatformAPI;
        
        // NOTE(ezexff): Init Log App
        {
#if ENGINE_INTERNAL
            imgui *ImGuiHandle = &DebugGlobalMemory->ImGuiHandle;
            ImGui::SetCurrentContext(ImGuiHandle->ContextImGui);
            ImPlot::SetCurrentContext(ImGuiHandle->ContextImPlot);
            ImGui::SetAllocatorFunctions(ImGuiHandle->AllocFunc, ImGuiHandle->FreeFunc, ImGuiHandle->UserData);
            Log = &DebugGlobalMemory->ImGuiHandle.Log;
            /*Log->Add("[log init]: Hello %d world\n", 123);*/
            
            ImGuiHandle->LogMouseInput = false;
            ImGuiHandle->LogKeyboardInput = false;
#endif
        }
        
        // NOTE(ezexff): Set current game mode
        {
            GameState->GameModeID = GameMode_World;
        }
        
        // NOTE(ezexff): Init sound mixer
        {
#if ENGINE_INTERNAL
            GameState->IsTestSineWave = false;
            GameState->ToneHz = 512;
            GameState->ToneVolume = 500;
            GameState->tSine = 0;
            GameState->SampleIndex = 0;
            
            //GameState->LoadedSound = LoadFirstWAV(&GameState->ConstArena, &Memory->PlatformAPI);
            /*if(!GameState->LoadedSound.SampleCount == 0)
            {
                Log->Add("[asset]: Sound successfully loaded\n");
            }
            else
            {
                Log->Add("[asset]: Sound loading failed\n");
            }*/
#endif
        }
        
        // NOTE(ezexff): Screen shader program
        {
            LoadFrameAndSkyboxShaders(ConstArena, Frame);
            Frame->CompileShaders = true;
        }
        
        // NOTE(ezexff): Set default frame frag effect
        {
            Frame->FragEffect = EFFECT_NORMAL;
        }
        
        GameState->IsInitialized = true;
    }
    
    Assert(sizeof(tran_state) <= Memory->TransientStorageSize);
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(tran_state), (u8 *)Memory->TransientStorage + sizeof(tran_state));
        memory_arena *TranArena = &TranState->TranArena;
        
        // NOTE(ezexff): Init thread queues
        {
            TranState->HighPriorityQueue = Memory->HighPriorityQueue;
            TranState->LowPriorityQueue = Memory->LowPriorityQueue;
            for(u32 TaskIndex = 0;
                TaskIndex < ArrayCount(TranState->Tasks);
                ++TaskIndex)
            {
                task_with_memory *Task = TranState->Tasks + TaskIndex;
                
                Task->BeingUsed = false;
                SubArena(&Task->Arena, &TranState->TranArena, Megabytes(1));
            }
        }
        
        // NOTE(ezexff): Init assets
        {
            TranState->Assets = AllocateGameAssets(TranArena, Megabytes(128), TranState);
            Frame->InitializeSkyboxTexture = true;
        }
        
        // TODO(ezexff): Do test work
        task_with_memory *Task = BeginTaskWithMemory(TranState);
        if(Task)
        {
            test_work *Work = PushStruct(&Task->Arena, test_work);
            Work->Task = Task;
            
            Platform.AddEntry(TranState->LowPriorityQueue, TestWork, Work);
        }
        
        
        
        TranState->IsInitialized = true;
    }
    
    /*if(Input->dtForFrame > 0.1f)
    {
        Input->dtForFrame = 0.1f;
        // TODO(casey): Warn on out-of-range refresh
    }
    else if(Input->dtForFrame < 0.001f)
    {
        Input->dtForFrame = 0.001f;
        // TODO(casey): Warn on out-of-range refresh
    }*/
    
    {
        // NOTE(ezexff): Clear push buffer memory
        while(Frame->PushBufferSize--)
        {
            Frame->PushBufferMemory[Frame->PushBufferSize] = 0;
        }
        Frame->PushBufferSize = 0;
        
        Frame->Renderer = 0;
    }
    
    switch(GameState->GameModeID)
    {
        case GameMode_Test:
        {
            UpdateAndRenderTest(Memory);
            //UpdateAndRenderDebug(GameState, GameState->WorldMode, RenderCommands, Input, GameState->CutScene);
        } break;
        
        case GameMode_World:
        {
            //UpdateAndRenderWorld(GameState, GameState->WorldMode, Input, RenderCommands, &HitTest);
            UpdateAndRenderWorld(Memory, Input);
        } break;
        
        InvalidDefaultCase;
    }
    
    Assert(Frame->Renderer);
}

extern "C" GET_SOUND_SAMPLES_FUNC(GetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
#if ENGINE_INTERNAL
    if(GameState->IsTestSineWave)
    {
        OutputTestSineWave(GameState, SoundBuffer);
    }
    else
    {
        OutputPlayingSounds(&GameState->AudioState, SoundBuffer, TranState->Assets, &TranState->TranArena);
    }
#else
    OutputPlayingSounds(&GameState->AudioState, SoundBuffer, TranState->Assets, &TranState->TranArena);
#endif
    
    /*loaded_sound *LoadedSound = &GameState->LoadedSound;
    u32 SampleCount = LoadedSound->SampleCount;
    u32 ChannelCount = LoadedSound->ChannelCount;
    
    if(GameState->SampleIndex < LoadedSound->SampleCount)
    {
        s16 *SampleOut = SoundBuffer->Samples;
        for(int SampleIndex = 0;
            SampleIndex < SoundBuffer->SampleCount;
            ++SampleIndex)
        {
            u32 SoundSampleIndex = (GameState->SampleIndex + SampleIndex) % LoadedSound->SampleCount;
            s16 SampleValue = LoadedSound->Samples[0][SoundSampleIndex];
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;
        }
        
        GameState->SampleIndex += SoundBuffer->SampleCount;
    }
    else
    {
        int WavePeriod = SoundBuffer->SamplesPerSecond / GameState->ToneHz;
        
        s16 *SampleOut = SoundBuffer->Samples;
        for(int SampleIndex = 0;
            SampleIndex < SoundBuffer->SampleCount;
            ++SampleIndex)
        {
            r32 SineValue = Sin(GameState->tSine);
            s16 SampleValue = (s16)(SineValue * GameState->ToneVolume);
            *SampleOut++ = SampleValue;
            *SampleOut++ = SampleValue;
            
            GameState->tSine += 2.0f * Pi32 * 1.0f / (r32)WavePeriod;
            if(GameState->tSine > 2.0f * Pi32)
            {
                GameState->tSine -= 2.0f * Pi32;
            }
        }
    }*/
}

#if ENGINE_INTERNAL
#include "engine_debug.cpp"
#else
extern "C" DEBUG_GAME_FRAME_END(DEBUGGameFrameEnd)
{
}
#endif