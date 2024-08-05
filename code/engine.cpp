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
            imgui *ImGuiHandle = &Memory->Frame.ImGuiHandle;
            ImGui::SetCurrentContext(ImGuiHandle->Context);
            ImGui::SetAllocatorFunctions(ImGuiHandle->AllocFunc, ImGuiHandle->FreeFunc, ImGuiHandle->UserData);
            Log = &Memory->Frame.ImGuiHandle.Log;
            /*Log->Add("[log init]: Hello %d world\n", 123);*/
            
            ImGuiHandle->LogMouseInput = false;
            ImGuiHandle->LogKeyboardInput = false;
#endif
        }
        
        // NOTE(ezexff): Set current game mode
        {
            GameState->GameMode = GameMode_World;
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
        
        // NOTE(ezexff): Terrain options
        // TODO(ezexff): Mb rework?
        GameState->GroundBufferWidth = 8;
        GameState->GroundBufferHeight = 8;
        GameState->TypicalFloorHeight = 3.0f;
        
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
        
        // NOTE(ezexff): Init ground buffers
        {
            Frame->GroundBufferCount = 64;
            Frame->TileCount = 16;
            Frame->MaxTerrainHeight = 0.2f;
            
            Frame->TileWidth = (r32)GameState->GroundBufferWidth / Frame->TileCount;
            Frame->TileHeight = (r32)GameState->GroundBufferHeight / Frame->TileCount;
            
            
            Frame->ChunkVertexCount = (Frame->TileCount + 1) * (Frame->TileCount + 1);
            //Frame->ChunkPositionsXY = PushArray(TranArena, Frame->ChunkVertexCount, v2);
            
            Frame->ChunkIndexCount = (6 * Frame->TileCount * Frame->TileCount);
            Frame->ChunkIndices = PushArray(TranArena, Frame->ChunkIndexCount, u32);
            u32 TmpPosCount = 0;
            u32 TmpIndCount = 0;
            for(u32 Y = 0;
                Y <= Frame->TileCount;
                ++Y)
            {
                for(u32 X = 0;
                    X <= Frame->TileCount;
                    ++X)
                {
                    if((X != Frame->TileCount) && (Y != Frame->TileCount))
                    {
                        Frame->ChunkIndices[TmpIndCount + 0] = TmpPosCount;
                        Frame->ChunkIndices[TmpIndCount + 1] = TmpPosCount + 1;
                        Frame->ChunkIndices[TmpIndCount + 2] = TmpPosCount + Frame->TileCount + 1;
                        
                        Frame->ChunkIndices[TmpIndCount + 3] = TmpPosCount + 1;
                        Frame->ChunkIndices[TmpIndCount + 4] = TmpPosCount + Frame->TileCount + 1;
                        Frame->ChunkIndices[TmpIndCount + 5] = TmpPosCount + Frame->TileCount + 2;
                        
                        TmpIndCount += 6;
                    }
                    TmpPosCount++;
                }
            }
            Assert(TmpIndCount == Frame->ChunkIndexCount);
            
            Frame->GroundBuffers = PushArray(TranArena, Frame->GroundBufferCount, ground_buffer);
            //Frame->GroundBuffers = PushArray(TranArena, Frame->GroundBufferCount, ground_buffer);
            for(u32 GroundBufferIndex = 0;
                GroundBufferIndex < Frame->GroundBufferCount;
                ++GroundBufferIndex)
            {
                ground_buffer *GroundBuffer = Frame->GroundBuffers + GroundBufferIndex;
                
                GroundBuffer->IsInitialized = false;
                GroundBuffer->IsFilled = false;
                
                GroundBuffer->P = NullPosition();
                
                GroundBuffer->OffsetP = V3(0, 0, 0); // смещение относительно камеры для рендера чанка
                
                // заполнение в fillgroundchunk
                //GroundBuffer->PositionsZ = PushArray(TranArena, Frame->ChunkVertexCount, r32);
                //GroundBuffer->Normals = PushArray(TranArena, Frame->ChunkVertexCount, v3);
                //GroundBuffer->TexCoords = PushArray(TranArena, Frame->ChunkVertexCount, v2);
                GroundBuffer->Vertices = PushArray(TranArena, Frame->ChunkVertexCount, vbo_vertex);
                
                // NOTE(ezexff): Fill default chunk positions and indices
                u32 TmpPosCount2 = 0;
                //u32 TmpIndCount = 0;
                for(u32 Y = 0;
                    Y <= Frame->TileCount;
                    ++Y)
                {
                    for(u32 X = 0;
                        X <= Frame->TileCount;
                        ++X)
                    {
                        GroundBuffer->Vertices[TmpPosCount2].Position.x = X * Frame->TileWidth;
                        GroundBuffer->Vertices[TmpPosCount2].Position.y = Y * Frame->TileHeight;
                        
                        GroundBuffer->Vertices[TmpPosCount2].TexCoords.x = (r32)X;
                        GroundBuffer->Vertices[TmpPosCount2].TexCoords.y = (r32)Y;
                        //GroundBuffer->Vertices[TmpPosCount].Position.z = 0;
                        
                        /*if((X != Frame->TileCount) && (Y != Frame->TileCount))
                        {
                            Frame->ChunkIndices[TmpIndCount + 0] = TmpPosCount;
                            Frame->ChunkIndices[TmpIndCount + 1] = TmpPosCount + 1;
                            Frame->ChunkIndices[TmpIndCount + 2] = TmpPosCount + Frame->TileCount + 1;
                            
                            Frame->ChunkIndices[TmpIndCount + 3] = TmpPosCount + 1;
                            Frame->ChunkIndices[TmpIndCount + 4] = TmpPosCount + Frame->TileCount + 1;
                            Frame->ChunkIndices[TmpIndCount + 5] = TmpPosCount + Frame->TileCount + 2;
                            
                            TmpIndCount += 6;
                        }*/
                        TmpPosCount2++;
                    }
                }
                Assert(TmpPosCount2 == Frame->ChunkVertexCount);
                
                //GroundBuffer->RandomZ = 0.0f;
                // GroundBuffer->Bitmap = MakeEmptyBitmap(TranArena, GroundBufferWidth, GroundBufferHeight, false);
                // GroundBuffer->Texture = U32Max;
                //s32 GroundTextureSizeMultiplyer = 32;
                //GroundBuffer->DrawBuffer.Width = (s32)GroundBufferWidth * GroundTextureSizeMultiplyer;
                //GroundBuffer->DrawBuffer.Height = (s32)GroundBufferHeight * GroundTextureSizeMultiplyer;
                //glGenTextures(1, &GroundBuffer->DrawBuffer.ID);
                //glGenTextures(1, &GroundBuffer->DrawBuffer.DepthTexture);
                //GroundBuffer->DrawBuffer.FBO = Buffer->GroundFBO;
                
                //GroundBuffer->TerrainModel = CreateTerrainChunkModel(TranArena, GroundBufferWidth, GroundBufferHeight);
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
    
    switch(GameState->GameMode)
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
    
    //~ NOTE(ezexff): ImGui windows
#if ENGINE_INTERNAL
    imgui *ImGuiHandle = &Frame->ImGuiHandle;
    
    if(ImGuiHandle->ShowImGuiWindows)
    {
        // NOTE(ezexff): ImGui game window
        if(ImGuiHandle->ShowGameWindow)
        {
            ImGui::Begin("Game", &ImGuiHandle->ShowGameWindow);
            ImGui::Text("Debug window for game layer...");
            
            ImGui::SeparatorText("Subsystems");
            if(ImGui::CollapsingHeader("Memory"))
            {
                ImGui::SeparatorText("ConstArena");
                ImGui::BulletText("Size = %d MB or %d KB or %d bytes",
                                  GameState->ConstArena.Size / Megabytes(1),
                                  GameState->ConstArena.Size / Kilobytes(1),
                                  GameState->ConstArena.Size);
                ImGui::BulletText("Used = %d MB or %d KB or %d bytes",
                                  GameState->ConstArena.Used / Megabytes(1),
                                  GameState->ConstArena.Used / Kilobytes(1),
                                  GameState->ConstArena.Used);
                ImGui::Spacing();
                ImGui::SeparatorText("TranArena");
                ImGui::BulletText("Size = %d MB or %d KB or %d bytes",
                                  TranState->TranArena.Size / Megabytes(1),
                                  TranState->TranArena.Size / Kilobytes(1),
                                  TranState->TranArena.Size);
                ImGui::BulletText("Used = %d MB or %d KB or %d bytes",
                                  TranState->TranArena.Used / Megabytes(1),
                                  TranState->TranArena.Used / Kilobytes(1),
                                  TranState->TranArena.Used);
                ImGui::BulletText("TempCount = %d", TranState->TranArena.TempCount);
            }
            
            if(ImGui::CollapsingHeader("Audio"))
            {
                audio_state *AudioState = &GameState->AudioState;
                if(AudioState)
                {
                    float *MasterVolume = (float *)&AudioState->MasterVolume;
                    ImGui::DragFloat2("MasterVolume", MasterVolume, 0.01f, 0.0f, 1.0f);
                }
                else
                {
                    ImGui::BulletText("AudioState isn't initialized");
                }
                
                ImGui::SeparatorText("PlayingSounds");
                playing_sound *PlayingSound = AudioState->FirstPlayingSound;
                u32 PlayingSoundCount = 0;
                if(PlayingSound)
                {
                    for(;;)
                    {
                        ImGui::SetNextItemOpen(true);
                        if(ImGui::TreeNode((void *)(intptr_t)PlayingSoundCount, "Sound%d", PlayingSoundCount))
                        {
                            ImGui::BulletText("SoundID = %d", PlayingSound->ID.Value);
                            ImGui::BulletText("CurrentVolume = %f %f", 
                                              PlayingSound->CurrentVolume.x, PlayingSound->CurrentVolume.y);
                            ImGui::BulletText("dCurrentVolume = %f %f", 
                                              PlayingSound->dCurrentVolume.x, PlayingSound->dCurrentVolume.y);
                            ImGui::BulletText("TargetVolume = %f %f", PlayingSound->TargetVolume.x, PlayingSound->TargetVolume.y);
                            ImGui::BulletText("dSample = %f", PlayingSound->dSample);
                            ImGui::BulletText("SamplesPlayed = %f", PlayingSound->SamplesPlayed);
                            r32 dSample = PlayingSound->dSample;
                            ImGui::InputFloat("Pitch", &dSample, 0.01f, 1.0f, "%.3f");
                            ChangePitch(AudioState, PlayingSound, dSample);
                            float *Volume = (float *)&PlayingSound->CurrentVolume;
                            ImGui::DragFloat2("Volume", Volume, 0.01f, 0.0f, 1.0f);
                            r32 FadeDurationInSeconds = 1.0f;
                            v2 NewVolume = V2(Volume[0], Volume[1]);
                            ChangeVolume(AudioState, PlayingSound, FadeDurationInSeconds, NewVolume);
                            
                            ImGui::TreePop();
                            ImGui::Spacing();
                        }
                        
                        PlayingSound = PlayingSound->Next;
                        PlayingSoundCount++;
                        if(!PlayingSound)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    ImGui::BulletText("Sounds isn't playing");
                }
                
                ImGui::SeparatorText("TestSineWave");
                ImGui::Checkbox("PlaySineWave", &GameState->IsTestSineWave);
                ImGui::SliderInt("tone hz", &GameState->ToneHz, 20, 20000);
                int ToneVolume = (int)GameState->ToneVolume;
                ImGui::SliderInt("tone volume", &ToneVolume, 0, 20000);
                GameState->ToneVolume = (s16)ToneVolume;
            }
            
            if(ImGui::CollapsingHeader("Frame"))
            {
                ImGui::BulletText("Dim = %dx%d", Frame->Dim.x, Frame->Dim.y);
                ImGui::BulletText("AspectRatio = %.3f", Frame->AspectRatio);
                ImGui::InputFloat("FOV##Frame", &Frame->FOV, 0.01f, 1.0f, "%.3f");
                float *WorldOrigin = (float *)&Frame->WorldOrigin;
                ImGui::DragFloat3("WorldOrigin##Frame", WorldOrigin, 0.01f, -100.0f, 100.0f);
                float *ClearColor = (float *)&Frame->ClearColor;
                ImGui::ColorEdit4("ClearColor##Frame", ClearColor);
                
                ImGui::SeparatorText("PushBuffer");
                ImGui::BulletText("MaxSize = %d", Frame->MaxPushBufferSize);
                ImGui::BulletText("Size = %d", Frame->PushBufferSize);
                
                ImGui::SeparatorText("Camera");
                camera *Camera = &Frame->Camera;
                ImGui::BulletText("Pos = %.3f %.3f %.3f", Camera->P.x, Camera->P.y, Camera->P.z);
                ImGui::BulletText("Ang = %.3f %.3f %.3f", Camera->Angle.x, Camera->Angle.y, Camera->Angle.z);
                ImGui::BulletText("PitchInverted = %.3f", Frame->CameraPitchInverted);
                
                ImGui::SeparatorText("Textures");
                ImVec2 WinDim = ImGui::GetWindowSize();
                s32 ScrollBarSize = 100;
                r32 AspectRatio = Frame->Dim.x / (WinDim.x - ScrollBarSize);
                
                if(ImGui::TreeNode("Color"))
                {
                    ImGui::Image((void *)(intptr_t)Frame->ColorTexture,
                                 ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                 ImVec2(0, 0), ImVec2(1, -1));
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                if(ImGui::TreeNode("Depth"))
                {
                    ImGui::Image((void *)(intptr_t)Frame->DepthTexture,
                                 ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                 ImVec2(0, 0), ImVec2(1, -1));
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                if(ImGui::TreeNode("ShadowMap"))
                {
                    ImGui::Image((void *)(intptr_t)Frame->ShadowMap,
                                 ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                 ImVec2(0, 0), ImVec2(1, -1));
                    
                    ImGui::InputFloat("Size##ShadowMap", &Frame->ShadowMapSize, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("NearPlane##ShadowMap", &Frame->ShadowMapNearPlane, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("FarPlane##ShadowMap", &Frame->ShadowMapFarPlane, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("CameraPitch##ShadowMap", &Frame->ShadowMapCameraPitch, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("CameraYaw##ShadowMap", &Frame->ShadowMapCameraYaw, 0.01f, 1.0f, "%.3f");
                    float *ShadowMapCameraPos = (float *)&Frame->ShadowMapCameraPos;
                    ImGui::DragFloat3("CameraPos##ShadowMap", ShadowMapCameraPos, 0.01f, -100.0f, 100.0f);
                    ImGui::InputFloat("Bias##ShadowMap", &Frame->ShadowMapBias, 0.001f, 1.0f, "%.3f");
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                if(ImGui::TreeNode("Water"))
                {
                    ImGui::Text("ReflectionColor");
                    ImGui::Image((void *)(intptr_t)Frame->WaterReflectionColorTexture,
                                 ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                 ImVec2(0, 0), ImVec2(1, -1));
                    
                    ImGui::Text("RefractionColor");
                    ImGui::Image((void *)(intptr_t)Frame->WaterRefractionColorTexture,
                                 ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                 ImVec2(0, 0), ImVec2(1, -1));
                    ImGui::Text("RefractionDepth");
                    ImGui::Image((void *)(intptr_t)Frame->WaterRefractionDepthTexture,
                                 ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                 ImVec2(0, 0), ImVec2(1, -1));
                    
                    ImGui::InputFloat("WaveSpeed##Water", &Frame->WaterWaveSpeed, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("Tiling##Water", &Frame->WaterTiling, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("WaveStrength##Water", &Frame->WaterWaveStrength, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("ShineDamper##Water", &Frame->WaterShineDamper, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("Reflectivity##Water", &Frame->WaterReflectivity, 0.01f, 1.0f, "%.3f");
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                ImGui::SeparatorText("FragEffects");
                ImGui::BulletText("EffectID = %d", Frame->FragEffect);
                char *FragEffects[] = 
                {
                    "Abberation",
                    "Blur",
                    "Emboss",
                    "Grayscale",
                    "Inverse",
                    "Sepia",
                    "Normal"
                };
                int RowCount = 4;
                int FragEffectsCount = IM_ARRAYSIZE(FragEffects);
                if(FragEffectsCount < RowCount)
                {
                    RowCount = FragEffectsCount;
                }
                ImGui::PushItemWidth(-1);
                ImGui::ListBox("##frageffects", &Frame->FragEffect, FragEffects, FragEffectsCount, RowCount);
                ImGui::PopItemWidth();
                
                ImGui::SeparatorText("EditShaders");
                ImGui::Checkbox("Frame shaders editor", &ImGuiHandle->ShowFrameShadersEditorWindow);
            }
            
            if(ImGui::CollapsingHeader("Input"))
            {
                // NOTE(ezexff): Mouse input
                ImGui::SeparatorText("Mouse");
                ImGui::BulletText("CursorPos = %d %d", Input->MouseP.x, Input->MouseP.y);
                ImGui::BulletText("CursorDelta = %d %d", Input->MouseDelta.x, Input->MouseDelta.y);
                ImGui::BulletText("ScrollDelta = %d", Input->MouseDelta.z);
                
                ImGui::Checkbox("Log mouse keys", &ImGuiHandle->LogMouseInput);
                
                if(ImGuiHandle->LogMouseInput)
                {
                    if(Input->MouseDelta.z != 0)
                    {
                        Log->Add("[engineinput] Scroll delta = %d\n", Input->MouseDelta.z);
                    }
                    
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
                    {
                        Log->Add("[engineinput] VK_LBUTTON was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Middle]))
                    {
                        Log->Add("[engineinput] VK_MBUTTON was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
                    {
                        Log->Add("[engineinput] VK_RBUTTON was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Extended0]))
                    {
                        Log->Add("[engineinput] VK_XBUTTON1 was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Extended1]))
                    {
                        Log->Add("[engineinput] VK_XBUTTON2 was pressed\n");
                    }
                }
                
                // NOTE(ezexff): Keyboard input
                ImGui::SeparatorText("Keyboard");
                ImGui::Checkbox("Log keyboard keys", &ImGuiHandle->LogKeyboardInput);
                
                if(ImGuiHandle->LogKeyboardInput)
                {
                    for(int ControllerIndex = 0;
                        ControllerIndex < ArrayCount(Input->Controllers);
                        ++ControllerIndex)
                    {
                        game_controller_input *Controller = GetController(Input, ControllerIndex);
                        
                        if(WasPressed(Controller->MoveUp))
                        {
                            Log->Add("[engineinput] MoveUp was pressed\n");
                        }
                        if(WasPressed(Controller->MoveDown))
                        {
                            Log->Add("[engineinput] MoveDown was pressed\n");
                        }
                        if(WasPressed(Controller->MoveLeft))
                        {
                            Log->Add("[engineinput] MoveLeft was pressed\n");
                        }
                        if(WasPressed(Controller->MoveRight))
                        {
                            Log->Add("[engineinput] MoveRight was pressed\n");
                        }
                        if(WasPressed(Controller->ActionUp))
                        {
                            Log->Add("[engineinput] ActionUp was pressed\n");
                        }
                        if(WasPressed(Controller->ActionDown))
                        {
                            Log->Add("[engineinput] ActionDown was pressed\n");
                        }
                        if(WasPressed(Controller->ActionLeft))
                        {
                            Log->Add("[engineinput] ActionLeft was pressed\n");
                        }
                        if(WasPressed(Controller->ActionRight))
                        {
                            Log->Add("[engineinput] ActionRight was pressed\n");
                        }
                        if(WasPressed(Controller->Start))
                        {
                            Log->Add("[engineinput] Start was pressed\n");
                        }
                    }
                }
            }
            
            if(ImGui::CollapsingHeader("Threads"))
            {
                ImGui::SeparatorText("Tasks");
                for(u32 TaskIndex = 0;
                    TaskIndex < ArrayCount(TranState->Tasks);
                    ++TaskIndex)
                {
                    task_with_memory *Task = TranState->Tasks + TaskIndex;
                    
                    ImGui::BulletText("#%d BeingUsed = %s", TaskIndex, Task->BeingUsed ? "true" : "false");
                    //Task->BeingUsed = false;
                    //SubArena(&Task->Arena, &TranState->TranArena, Megabytes(1));
                }
            }
            
            if(ImGui::CollapsingHeader("Assets"))
            {
                game_assets *Assets = TranState->Assets;
                
                ImGui::BulletText("FileCount = %d", Assets->FileCount);
                ImGui::BulletText("TagCount = %d", Assets->TagCount);
                ImGui::BulletText("AssetCount = %d", Assets->AssetCount);
                
                u32 BlockCount = 0;
                if(ImGui::TreeNode("Memory blocks"))
                {
                    asset_memory_block *MemoryBlock = &Assets->MemorySentinel;
                    while(MemoryBlock->Next != &Assets->MemorySentinel)
                    {
                        MemoryBlock = MemoryBlock->Next;
                        ImGui::BulletText("Block #%d Size = %d", BlockCount, MemoryBlock->Size);
                        BlockCount++;
                    }
                    ImGui::BulletText("BlockCount = %d", BlockCount);
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                ImGui::SeparatorText("AssetCountByType");
                char *TypeNames[] =
                {
                    "None",
                    "Grass",
                    "Tuft",
                    "Stone",
                    "Clip",
                    "Ground",
                    "Skybox",
                    "Terrain",
                    "DuDvMap",
                    "NormalMap",
                    "Bloop",
                    "Music",
                    "Font",
                    "FontGlyph"
                };
                u32 TypeNamesCount = ArrayCount(TypeNames);
                Assert(TypeNamesCount == Asset_Count);
                
                for(u32 TypeID = 1;
                    TypeID < Asset_Count;
                    TypeID++)
                {
                    
                    asset_type *Type = Assets->AssetTypes + TypeID;
                    u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                    if(AssetCount > 0)
                    {
                        ImGui::BulletText("%s = %d", TypeNames[TypeID], AssetCount);
                    }
                }
                
                ImGui::SeparatorText("EAB tree");
                if(ImGui::TreeNode("Bitmaps"))
                {
                    for(u32 TypeID = Asset_Grass;
                        TypeID <= Asset_NormalMap;
                        TypeID++)
                    {
                        asset_type *Type = Assets->AssetTypes + TypeID;
                        u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                        
                        if(AssetCount > 0)
                        {
                            if(ImGui::TreeNode((void *)(intptr_t)TypeID, "%s", TypeNames[TypeID]))
                            {
                                for(u32 AssetIndex = Type->FirstAssetIndex;
                                    AssetIndex < Type->OnePastLastAssetIndex;
                                    AssetIndex++)
                                {
                                    asset *Asset = Assets->Assets + AssetIndex;
                                    eab_bitmap EABBitmap = Asset->EAB.Bitmap;
                                    ImGui::BulletText("#%d UnicodeCodepoint = Dim = %dx%d BPS = %d State = %d", AssetIndex,
                                                      EABBitmap.Dim[0], EABBitmap.Dim[1], EABBitmap.BytesPerPixel,
                                                      Asset->State);
                                    ImGui::SameLine();
                                    
                                    bitmap_id ID = {};
                                    ID.Value = AssetIndex;
                                    if(Asset->State == AssetState_Loaded)
                                    {
                                        loaded_bitmap *Bitmap = GetBitmap(Assets, ID, true);
                                        ImGui::PushID(AssetIndex);
                                        if(ImGui::Button("preview"))
                                        {
                                            Frame->Preview = *Bitmap;
                                            ImGuiHandle->ShowBitmapPreviewWindow = true;
                                            ImGui::SetWindowFocus("Bitmap preview");
                                        }
                                        ImGui::PopID();
                                    }
                                    else if(Asset->State == AssetState_Unloaded)
                                    {
                                        ImGui::Text("LOADING...");
                                        LoadBitmap(Assets, ID, true);
                                        Frame->MissingResourceCount++;
                                    }
                                }
                                
                                ImGui::TreePop();
                                ImGui::Spacing();
                            }
                        }
                    }
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                if(ImGui::TreeNode("Fonts"))
                {
                    {
                        font_id FirstFont = GetFirstFontFrom(TranState->Assets, Asset_Font);
                        
                        u32 FirstFontAsset = GetFirstAssetFrom(TranState->Assets, Asset_Font);
                        asset *Asset = Assets->Assets + FirstFontAsset;
                        
                        if(Asset->State == AssetState_Loaded)
                        {
                            loaded_font *Font = GetFont(Assets, FirstFont, true);
                        }
                        else if(Asset->State == AssetState_Unloaded)
                        {
                            LoadFont(TranState->Assets, FirstFont, true);
                        }
                    }
                    {
                        u32 TypeID = Asset_Font;
                        asset_type *Type = Assets->AssetTypes + TypeID;
                        u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                        if(AssetCount > 0)
                        {
                            for(u32 AssetIndex = Type->FirstAssetIndex;
                                AssetIndex < Type->OnePastLastAssetIndex;
                                AssetIndex++)
                            {
                                asset *Asset = Assets->Assets + AssetIndex;
                                eab_font EABFont = Asset->EAB.Font;
                                
                                ImGui::Text("Font#%d", AssetIndex);
                                
                                int fds32432 = 0;
                            }
                        }
                    }
                    
                    {
                        u32 TypeID = Asset_FontGlyph;
                        asset_type *Type = Assets->AssetTypes + TypeID;
                        u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                        if(AssetCount > 0)
                        {
                            if(ImGui::TreeNode((void *)(intptr_t)TypeID, "%s", TypeNames[TypeID]))
                            {
                                for(u32 AssetIndex = Type->FirstAssetIndex;
                                    AssetIndex < Type->OnePastLastAssetIndex;
                                    AssetIndex++)
                                {
                                    asset *Asset = Assets->Assets + AssetIndex;
                                    eab_bitmap EABBitmap = Asset->EAB.Bitmap;
                                    ImGui::BulletText("#%d Dim = %dx%d BPS = %d State = %d", AssetIndex,
                                                      EABBitmap.Dim[0], EABBitmap.Dim[1], EABBitmap.BytesPerPixel,
                                                      Asset->State);
                                    ImGui::SameLine();
                                    
                                    bitmap_id ID = {};
                                    ID.Value = AssetIndex;
                                    if(Asset->State == AssetState_Loaded)
                                    {
                                        loaded_bitmap *Bitmap = GetBitmap(Assets, ID, true);
                                        ImGui::PushID(AssetIndex);
                                        if(ImGui::Button("preview"))
                                        {
                                            Frame->Preview = *Bitmap;
                                            ImGuiHandle->ShowBitmapPreviewWindow = true;
                                            ImGui::SetWindowFocus("Bitmap preview");
                                        }
                                        ImGui::PopID();
                                    }
                                    else if(Asset->State == AssetState_Unloaded)
                                    {
                                        ImGui::Text("LOADING...");
                                        LoadBitmap(Assets, ID, true);
                                        Frame->MissingResourceCount++;
                                    }
                                    
                                    /*char *TagNames[] = 
                                    {
                                        "Opacity",
                                        "Face",
                                        "UnicodeCodepoint",
                                    };
                                    
                                    for(u32 TagIndex = Asset->EAB.FirstTagIndex;
                                        TagIndex < Asset->EAB.OnePastLastTagIndex;
                                        TagIndex++)
                                    {
                                        eab_tag Tag = Assets->Tags[TagIndex];
                                        ImGui::Text("  Tag#%d ID = %s Value = %.3f", 
                                                    TagIndex, TagNames[Tag.ID], Tag.Value);
                                    }*/
                                }
                                
                                ImGui::TreePop();
                                ImGui::Spacing();
                            }
                        }
                    }
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                if(ImGui::TreeNode("Sounds"))
                {
                    for(u32 TypeID = Asset_Bloop;
                        TypeID <= Asset_Music;
                        TypeID++)
                    {
                        asset_type *Type = Assets->AssetTypes + TypeID;
                        u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                        if(AssetCount > 0)
                        {
                            if(ImGui::TreeNode((void *)(intptr_t)TypeID, "%s", TypeNames[TypeID]))
                            {
                                for(u32 AssetIndex = Type->FirstAssetIndex;
                                    AssetIndex < Type->OnePastLastAssetIndex;
                                    AssetIndex++)
                                {
                                    asset *Asset = Assets->Assets + AssetIndex;
                                    eab_sound Sound = Asset->EAB.Sound;
                                    ImGui::BulletText("#%d SampleCount = %d ChannelCount = %d", AssetIndex,
                                                      Sound.SampleCount, Sound.ChannelCount);
                                    ImGui::SameLine();
                                    
                                    sound_id ID = {};
                                    ID.Value = AssetIndex;
                                    ImGui::PushID(AssetIndex);
                                    if(ImGui::Button("play"))
                                    {
                                        GameState->PlayingSound = PlaySound(&GameState->AudioState, ID);
                                    }
                                    ImGui::PopID();
                                    /*if(GetState(Asset) == AssetState_Loaded)
                                    {
                                        loaded_sound *Sound = GetSound(TranState->Assets, ID);
                                        ImGui::PushID(AssetIndex);
                                        if(ImGui::Button("play"))
                                        {
                                            GameState->PlayingSound = PlaySound(&GameState->AudioState, ID);
                                        }
                                        ImGui::PopID();
                                    }
                                    else if(GetState(Asset) == AssetState_Unloaded)
                                    {
                                        ImGui::Text("LOADING...");
                                        LoadSound(Assets, ID);
                                        Frame->MissingResourceCount++;
                                    }*/
                                }
                                ImGui::TreePop();
                                ImGui::Spacing();
                            }
                        }
                    }
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
            }
            
            if(ImGui::CollapsingHeader("Mode"))
            {
                ImGui::SeparatorText("Change mode");
                char *GameModes[] = 
                {
                    "Test",
                    "World"
                };
                int RowCount = 4;
                int ModeCount = IM_ARRAYSIZE(GameModes);
                if(ModeCount < RowCount)
                {
                    RowCount = ModeCount;
                }
                int CurrentMode = GameState->GameMode;
                ImGui::PushItemWidth(-1);
                ImGui::ListBox("##gamemode", &CurrentMode, GameModes, ModeCount, RowCount);
                ImGui::PopItemWidth();
                GameState->GameMode = CurrentMode;
                
                //camera *Camera = 0;
                switch(GameState->GameMode)
                {
                    case GameMode_Test:
                    {
                        //Camera = &GameState->ModeTest.Camera;
                    } break;
                    
                    case GameMode_World:
                    {
                        //Camera = &GameState->ModeWorld.Camera;
                        ImGui::SeparatorText("SimRegion");
                        ImGui::Checkbox("ShowSimRegionWindow", &ImGuiHandle->ShowSimRegionWindow);
                        ImGui::Checkbox("DrawSpaceBounds (white)", &ImGuiHandle->DrawSpaceBounds);
                        ImGui::Checkbox("DrawCameraBounds (yellow)", &ImGuiHandle->DrawCameraBounds);
                        ImGui::Checkbox("DrawSimBounds (cyan)", &ImGuiHandle->DrawSimBounds);
                        ImGui::Checkbox("DrawSimRegionBounds (orange)", &ImGuiHandle->DrawSimRegionBounds);
                        ImGui::Checkbox("DrawSimRegionUpdatableBounds (purple)", &ImGuiHandle->DrawSimRegionUpdatableBounds);
                        ImGui::Checkbox("DrawGroundBufferBounds (blue)", &ImGuiHandle->DrawGroundBufferBounds);
                        
                        ImGui::SeparatorText("Skybox");
                        ImGui::Checkbox("Visibility##Skybox", &Frame->DrawSkybox);
                        
                        ImGui::SeparatorText("Terrain");
                        s32 TilesPerChunkRow = GameState->TilesPerChunkRow;
                        if(ImGui::InputInt("TilesPerChunkRow", &TilesPerChunkRow))
                        {
                            if(TilesPerChunkRow < 1)
                            {
                                TilesPerChunkRow = 1;
                            } 
                            else if(TilesPerChunkRow > 128)
                            {
                                TilesPerChunkRow = 128;
                            }
                            GameState->TilesPerChunkRow = TilesPerChunkRow;
                        }
                        ImGui::InputFloat("MaxTerrainHeight", &GameState->MaxTerrainHeight, 0.01f, 1.0f, "%.3f");
                        ImGui::Checkbox("Visibility##Terrain", &Frame->DrawTerrain);
                        ImGui::Checkbox("GL_LINE", &Frame->IsTerrainInLinePolygonMode);
                        ImGui::Checkbox("FixCameraOnTerrain", &Frame->FixCameraOnTerrain);
                        ImGui::Text("Material");
                        float *TerrainMaterialAmbient = (float *)&Frame->TerrainMaterial.Ambient;
                        ImGui::DragFloat4("Ambient##Terrain", TerrainMaterialAmbient, 0.01f, -100.0f, 100.0f);
                        float *TerrainMaterialDiffuse = (float *)&Frame->TerrainMaterial.Diffuse;
                        ImGui::DragFloat4("Diffuse##Terrain", TerrainMaterialDiffuse, 0.01f, -100.0f, 100.0f);
                        float *TerrainMaterialSpecular = (float *)&Frame->TerrainMaterial.Specular;
                        ImGui::DragFloat4("Specular##Terrain", TerrainMaterialSpecular, 0.01f, -100.0f, 100.0f);
                        float *TerrainMaterialEmission = (float *)&Frame->TerrainMaterial.Emission;
                        ImGui::DragFloat4("Emission##Terrain", TerrainMaterialEmission, 0.01f, -100.0f, 100.0f);
                        ImGui::InputFloat("Shininess##Terrain", &Frame->TerrainMaterial.Shininess, 0.01f, 1.0f, "%.3f");
                        
                        ImGui::SeparatorText("DEBUGTextLine");
                        ImGui::Checkbox("Visibility##DEBUGTextLine", &Frame->DrawDebugTextLine);
                        
                        ImGui::SeparatorText("DirLight");
                        float *DirLightBaseColor = (float *)&Frame->DirLight.Base.Color;
                        ImGui::ColorEdit3("Color##DirLight", DirLightBaseColor);
                        ImGui::InputFloat("AmbientIntensity##TerrainDirLight", &Frame->DirLight.Base.AmbientIntensity, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("DiffuseIntensity##TerrainDirLight", &Frame->DirLight.Base.DiffuseIntensity, 0.01f, 1.0f, "%.3f");
                        
                        float *DirLightWorldDirection = (float *)&Frame->DirLight.WorldDirection;
                        ImGui::DragFloat3("WorldDirection##TerrainDirLight", DirLightWorldDirection, 0.01f, -100.0f, 100.0f);
                        
                        float *TestSun2P = (float *)&Frame->TestSun2P;
                        ImGui::DragFloat3("TestSun2P##TerrainDirLight", TestSun2P, 0.01f, -100.0f, 100.0f);
                        
                        ImGui::SeparatorText("Test");
                        ImGui::Checkbox("PushBufferWithLight##Test", &Frame->PushBufferWithLight);
                        
                    } break;
                    
                    InvalidDefaultCase;
                }
                /*if(Camera != 0)
                {
                ImGui::SeparatorText("Camera");
                    ImGui::Text("Pos");
                    ImGui::InputFloat("x", &Camera->P.x, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("y", &Camera->P.y, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("z", &Camera->P.z, 0.01f, 1.0f, "%.3f");
                    ImGui::Text("Angle");
                    ImGui::InputFloat("Pitch", &Camera->Angle.x, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("Yaw", &Camera->Angle.y, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("Roll", &Camera->Angle.z, 0.01f, 1.0f, "%.3f");
                }*/
                
                float *ClearColor = 0;
                switch(GameState->GameMode)
                {
                    case GameMode_Test:
                    {
                        ClearColor = (float*)&GameState->ModeTest.ClearColor;
                    } break;
                    
                    case GameMode_World:
                    {
                        //ClearColor = (float*)&GameState->ModeWorld.ClearColor;
                    } break;
                    
                    InvalidDefaultCase;
                }
                if(ClearColor != 0)
                {
                    ImGui::SeparatorText("Background");
                    ImGui::ColorEdit4("clear color", ClearColor);
                }
            }
            
            ImGui::End();
        }
        
        
        // NOTE(ezexff): Shaders editor window
        if(ImGuiHandle->ShowFrameShadersEditorWindow)
        {
            ImGui::Begin("Frame shaders editor", &ImGuiHandle->ShowFrameShadersEditorWindow);
            ImGui::Text("Debug window for frame shaders...");
            
            ImGui::SeparatorText("Vertex");
            ImGui::Text("ID = %d", Frame->Vert.ID);
            static ImGuiInputTextFlags VertFlags = ImGuiInputTextFlags_AllowTabInput;
            // NOTE(ezexff): InputTextMultiline(label, text, bufferSize, [w=0, h=0, ImGuiInputTextFlags=0])
            if(ImGui::InputTextMultiline("##vert", (char *)Frame->Vert.Text,
                                         IM_ARRAYSIZE(Frame->Vert.Text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), VertFlags))
            {
                //Log->Add("Shader InputTextMultiline changed\n");
            }
            ImGui::SeparatorText("Fragment");
            ImGui::Text("ID = %d", Frame->Frag.ID);
            static ImGuiInputTextFlags FragFlags = ImGuiInputTextFlags_AllowTabInput;
            // NOTE(ezexff): InputTextMultiline(label, text, bufferSize, [w=0, h=0, ImGuiInputTextFlags=0])
            if(ImGui::InputTextMultiline("##frag", (char *)Frame->Frag.Text,
                                         IM_ARRAYSIZE(Frame->Frag.Text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), FragFlags))
            {
                //Log->Add("Shader InputTextMultiline changed\n");
            }
            
            if(ImGui::Button("Recompile shaders and link program"))
            {
                Frame->CompileShaders = true;
                /*
OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->Vert);
                OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->Frag);
                OpenglLinkProgram(Opengl, &Frame->Program, &Frame->Vert, &Frame->Frag);
*/
            }
            ImGui::End();
        }
        
        // NOTE(ezexff): Bitmap preview window
        if(ImGuiHandle->ShowBitmapPreviewWindow)
        {
            v2 WindowMax = V2(512, 512);
            v2 WindowDim = V2((r32)Frame->Preview.Width, (r32)Frame->Preview.Height);
            if(WindowDim.x > WindowMax.x)
            {
                WindowDim.x = WindowMax.x;
            }
            if(WindowDim.y > WindowMax.y)
            {
                WindowDim.y = WindowMax.y;
            }
            
            ImGui::SetNextWindowSize(ImVec2(WindowDim.x + 15, WindowDim.y + 35));
            
            ImGuiWindowFlags Flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
            ImGui::Begin("Bitmap preview", &ImGuiHandle->ShowBitmapPreviewWindow, Flags);
            ImGui::Image((void *)(intptr_t)Frame->PreviewTexture,
                         ImVec2(WindowDim.x,WindowDim.y));
            
            ImGui::End();
        }
    }
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