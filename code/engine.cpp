#include "engine.h"

#include "engine_asset.cpp"

#include "engine_audio.cpp"
#include "engine_renderer.cpp"

#include "engine_entity.cpp"
#include "engine_world.cpp"
#include "engine_sim_region.cpp"

/* 
internal void
DEBUGTextLine(renderer_frame *Frame, game_assets *Assets, char *String, v2s P)
{
    font_id FontID = GetFirstFontFrom(Assets, Asset_Font);
    FontID.Value++;
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
        s32 AtX = 0 + P.x;
        s32 AtY = 0 + P.y;
        
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
                r32 TexCoords[8] = 
                {
                    0, 0,
                    1, 0,
                    1, 1,
                    0, 1,
                };
                PushBitmapOnScreen(Frame, Assets, BitmapID, Pos, GlyphDim, TexCoords);
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
 */

internal void
TestBubbleSort(void)
{
    u32 Array[10] = 
    {
        0, 5, 2, 4, 1, 
        6, 9, 2, 4, 5
    };
    u32 Count = ArrayCount(Array);
    
    for(u32 Outer = 0;
        Outer < Count;
        ++Outer)
    {
        b32 ListIsSorted = true;
        for(u32 Inner = 0;
            Inner < Count - 1;
            ++Inner)
        {
            u32 A = Array[Inner];
            u32 B = Array[Inner + 1];
            if(A > B)
            {
                Array[Inner] = B;
                Array[Inner + 1] = A;
                ListIsSorted = false;
            }
        }
        
        if(ListIsSorted)
        {
            break;
        }
    }
    
    u32 CheckResult = 321321;
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
#if ENGINE_IMGUI
    Log->Add("[enginework] TestWorkWithMemory\n");
#endif
#endif
    
    EndTaskWithMemory(Work->Task);
}

