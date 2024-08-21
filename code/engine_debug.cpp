#include "engine_debug.h"

internal void
UpdateAndRenderImgui()
{
    game_state *GameState = (game_state *)DebugGlobalMemory->PermanentStorage;
    tran_state *TranState = (tran_state *)DebugGlobalMemory->TransientStorage;
    debug_state *DebugState = (debug_state *)DebugGlobalMemory->DebugStorage;
    
    renderer_frame *Frame = &DebugGlobalMemory->Frame;
    game_input *Input = DebugGlobalInput;
    imgui *ImGuiHandle = &DebugGlobalMemory->ImGuiHandle;
    
    if(ImGuiHandle->ShowImGuiWindows)
    {
        if(ImGuiHandle->ShowGameWindow)
        {
            ImGui::Begin("Game", &ImGuiHandle->ShowGameWindow);
            ImGui::Text("Debug window for game layer...");
            
            ImGui::SeparatorText("Subsystems");
            if(ImGui::CollapsingHeader("Memory"))
            {
                ImGui::BulletText("TempCount = %d (begin or end w/o pair)", TranState->TranArena.TempCount);
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
                ImGui::SeparatorText("DebugArena");
                ImGui::BulletText("Size = %d MB or %d KB or %d bytes",
                                  DebugState->DebugArena.Size / Megabytes(1),
                                  DebugState->DebugArena.Size / Kilobytes(1),
                                  DebugState->DebugArena.Size);
                ImGui::BulletText("Used = %d MB or %d KB or %d bytes",
                                  DebugState->DebugArena.Used / Megabytes(1),
                                  DebugState->DebugArena.Used / Kilobytes(1),
                                  DebugState->DebugArena.Used);
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
                /* 
                                ImGui::InputFloat("FOV##Frame", &Frame->FOV, 0.01f, 1.0f, "%.3f");
                                float *WorldOrigin = (float *)&Frame->WorldOrigin;
                                ImGui::DragFloat3("WorldOrigin##Frame", WorldOrigin, 0.01f, -100.0f, 100.0f);
                                float *ClearColor = (float *)&Frame->ClearColor;
                                ImGui::ColorEdit4("ClearColor##Frame", ClearColor);
                 */
                
                ImGui::SeparatorText("PushBuffer");
                ImGui::BulletText("MaxSize = %d", Frame->MaxPushBufferSize);
                ImGui::BulletText("Size = %d", Frame->PushBufferSize);
                
                ImGui::SeparatorText("Renderer");
                if(Frame->Renderer)
                {
                    renderer *Renderer = (renderer *)Frame->Renderer;
                    
                    /* 
                                        renderer_camera *Camera = (renderer_camera *)Frame->Camera;
                                        if(Camera)
                                        {
                                            ImGui::SeparatorText("Camera");
                                            //ImGui::BulletText("Pos = %.3f %.3f %.3f", Camera->P.pitch, Camera->P.y, Camera->P.z);
                                            ImGui::BulletText("Ang = %.3f %.3f %.3f", Camera->Angle.x, Camera->Angle.y, Camera->Angle.z);
                                            ImGui::BulletText("PitchInverted = %.3f", Frame->CameraPitchInverted);
                                        }
                     */
                    
                    ImGui::SeparatorText("Flags");
                    {
                        u32 Size = sizeof(Renderer->Flags);
                        u8 *ValueByte = (u8 *)&Renderer->Flags;
                        char Bits[33] = {};
                        
                        u32 TmpCount = 0;
                        for(u32 ByteIndex = Size;
                            ByteIndex > 0;
                            ByteIndex--)
                        {
                            for(u32 BitIndex = 8;
                                BitIndex > 0;
                                --BitIndex)
                            {
                                char Character = (ValueByte[ByteIndex - 1] >> (BitIndex - 1)) & 1;
                                Bits[TmpCount] = Character ? '1' : '0';
                                TmpCount++;
                            }
                        }
                        Bits[TmpCount] = '\0';
                        ImGui::BulletText("%s\n", Bits);
                    }
                    
                    //ImGui::InputFloat("MaxTerrainHeight", &GameState->MaxTerrainHeight, 0.01f, 1.0f, "%.3f");
                    bool DrawTerrain = IsSet(Renderer, RendererFlag_Terrain);
                    if(ImGui::Checkbox("Visibility##Terrain", &DrawTerrain))
                    {
                        FlipFlag(Renderer, RendererFlag_Terrain);
                    }
                }
                
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
                                        if(ImGui::SmallButton("preview"))
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
                                        //Frame->MissingResourceCount++;
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
                                        if(ImGui::SmallButton("preview"))
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
                                        //Frame->MissingResourceCount++;
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
                                    if(ImGui::SmallButton("play"))
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
                int CurrentModeID = GameState->GameModeID;
                ImGui::PushItemWidth(-1);
                ImGui::ListBox("##gamemode", &CurrentModeID, GameModes, ModeCount, RowCount);
                ImGui::PopItemWidth();
                GameState->GameModeID = CurrentModeID;
                
                //camera *Camera = 0;
                switch(GameState->GameModeID)
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
                        //ImGui::Checkbox("Visibility##Skybox", &Frame->DrawSkybox);
                        
                        ImGui::SeparatorText("Terrain");
                        /* 
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
                         */
                        
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
                switch(GameState->GameModeID)
                {
                    case GameMode_Test:
                    {
                        //ClearColor = (float*)&GameState->ModeTest.ClearColor;
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
}

internal debug_thread *
GetDebugThread(debug_state *DebugState, u32 ThreadID)
{
    debug_thread *Result = 0;
    for(debug_thread *Thread = DebugState->FirstThread;
        Thread;
        Thread = Thread->Next)
    {
        if(Thread->ID == ThreadID)
        {
            Result = Thread;
            break;
        }
    }
    
    if(!Result)
    {
        Result = PushStruct(&DebugState->CollateArena, debug_thread);
        Result->ID = ThreadID;
        Result->LaneIndex = DebugState->FrameBarLaneCount++;
        Result->FirstOpenBlock = 0;
        Result->Next = DebugState->FirstThread;
        DebugState->FirstThread = Result;
    }
    
    return(Result);
}

inline debug_record *
GetRecordFrom(open_debug_block *Block)
{
    debug_record *Result = Block ? Block->Source : 0;
    
    return(Result);
}

debug_frame_region *
AddRegion(debug_state *DebugState, debug_frame *CurrentFrame)
{
    Assert(CurrentFrame->RegionCount < MAX_REGIONS_PER_FRAME);
    debug_frame_region *Result = CurrentFrame->Regions + CurrentFrame->RegionCount++;
    
    return(Result);
}

internal void
CollateDebugRecords(debug_state *DebugState, u32 InvalidEventArrayIndex)
{
    for(;
        ;
        ++DebugState->CollationArrayIndex)
    {
        
        if(DebugState->CollationArrayIndex == MAX_DEBUG_EVENT_ARRAY_COUNT)
        {
            DebugState->CollationArrayIndex = 0;
        }
        
        u32 EventArrayIndex = DebugState->CollationArrayIndex;
        if(EventArrayIndex == InvalidEventArrayIndex)
        {
            break;
        }
        
        for(u32 EventIndex = 0;
            EventIndex < GlobalDebugTable->EventCount[EventArrayIndex];
            ++EventIndex)
        {
            debug_event *Event = GlobalDebugTable->Events[EventArrayIndex] + EventIndex;            
            debug_record *Source = (GlobalDebugTable->Records[Event->TranslationUnit] +
                                    Event->DebugRecordIndex);
            if(Event->Type == DebugEvent_FrameMarker)
            {
                if(DebugState->CollationFrame)
                {
                    DebugState->CollationFrame->EndClock = Event->Clock;
                    DebugState->CollationFrame->WallSecondsElapsed = Event->SecondsElapsed;
                    ++DebugState->FrameCount;
                    
                    r32 ClockRange = (r32)(DebugState->CollationFrame->EndClock - DebugState->CollationFrame->BeginClock);
                    
                }
                
                DebugState->CollationFrame = DebugState->Frames + DebugState->FrameCount;
                DebugState->CollationFrame->BeginClock = Event->Clock;
                DebugState->CollationFrame->EndClock = 0;
                DebugState->CollationFrame->RegionCount = 0;
                DebugState->CollationFrame->Regions = PushArray(&DebugState->CollateArena, MAX_REGIONS_PER_FRAME, debug_frame_region);
                DebugState->CollationFrame->WallSecondsElapsed = 0.0f;
            }
            else if(DebugState->CollationFrame)
            {
                u32 FrameIndex = DebugState->FrameCount - 1;
                debug_thread *Thread = GetDebugThread(DebugState, Event->TC.ThreadID);
                u64 RelativeClock = Event->Clock - DebugState->CollationFrame->BeginClock;
                
                if(Event->Type == DebugEvent_BeginBlock)
                {
                    open_debug_block *DebugBlock = DebugState->FirstFreeBlock;
                    if(DebugBlock)
                    {
                        DebugState->FirstFreeBlock = DebugBlock->NextFree;
                    }
                    else
                    {
                        DebugBlock = PushStruct(&DebugState->CollateArena, open_debug_block);
                    }
                    
                    DebugBlock->StartingFrameIndex = FrameIndex;
                    DebugBlock->OpeningEvent = Event;
                    DebugBlock->Parent = Thread->FirstOpenBlock;
                    DebugBlock->Source = Source;
                    Thread->FirstOpenBlock = DebugBlock;
                    DebugBlock->NextFree = 0;
                }
                else if(Event->Type == DebugEvent_EndBlock)
                {                    
                    if(Thread->FirstOpenBlock)
                    {
                        open_debug_block *MatchingBlock = Thread->FirstOpenBlock;
                        debug_event *OpeningEvent = MatchingBlock->OpeningEvent;
                        if((OpeningEvent->TC.ThreadID == Event->TC.ThreadID) &&
                           (OpeningEvent->DebugRecordIndex == Event->DebugRecordIndex) &&
                           (OpeningEvent->TranslationUnit == Event->TranslationUnit))
                        {
                            if(MatchingBlock->StartingFrameIndex == FrameIndex)
                            {
                                //if(GetRecordFrom(MatchingBlock->Parent) == DebugState->ScopeToRecord)
                                {
                                    r32 MinT = (r32)(OpeningEvent->Clock - DebugState->CollationFrame->BeginClock);
                                    r32 MaxT = (r32)(Event->Clock - DebugState->CollationFrame->BeginClock);
                                    r32 ThresholdT = 0.01f;
                                    if((MaxT - MinT) > ThresholdT)
                                    {
                                        debug_frame_region *Region = AddRegion(DebugState, DebugState->CollationFrame);
                                        Region->Record = Source;
                                        Region->CycleCount = (Event->Clock - OpeningEvent->Clock);
                                        Region->LaneIndex = (u16)Thread->LaneIndex;
                                        Region->MinT = MinT;
                                        Region->MaxT = MaxT;
                                        Region->ColorIndex = (u16)OpeningEvent->DebugRecordIndex;
                                    }
                                }
                            }
                            else
                            {
                                // TODO(casey): Record all frames in between and begin/end spans!
                            }
                            
                            Thread->FirstOpenBlock->NextFree = DebugState->FirstFreeBlock;
                            DebugState->FirstFreeBlock = Thread->FirstOpenBlock;
                            Thread->FirstOpenBlock = MatchingBlock->Parent;
                        }
                        else
                        {
                            // TODO(casey): Record span that goes to the beginning of the frame series?
                        }
                    }
                }
                else
                {
                    Assert(!"Invalid event type");
                }
            }
        }
    }
}

internal void
RestartCollation(debug_state *DebugState, u32 InvalidEventArrayIndex)
{
    EndTemporaryMemory(DebugState->CollateTemp);            
    DebugState->CollateTemp = BeginTemporaryMemory(&DebugState->CollateArena);
    
    DebugState->FirstThread = 0;
    DebugState->FirstFreeBlock = 0;
    
    DebugState->Frames = PushArray(&DebugState->CollateArena, MAX_DEBUG_EVENT_ARRAY_COUNT*4, debug_frame);
    DebugState->FrameBarLaneCount = 0;
    DebugState->FrameCount = 0;    
    DebugState->FrameBarScale = 1.0f / 60000000.0f;
    
    DebugState->CollationArrayIndex = InvalidEventArrayIndex + 1;
    DebugState->CollationFrame = 0;
}

internal void
DEBUGStart(debug_state *DebugState)
{
    TIMED_FUNCTION();
    
    if(!DebugState->Initialized)
    {
        DebugState->HighPriorityQueue = DebugGlobalMemory->HighPriorityQueue;
        DebugState->TreeSentinel.Next = &DebugState->TreeSentinel;
        DebugState->TreeSentinel.Prev = &DebugState->TreeSentinel;
        DebugState->TreeSentinel.Group = 0;
        
        InitializeArena(&DebugState->DebugArena, DebugGlobalMemory->DebugStorageSize - sizeof(debug_state), DebugState + 1);
        
        SubArena(&DebugState->CollateArena, &DebugState->DebugArena, Megabytes(32), 4);
        DebugState->CollateTemp = BeginTemporaryMemory(&DebugState->CollateArena);            
        
        RestartCollation(DebugState, 0);
        
        DebugState->Paused = false;
        DebugState->ScopeToRecord = 0;
        
        DebugState->Initialized = true;
    }
}

internal void
DEBUGEnd(debug_state *DebugState)
{
    imgui *ImGuiHandle = &DebugGlobalMemory->ImGuiHandle;
    if(ImGuiHandle->ShowImGuiWindows)
    {
        if(ImGuiHandle->ShowDebugCollationWindow)
        {
            ImGui::Begin("DebugCollation", &ImGuiHandle->ShowDebugCollationWindow);
            if(DebugState->FrameCount)
            {/* 
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            char TextBuffer[256];
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            _snprintf_s(TextBuffer, sizeof(TextBuffer),
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        "Last frame time: %.02fms",
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        DebugState->Frames[DebugState->FrameCount - 1].WallSecondsElapsed * 1000.0f);
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     */
                
                ImGui::Text("Last frame time: %.02fms", DebugState->Frames[DebugState->FrameCount - 1].WallSecondsElapsed * 1000.0f);
            }
            
            u32 MaxFrame = DebugState->FrameCount;
            if(MaxFrame > 10)
            {
                MaxFrame = 10;
            }
            
            temporary_memory TempMem = BeginTemporaryMemory(&DebugState->DebugArena);
            
            for(u32 FrameIndex = 0;
                FrameIndex < MaxFrame;
                ++FrameIndex)
            {
                debug_frame *Frame = DebugState->Frames + DebugState->FrameCount - (FrameIndex + 1);
                
                r64 *CycleCountArray = PushArray(&DebugState->DebugArena, Frame->RegionCount, r64);
                r64 CycleCountMax = 0;
                
                char **BlockNameArray = PushArray(&DebugState->DebugArena, Frame->RegionCount, char *);
                
                r32 *TmpArrayOld = PushArray(&DebugState->DebugArena, Frame->RegionCount, r32);
                r32 TmpMaxOld = 0;
                
                ImGui::BulletText("FrameIndex = %d", FrameIndex);
                //r32 StackX = ChartLeft;
                //r32 StackY = ChartTop - BarsPlusSpacing*(r32)FrameIndex;
                for(u32 RegionIndex = 0;
                    RegionIndex < Frame->RegionCount;
                    ++RegionIndex)
                {
                    debug_frame_region *Region = Frame->Regions + RegionIndex;
                    debug_record *Record = Region->Record;
                    /* 
                                        ImGui::BulletText("#%d - %s = %lu - %s - %d", RegionIndex, Record->BlockName, Region->CycleCount, Record->FileName, Record->LineNumber);
                     */
                    CycleCountArray[RegionIndex] = (r64)Region->CycleCount;
                    BlockNameArray[RegionIndex] = Record->BlockName;
                    if(Region->CycleCount > CycleCountMax)
                    {
                        CycleCountMax = (r64)Region->CycleCount;
                    }
                    
                    TmpArrayOld[RegionIndex] = (r32)Region->CycleCount;
                    if(Region->CycleCount > TmpMaxOld)
                    {
                        TmpMaxOld = (r32)Region->CycleCount;
                    }
                }
                
                r32 BarSize = 0.95f;
                r32 Shift = 0.5f;
                r32 BarStep = 1.0f;
                
                char TitleTextBuffer[256];
                _snprintf_s(TitleTextBuffer, sizeof(TitleTextBuffer),
                            "FrameIndex = %d##Histogram", FrameIndex);
                if(ImPlot::BeginPlot(TitleTextBuffer))
                {
                    ImPlot::SetupAxes("Regions", "Cycles", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
                    ImPlot::SetupAxesLimits(0, (u32)Frame->RegionCount, 0, (r64)CycleCountMax);
                    
                    ImPlot::SetupAxisFormat(ImAxis_X1, "%.0f");
                    ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
                    
                    ImPlot::PlotBars("TestPlotBars", CycleCountArray, Frame->RegionCount, BarSize, Shift);
                    
                    ImDrawList *DrawList = ImPlot::GetPlotDrawList();
                    if(ImPlot::IsPlotHovered())
                    {
                        ImPlotPoint MouseP = ImPlot::GetPlotMousePos();
                        if((MouseP.x > 0) && MouseP.x < Frame->RegionCount)
                        {
                            ImGui::BeginTooltip();
                            u32 RegionIndex = FloorR32ToS32((r32)MouseP.x);
                            r32 TooltipLeft = ImPlot::PlotToPixels(RegionIndex, MouseP.y).x;
                            r32 TooltipRight = ImPlot::PlotToPixels(RegionIndex + BarStep, MouseP.y).x;
                            r32 TooltipTop = ImPlot::GetPlotPos().y;
                            r32 TooltipBot = TooltipTop + ImPlot::GetPlotSize().y;
                            ImPlot::PushPlotClipRect();
                            DrawList->AddRectFilled(ImVec2(TooltipLeft, TooltipTop), ImVec2(TooltipRight, TooltipBot), IM_COL32(128,128,128,64));
                            ImPlot::PopPlotClipRect();
                            
                            debug_frame_region *Region = Frame->Regions + RegionIndex;
                            debug_record *Record = Region->Record;
                            ImGui::Text("RegionIndex = %d", RegionIndex);
                            ImGui::Text("BlockName = %s", Record->BlockName);
                            ImGui::Text("CycleCount = %lu", Region->CycleCount);
                            ImGui::Text("FileName = %s", Record->FileName);
                            ImGui::Text("LineNumber = %d", Record->LineNumber);
                            
                            ImGui::EndTooltip();
                        }
                    }
                    
                    ImPlot::EndPlot();
                }
                
                if(ImPlot::BeginPlot("##Pie1", ImVec2(550,550), ImPlotFlags_Equal))
                {
                    ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations);
                    ImPlot::SetupAxesLimits(0, 1, 0, 1);
                    ImPlot::PlotPieChart(BlockNameArray, CycleCountArray, Frame->RegionCount, 0.5, 0.5, 0.4, "%.0f", 90, ImPlotPieChartFlags_Normalize);
                    ImPlot::EndPlot();
                }
                
                char *TestStr = "1st ver histogram";
                ImGui::PlotHistogram("##Histogram", TmpArrayOld, Frame->RegionCount, 0, TestStr, 0.0f, TmpMaxOld, ImVec2(0, 200.0f));
            }
            
            EndTemporaryMemory(TempMem);
            
            
            ImGui::End();
        }
    }
    /* 
        for(u32 CounterIndex = 0;
            CounterIndex < DebugState->CounterCount;
            ++CounterIndex)
        {
            debug_counter_state *Counter = DebugState->CounterStates + CounterIndex;
        }
     */
    
    UpdateAndRenderImgui();
}

extern "C" DEBUG_GAME_FRAME_END(DEBUGGameFrameEnd)
{
    
    //GlobalDebugTable->RecordCount[0] = DebugRecords_Main_Count;
    //GlobalDebugTable->RecordCount[1] = DebugRecords_Optimized_Count;
    
    ++GlobalDebugTable->CurrentEventArrayIndex;
    if(GlobalDebugTable->CurrentEventArrayIndex >= ArrayCount(GlobalDebugTable->Events))
    {
        GlobalDebugTable->CurrentEventArrayIndex = 0;
    }
    
    u64 ArrayIndex_EventIndex = AtomicExchangeU64(&GlobalDebugTable->EventArrayIndex_EventIndex,
                                                  (u64)GlobalDebugTable->CurrentEventArrayIndex << 32);
    
    u32 EventArrayIndex = ArrayIndex_EventIndex >> 32;
    u32 EventCount = ArrayIndex_EventIndex & 0xFFFFFFFF;
    GlobalDebugTable->EventCount[EventArrayIndex] = EventCount;
    
    debug_state *DebugState = (debug_state *)Memory->DebugStorage;
    if(DebugState)
    {
        //game_assets *Assets = DEBUGGetGameAssets(Memory);
        
        DEBUGStart(DebugState);
        
        /* 
                if(Memory->ExecutableReloaded)
                {
                    RestartCollation(DebugState, GlobalDebugTable->CurrentEventArrayIndex);
                }
         */
        
        if(!DebugState->Paused)
        {
            
            if(DebugState->FrameCount >= MAX_DEBUG_EVENT_ARRAY_COUNT*4)
            {
                RestartCollation(DebugState, GlobalDebugTable->CurrentEventArrayIndex);
            }
            CollateDebugRecords(DebugState, GlobalDebugTable->CurrentEventArrayIndex);
        }        
        
        DEBUGEnd(DebugState);
    }
}