#if ENGINE_INTERNAL
debug_table *GlobalDebugTable;
#endif
extern "C" UPDATE_AND_RENDER_FUNC(UpdateAndRender)
{
    renderer_frame *Frame = &Memory->Frame;
    opengl *Opengl = &Frame->Opengl;
    
#if ENGINE_INTERNAL
    GlobalDebugTable = Memory->DebugTable;
    GlobalDebugMemory = Memory;
    GlobalDebugInput = Input;
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
#if ENGINE_IMGUI
            imgui *ImGuiHandle = &GlobalDebugMemory->ImGuiHandle;
            ImGui::SetCurrentContext(ImGuiHandle->ContextImGui);
            ImPlot::SetCurrentContext(ImGuiHandle->ContextImPlot);
            ImGui::SetAllocatorFunctions(ImGuiHandle->AllocFunc, ImGuiHandle->FreeFunc, ImGuiHandle->UserData);
            Log = &GlobalDebugMemory->ImGuiHandle.Log;
            /*Log->Add("[log init]: Hello %d world\n", 123);*/
            
            ImGuiHandle->LogMouseInput = false;
            ImGuiHandle->LogKeyboardInput = false;
#endif
#endif
        }
        
        // NOTE(ezexff): Set current game mode
        {
            //GameState->GameModeID = GameMode_World;
            GameState->GameModeID = GameMode_Test;
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
        
        // NOTE(ezexff): Init memory for push buffer
        //Frame->MaxPushBufferSize = Megabytes(1);
        Frame->MaxPushBufferSize = Kilobytes(4);
        Frame->PushBufferMemory = (u8 *)PushSize(ConstArena, Frame->MaxPushBufferSize);
        Frame->PushBufferBase = Frame->PushBufferMemory;
        
        // TODO(ezexff): test new push buffer
        Frame->TextPushBuffer = CreatePushBuffer(ConstArena, Kilobytes(200));
        
        // NOTE(ezexff): Screen shader program
        {
            DEBUGLoadShaders(ConstArena, &Frame->Shaders);
            Frame->CompileShaders = true;
        }
        
        // NOTE(ezexff): Set default frame frag effect
        {
            Frame->EffectID = FrameEffect_Normal;
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
        
        // NOTE(ezexff): init ui
        {
            UI_Init(&GameState->ConstArena, &TranState->TranArena);
        }
        
        // NOTE(ezexff): Init assets
        {
            TranState->Assets = AllocateGameAssets(TranArena, Megabytes(128), TranState);
        }
        
        // NOTE(ezexff): Load bitmaps
        {
            LoadBitmap(TranState->Assets, GetFirstBitmapFrom(TranState->Assets, Asset_Terrain), true);
            LoadBitmap(TranState->Assets, GetFirstBitmapFrom(TranState->Assets, Asset_DuDvMap), true);
            LoadBitmap(TranState->Assets, GetFirstBitmapFrom(TranState->Assets, Asset_NormalMap), true);
            
            for(u32 Index = 0;
                Index < 6;
                ++Index)
            {
                bitmap_id ID = {GetFirstAssetFrom(TranState->Assets, Asset_Skybox) + Index};
                LoadBitmap(TranState->Assets, ID, true);
            }
        }
        
        // TODO(ezexff): Do test work
        task_with_memory *Task = BeginTaskWithMemory(TranState);
        if(Task)
        {
            test_work *Work = PushStruct(&Task->Arena, test_work);
            Work->Task = Task;
            
            Platform.AddEntry(TranState->LowPriorityQueue, TestWork, Work);
        }
        
        // TODO(ezexff): Test sort
        TestBubbleSort();
        
        // NOTE(ezexff): Init world gen
        {
            GameState->GroundBufferWidth = 8;
            GameState->GroundBufferHeight = 8;
            GameState->TypicalFloorHeight = 3.0f;
        }
        
        // NOTE(ezexff): Init renderer
        {
            Frame->Renderer = PushStruct(&GameState->ConstArena, renderer);
            renderer *Renderer = (renderer *)Frame->Renderer;
            Renderer->FOV = 0.1f;
#define PLAYER_EYE_HEIGHT_FROM_GROUND 1.75f  // TODO(ezexff): Replace
            Renderer->Camera.P = V3(0, 0, PLAYER_EYE_HEIGHT_FROM_GROUND);
            Renderer->ClearColor = V4(0.5, 0.5, 0.5, 1);
            
            //~ NOTE(ezexff): Push buffers
            Renderer->PushBufferUI.Base = Renderer->PushBufferUI.Memory;
            Renderer->PushBufferUI.ElementCountMax = sizeof(Renderer->PushBufferUI.Memory) / 8; // min element size is 8 bytes (header + value)
            Renderer->PushBufferUI.SortEntryArray = PushArray(&TranState->TranArena, Renderer->PushBufferUI.ElementCountMax, tile_sort_entry);
            
            //~ NOTE(ezexff): Skybox
            Renderer->Skybox = PushStruct(&GameState->ConstArena, renderer_skybox);
            
            //~ NOTE(ezexff): Lighting
            Renderer->Lighting = PushStruct(&GameState->ConstArena, renderer_lighting);
            renderer_lighting *Lighting = Renderer->Lighting;
            Lighting->DirLight.Base.Color = V3(1.0f, 1.0f, 1.0f);
            Lighting->DirLight.Base.AmbientIntensity = 0.1f;
            Lighting->DirLight.Base.DiffuseIntensity = 1.0f;
            Lighting->DirLight.WorldDirection = V3(0.0f, 0.0f, -1.0f);
            
            //~ NOTE(ezexff): ShadowMap
            Renderer->ShadowMap = PushStruct(&GameState->ConstArena, renderer_shadowmap);
            renderer_shadowmap *ShadowMap = Renderer->ShadowMap;
            ShadowMap->Size = 60.0f;
            ShadowMap->NearPlane = -50.0f;
            ShadowMap->FarPlane = 100.0f;
            ShadowMap->CameraPitch = 60.0f;
            ShadowMap->CameraYaw = -120.0f;
            ShadowMap->CameraP = V3(0.0f, 0.0f, 0.0f);
            ShadowMap->Bias = 0.005f;
            
            //~ NOTE(ezexff): Water
            Renderer->Water = PushStruct(&GameState->ConstArena, renderer_water);
            renderer_water *Water = Renderer->Water;
            Water->WaveSpeed = 0.4f;
            Water->Tiling = 0.5f;
            Water->WaveStrength = 0.02f;
            Water->ShineDamper = 20.0f;
            Water->Reflectivity = 0.6f;
            
            //~ NOTE(ezexff): Terrain
            Renderer->Terrain = PushStruct(&GameState->ConstArena, renderer_terrain);
            renderer_terrain *Terrain = Renderer->Terrain;
            Terrain->TileCount = 16;
            Terrain->MaxHeight = 0.2f;
            Terrain->GroundBufferArray.Count = 64;
            
            Terrain->Material.Ambient = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Terrain->Material.Diffuse = V4(0.1f, 0.35f, 0.1f, 1.0f);
            Terrain->Material.Specular = V4(0.45f, 0.55f, 0.45f, 1.0f);
            Terrain->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Terrain->Material.Shininess = 0.25f;
            
            {
                Terrain->TileWidth = (r32)GameState->GroundBufferWidth / Terrain->TileCount;
                Terrain->TileHeight = (r32)GameState->GroundBufferHeight / Terrain->TileCount;
                Terrain->IndexArray.Count = (6 * Terrain->TileCount * Terrain->TileCount);
                Terrain->IndexArray.Indices = PushArray(&GameState->ConstArena, Terrain->IndexArray.Count, u32);
                
                // NOTE(ezexff): Create default index array
                u32 TmpPosCount = 0;
                u32 TmpIndCount = 0;
                for(u32 Y = 0;
                    Y <= Terrain->TileCount;
                    ++Y)
                {
                    for(u32 X = 0;
                        X <= Terrain->TileCount;
                        ++X)
                    {
                        if((X != Terrain->TileCount) && (Y != Terrain->TileCount))
                        {
                            Terrain->IndexArray.Indices[TmpIndCount + 0] = TmpPosCount;
                            Terrain->IndexArray.Indices[TmpIndCount + 1] = TmpPosCount + 1;
                            Terrain->IndexArray.Indices[TmpIndCount + 2] = TmpPosCount + Terrain->TileCount + 1;
                            
                            Terrain->IndexArray.Indices[TmpIndCount + 3] = TmpPosCount + 1;
                            Terrain->IndexArray.Indices[TmpIndCount + 4] = TmpPosCount + Terrain->TileCount + 1;
                            Terrain->IndexArray.Indices[TmpIndCount + 5] = TmpPosCount + Terrain->TileCount + 2;
                            
                            TmpIndCount += 6;
                        }
                        TmpPosCount++;
                    }
                }
                Assert(TmpIndCount == Terrain->IndexArray.Count);
                
                ground_buffer_array *GroundBufferArray = &Terrain->GroundBufferArray;
                GroundBufferArray->VertexCount = (Terrain->TileCount + 1) * (Terrain->TileCount + 1);
                GroundBufferArray->Buffers = PushArray(&GameState->ConstArena, GroundBufferArray->Count, ground_buffer);
                for(u32 GroundBufferIndex = 0;
                    GroundBufferIndex < GroundBufferArray->Count;
                    ++GroundBufferIndex)
                {
                    ground_buffer *GroundBuffer = GroundBufferArray->Buffers + GroundBufferIndex;
                    
                    GroundBuffer->IsInitialized = false;
                    GroundBuffer->IsFilled = false;
                    GroundBuffer->P = NullPosition();
                    GroundBuffer->OffsetP = V3(0, 0, 0);
                    GroundBuffer->Vertices = PushArray(&GameState->ConstArena, GroundBufferArray->VertexCount, vbo_vertex);
                    
                    // NOTE(ezexff): Fill default chunk positions and indices
                    u32 TmpPosCount2 = 0;
                    for(u32 Y = 0;
                        Y <= Terrain->TileCount;
                        ++Y)
                    {
                        for(u32 X = 0;
                            X <= Terrain->TileCount;
                            ++X)
                        {
                            GroundBuffer->Vertices[TmpPosCount2].Position.x = X * Terrain->TileWidth;
                            GroundBuffer->Vertices[TmpPosCount2].Position.y = Y * Terrain->TileHeight;
                            
                            GroundBuffer->Vertices[TmpPosCount2].TexCoords.x = (r32)X;
                            GroundBuffer->Vertices[TmpPosCount2].TexCoords.y = (r32)Y;
                            
                            TmpPosCount2++;
                        }
                    }
                    Assert(TmpPosCount2 == GroundBufferArray->VertexCount);
                }
            }
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
    
    BEGIN_BLOCK("UI_BeginFrame");
    UI_BeginFrame(GameState, TranState, Frame, Input);
    END_BLOCK();
    
    switch(GameState->GameModeID)
    {
        case GameMode_Test:
        {
            UpdateAndRenderTest(Memory, Input);
            //UpdateAndRenderDebug(GameState, GameState->WorldMode, RenderCommands, Input, GameState->CutScene);
        } break;
        
        case GameMode_World:
        {
            //UpdateAndRenderWorld(GameState, GameState->WorldMode, Input, RenderCommands, &HitTest);
            UpdateAndRenderWorld(Memory, Input);
        } break;
        
        InvalidDefaultCase;
    }
    
    // NOTE(ezexff): test ui
#if 1
    {
        local b32 IsWindowVisible4 = true;
        if(IsWindowVisible4)
        {
            UI_BeginWindow("TestPhysics", &IsWindowVisible4);
            
            r32 FPS = 1 / UI_State->Input->dtForFrame;
            UI_Label("FPS = %.2f", FPS);
            
            local b32 TestMode = true;
            UI_CheckBox("TestCheckBox", &TestMode);
            if(TestMode)
            {
                GameState->GameModeID = GameMode_Test;
            }
            else
            {
                GameState->GameModeID = GameMode_World;
            }
            
            mode_test *ModeTest = &GameState->ModeTest;
            u32 EntityIndex = ModeTest->ControlledEntityArray[0].EntityIndex;
            test_entity *Entity = ModeTest->EntityArray + EntityIndex;
            UI_Label("EntityIndex = %d", EntityIndex);
            UI_Label("Type = %d", Entity->Type);
            UI_Label("ForceMagnitude = %.2f", Entity->ForceMagnitude);
            UI_Label("P = %.2f %.2f", Entity->P.x, Entity->P.y);
            UI_Label("dP = %.2f %.2f", Entity->dP.x, Entity->dP.y);
            UI_Label("ddP = %.2f %.2f", Entity->ddP.x, Entity->ddP.y);
            
            if(UI_IsPressed(UI_Button("Index++")))
            {
                ModeTest->ControlledEntityArray[0].EntityIndex++;
                ModeTest->ControlledEntityArray[0].EntityIndex = Clamp(0, ModeTest->ControlledEntityArray[0].EntityIndex, ArrayCount(ModeTest->EntityArray) - 1);
            }
            if(UI_IsPressed(UI_Button("Index--")))
            {
                ModeTest->ControlledEntityArray[0].EntityIndex--;
                ModeTest->ControlledEntityArray[0].EntityIndex = Clamp(0, ModeTest->ControlledEntityArray[0].EntityIndex, ArrayCount(ModeTest->EntityArray) - 1);
            }
            
            UI_EndWindow();
        }
    }
#endif
    
#if !ENGINE_INTERNAL
    UI_EndFrame();
    CheckArena(&GameState->ConstArena);
    CheckArena(&TranState->TranArena);
#endif
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