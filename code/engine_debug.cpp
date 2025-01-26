#include "engine_debug.h"

internal void
UpdateAndRenderImgui()
{
    game_state *GameState = (game_state *)GlobalDebugMemory->PermanentStorage;
    tran_state *TranState = (tran_state *)GlobalDebugMemory->TransientStorage;
    debug_state *DebugState = (debug_state *)GlobalDebugMemory->DebugStorage;
    
    renderer_frame *Frame = &GlobalDebugMemory->Frame;
    game_input *Input = GlobalDebugInput;
    imgui *ImGuiHandle = &GlobalDebugMemory->ImGuiHandle;
    
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
                //ImGui::Spacing();
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
                ImGui::SeparatorText("DebugPerFrameArena");
                ImGui::BulletText("Size = %d MB or %d KB or %d bytes",
                                  DebugState->PerFrameArena.Size / Megabytes(1),
                                  DebugState->PerFrameArena.Size / Kilobytes(1),
                                  DebugState->PerFrameArena.Size);
                ImGui::BulletText("Used = %d MB or %d KB or %d bytes",
                                  DebugState->PerFrameArena.Used / Megabytes(1),
                                  DebugState->PerFrameArena.Used / Kilobytes(1),
                                  DebugState->PerFrameArena.Used);
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
                
                renderer *Renderer = (renderer *)Frame->Renderer;
                if(ImGui::TreeNode("ShadowMap"))
                {
                    renderer_shadowmap *ShadowMap = Renderer->ShadowMap;
                    if(ShadowMap->Texture)
                    {
                        ImGui::Image((void *)(intptr_t)ShadowMap->Texture,
                                     ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                     ImVec2(0, 0), ImVec2(1, -1));
                        
                        ImGui::InputFloat("Size##ShadowMap", &ShadowMap->Size, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("NearPlane##ShadowMap", &ShadowMap->NearPlane, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("FarPlane##ShadowMap", &ShadowMap->FarPlane, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("CameraPitch##ShadowMap", &ShadowMap->CameraPitch, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("CameraYaw##ShadowMap", &ShadowMap->CameraYaw, 0.01f, 1.0f, "%.3f");
                        float *ShadowMapCameraP = (float *)&ShadowMap->CameraP;
                        ImGui::DragFloat3("CameraPos##ShadowMap", ShadowMapCameraP, 0.01f, -100.0f, 100.0f);
                        ImGui::InputFloat("Bias##ShadowMap", &ShadowMap->Bias, 0.001f, 1.0f, "%.3f");
                    }
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                if(ImGui::TreeNode("Water"))
                {
                    renderer_water *Water = Renderer->Water;
                    if(Water)
                    {                    
                        ImGui::Text("ReflectionColor");
                        ImGui::Image((void *)(intptr_t)Water->Reflection.ColorTexture,
                                     ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                     ImVec2(0, 0), ImVec2(1, -1));
                        
                        ImGui::Text("RefractionColor");
                        ImGui::Image((void *)(intptr_t)Water->Refraction.ColorTexture,
                                     ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                     ImVec2(0, 0), ImVec2(1, -1));
                        ImGui::Text("RefractionDepth");
                        ImGui::Image((void *)(intptr_t)Water->Refraction.DepthTexture,
                                     ImVec2((r32)Frame->Dim.x / AspectRatio, (r32)Frame->Dim.y / AspectRatio), 
                                     ImVec2(0, 0), ImVec2(1, -1));
                        
                        ImGui::InputFloat("WaveSpeed##Water", &Water->WaveSpeed, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("Tiling##Water", &Water->Tiling, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("WaveStrength##Water", &Water->WaveStrength, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("ShineDamper##Water", &Water->ShineDamper, 0.01f, 1.0f, "%.3f");
                        ImGui::InputFloat("Reflectivity##Water", &Water->Reflectivity, 0.01f, 1.0f, "%.3f");
                    }
                    
                    ImGui::TreePop();
                    ImGui::Spacing();
                }
                
                ImGui::SeparatorText("FragEffects");
                ImGui::BulletText("EffectID = %d", Frame->EffectID);
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
                int FragEffectCount = IM_ARRAYSIZE(FragEffects);
                Assert(FragEffectCount == FrameEffect_Count);
                if(FragEffectCount < RowCount)
                {
                    RowCount = FragEffectCount;
                }
                ImGui::PushItemWidth(-1);
                ImGui::ListBox("##frageffects", &Frame->EffectID, FragEffects, FragEffectCount, RowCount);
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
                if(ImGui::Button("Button1")){};
                if(ImGui::Button("Button2")){};
                if(ImGui::Button("Button3")){};
                
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
                        /* 
                                                float *TerrainMaterialAmbient = (float *)&Frame->TerrainMaterial.Ambient;
                                                ImGui::DragFloat4("Ambient##Terrain", TerrainMaterialAmbient, 0.01f, -100.0f, 100.0f);
                                                float *TerrainMaterialDiffuse = (float *)&Frame->TerrainMaterial.Diffuse;
                                                ImGui::DragFloat4("Diffuse##Terrain", TerrainMaterialDiffuse, 0.01f, -100.0f, 100.0f);
                                                float *TerrainMaterialSpecular = (float *)&Frame->TerrainMaterial.Specular;
                                                ImGui::DragFloat4("Specular##Terrain", TerrainMaterialSpecular, 0.01f, -100.0f, 100.0f);
                                                float *TerrainMaterialEmission = (float *)&Frame->TerrainMaterial.Emission;
                                                ImGui::DragFloat4("Emission##Terrain", TerrainMaterialEmission, 0.01f, -100.0f, 100.0f);
                                                ImGui::InputFloat("Shininess##Terrain", &Frame->TerrainMaterial.Shininess, 0.01f, 1.0f, "%.3f");
                                                 */
                        
                        ImGui::SeparatorText("DEBUGTextLine");
                        ImGui::Checkbox("Visibility##DEBUGTextLine", &Frame->DrawDebugTextLine);
                        
                        ImGui::SeparatorText("DirLight");
                        /* 
                                                float *DirLightBaseColor = (float *)&Frame->DirLight.Base.Color;
                                                ImGui::ColorEdit3("Color##DirLight", DirLightBaseColor);
                                                ImGui::InputFloat("AmbientIntensity##TerrainDirLight", &Frame->DirLight.Base.AmbientIntensity, 0.01f, 1.0f, "%.3f");
                                                ImGui::InputFloat("DiffuseIntensity##TerrainDirLight", &Frame->DirLight.Base.DiffuseIntensity, 0.01f, 1.0f, "%.3f");
                                                
                                                float *DirLightWorldDirection = (float *)&Frame->DirLight.WorldDirection;
                         
                        ImGui::DragFloat3("WorldDirection##TerrainDirLight", DirLightWorldDirection, 0.01f, -100.0f, 100.0f);
*/
                        
                        renderer *Renderer = (renderer *)Frame->Renderer;
                        if(Renderer->Lighting)
                        {
                            float *TestSunP = (float *)&Renderer->Lighting->TestSunP;
                            ImGui::DragFloat3("TestSun2P##TerrainDirLight", TestSunP, 0.01f, -100.0f, 100.0f);
                        }
                        
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
            /* 
                        ImGui::Text("ID = %d", Frame->Vert.OpenglID);
                        static ImGuiInputTextFlags VertFlags = ImGuiInputTextFlags_AllowTabInput;
                        // NOTE(ezexff): InputTextMultiline(label, text, bufferSize, [w=0, h=0, ImGuiInputTextFlags=0])
                        if(ImGui::InputTextMultiline("##vert", (char *)Frame->Vert.Text,
                                                     IM_ARRAYSIZE(Frame->Vert.Text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), VertFlags))
                        {
                            //Log->Add("Shader InputTextMultiline changed\n");
                        }
                        ImGui::SeparatorText("Fragment");
                        ImGui::Text("ID = %d", Frame->Frag.OpenglID);
                        static ImGuiInputTextFlags FragFlags = ImGuiInputTextFlags_AllowTabInput;
                        // NOTE(ezexff): InputTextMultiline(label, text, bufferSize, [w=0, h=0, ImGuiInputTextFlags=0])
                        if(ImGui::InputTextMultiline("##frag", (char *)Frame->Frag.Text,
                                                     IM_ARRAYSIZE(Frame->Frag.Text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), FragFlags))
                        {
                            //Log->Add("Shader InputTextMultiline changed\n");
                        }
                         */
            
            if(ImGui::Button("Recompile shaders and link program"))
            {
                Frame->CompileShaders = false;
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
            ImGui::Image((void *)(intptr_t)Frame->Preview.OpenglID,
                         ImVec2(WindowDim.x,WindowDim.y));
            
            ImGui::End();
        }
    }
}

internal void
DrawStoredBlockTree(debug_stored_block *InNode, u32 Depth, debug_state *DebugState, r64 FrameClock)
{
    for(debug_stored_block *Node = InNode;
        Node != 0;
        Node = Node->NextChild)
    {
        debug_parsed_name ParsedName = DebugParseName(Node->GUID);
        for(u32 Index = 0;
            Index < Depth;
            Index++)
        {
            ImGui::Text(" ");
            ImGui::SameLine();
        }
        ImGui::SameLine();
        
        debug_statistic *Stat = &DebugState->BlockStatArray[DebugState->TmpBlockCount] ;
        Stat->Min = Minimum(Stat->Min, (r64)Node->Clock);
        Stat->Max = Maximum(Stat->Max, (r64)Node->Clock);
        Stat->Sum += (r64)Node->Clock;
        Stat->Count++;
        Stat->Avg = Stat->Sum / Stat->Count;
        Assert(DebugState->TmpBlockCount < ArrayCount(DebugState->BlockStatArray));
        
        r64 FramePercent = 100 * Node->Clock / FrameClock;
        if(FramePercent > 5.0f)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
        }
        else if((FramePercent <= 5.0f) && (FramePercent > 0.5f))
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        }
        ImGui::Text("%s %.2f%% ThreadID=%d avg=%.2f min=%.2f max=%.2f clock=%d\n", ParsedName.Name, FramePercent, Node->ThreadID, (r64)Stat->Avg, (r64)Stat->Min, (r64)Stat->Max, Node->Clock);
        ImGui::PopStyleColor();
        
        DebugState->TmpBlockCount++;
        
        if(Node->FirstChild != 0)
        {
            DrawStoredBlockTree(Node->FirstChild, Depth + 1, DebugState, FrameClock);
        }
    }
}

internal void
DEBUGInitFrame(debug_state *DebugState)
{
    DebugState->StoredBlockCount = 1;
    DebugState->OpenBlockIndex = 0;
    DebugState->DebugFrameIndex = 0;
    
    u32 FrameCount = ArrayCount(DebugState->DebugFrameArray);
    for(u32 Index = 0;
        Index < FrameCount;
        ++Index)
    {
        DebugState->DebugFrameArray[Index].ClockInCycles = 0;
        DebugState->DebugFrameArray[Index].RootStoredBlock = {};
        char TextBuffer[256];
        _snprintf_s(TextBuffer, sizeof(TextBuffer), "0|0|0|Root#%d", Index);
        DebugState->DebugFrameArray[Index].RootStoredBlock.GUID = PushString(&DebugState->DebugArena, TextBuffer);
    }
}

internal void
CollateDebugRecords(debug_state *DebugState, u32 EventCount, debug_event *EventArray)
{
    TIMED_FUNCTION();
    
    for(u32 EventIndex = 0;
        EventIndex < EventCount;
        ++EventIndex)
    {
        debug_event *Event = EventArray + EventIndex;
        if(Event->Type == DebugType_FrameMarker)
        {
            DebugState->DebugFrameArray[DebugState->DebugFrameIndex].ClockInMs = Event->Value_r32;
            DebugState->DebugFrameIndex++;
            if(DebugState->DebugFrameIndex >= DEBUG_FRAME_COUNT)
            {
                EndTemporaryMemory(DebugState->FramesTempMemory);
                DebugState->FramesTempMemory = BeginTemporaryMemory(&DebugState->DebugArena);
                DEBUGInitFrame(DebugState);
            }
            DebugState->TotalFrameCount++;
        }
        else 
        {
            switch(Event->Type)
            {
                case DebugType_BeginBlock:
                {
                    DebugState->StoredBlockCount++;
                    debug_stored_block *StoredBlock = PushStruct(&DebugState->DebugArena, debug_stored_block, NoClear());
                    StoredBlock->Clock = Event->Clock;
                    StoredBlock->GUID = Event->GUID;
                    StoredBlock->ThreadID = Event->ThreadID;
                    StoredBlock->CoreIndex = Event->CoreIndex;
                    StoredBlock->Type = Event->Type;
                    StoredBlock->FrameIndex = DebugState->TotalFrameCount;
                    
                    StoredBlock->FirstChild = 0;
                    StoredBlock->NextChild = 0;
                    StoredBlock->LastChild = 0;
                    
                    // NOTE(ezexff): Add child to prev opened block
                    DebugState->OpenBlockArray[0] = &DebugState->DebugFrameArray[DebugState->DebugFrameIndex].RootStoredBlock;
                    if(!DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->FirstChild)
                    {
                        DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->FirstChild = StoredBlock;
                        DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->LastChild = StoredBlock;
                        //DebugState->DebugFrameArray[Index].FirstStoredBlock;
                    }
                    else
                    {
                        DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->LastChild->NextChild = StoredBlock;
                        DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->LastChild = StoredBlock;
                        /* 
                                                if(!DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->Child->Next)
                                                {
                                                    DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->Child->Next = StoredBlock;
                                                }
                                                else
                                                {
                                                    debug_stored_block *LastChild = 0;
                                                    for(debug_stored_block *Node = DebugState->OpenBlockArray[DebugState->OpenBlockIndex]->Child;
                                                        Node != 0;
                                                        Node = Node->Next)
                                                    {
                                                        LastChild = Node;
                                                    }
                                                    LastChild->Next = StoredBlock;
                                                }
                         */
                    }
                    
                    // NOTE(ezexff): Save last opened block
                    DebugState->OpenBlockIndex++;
                    Assert(DebugState->OpenBlockIndex < 10);
                    DebugState->OpenBlockArray[DebugState->OpenBlockIndex] = StoredBlock;
                    
                    /* 
                                        if(DebugState->CurrentStoredBlockInHierarchy->Child == 0)
                                        {
                                            DebugState->CurrentStoredBlockInHierarchy->Child = StoredBlock;
                                        }
                                        else
                                        {
                                            DebugState->CurrentStoredBlockInHierarchy->Next = StoredBlock;
                                        }
                                        
                                        StoredBlock->Parent = DebugState->CurrentStoredBlockInHierarchy;
                                        DebugState->CurrentStoredBlockInHierarchy = StoredBlock;
                                        fdsfdsf;
                                         */
                    
                    /* 
                                                            Assert(DebugState->OpenBlockIndex < 10);
                                                            if(DebugState->OpenBlockIndex > 0)
                                                            {
                                                                if(DebugState->OpenBlockArray[DebugState->OpenBlockIndex - 1])
                                                                {
                                                                    StoredBlock->Parent = DebugState->OpenBlockArray[DebugState->OpenBlockIndex - 1];
                                                                }
                                                            }
                     */
                    
                    /* 
                                        DebugState->OpenBlockArray[DebugState->OpenBlockIndex] = StoredBlock;
                                        ++DebugState->OpenBlockIndex;
                     */
                    /* 
                                        u32 Index = DebugState->DebugFrameIndex;
                                        if(!DebugState->DebugFrameArray[Index].FirstStoredBlock)
                                        {
                                            DebugState->DebugFrameArray[Index].FirstStoredBlock = StoredBlock;
                                        }
                                        else
                                        {
                                            DebugState->DebugFrameArray[Index].LastStoredBlock->Next = StoredBlock;
                                        }
                                        
                                        DebugState->DebugFrameArray[Index].LastStoredBlock = StoredBlock;
                     */
                    
                } break;
                
                case DebugType_EndBlock:
                {
                    /* 
                                        --DebugState->OpenBlockIndex;
                                        debug_stored_block *StoredBlock = DebugState->OpenBlockArray[DebugState->OpenBlockIndex];
                     */
                    
                    debug_stored_block *OpenStoredBlock = DebugState->OpenBlockArray[DebugState->OpenBlockIndex];
                    if(OpenStoredBlock->ThreadID == Event->ThreadID)
                    {
                        OpenStoredBlock->Clock = Event->Clock - OpenStoredBlock->Clock;
                        DebugState->DebugFrameArray[DebugState->DebugFrameIndex].ClockInCycles += (r64)OpenStoredBlock->Clock;
                    }
                    else
                    {
                        InvalidCodePath;
                    }
                    
                    DebugState->OpenBlockArray[DebugState->OpenBlockIndex] = 0;
                    DebugState->OpenBlockIndex--;
                    
                    
                    /* 
                                        DebugState->OpenBlockArray[DebugState->OpenBlockIndex] = 0;
                     */
                    
                    
                } break;
                
                InvalidDefaultCase;
            }
        }
    }
    
#if 0    
    if(ImGuiHandle->ShowImGuiWindows)
    {
        ImGui::Begin("Test", &ImGuiHandle->ShowDebugCollationWindow);
        
        local bool DrawRollingPlot = false;
        ImGui::Checkbox("DrawRollingPlot", &DrawRollingPlot);
        local bool ShowPieChart = false;
        ImGui::Checkbox("ShowPieChart", &ShowPieChart);
        
        u32 RegionCount = EventCount / 2;
        r64 *CycleCountArray = PushArray(&DebugState->DebugArena, RegionCount, r64);
        char **BlockNameArray = PushArray(&DebugState->DebugArena, RegionCount, char *);
        
        
        //local RollingBuffer RData[32]; // TODO(ezexff): Replace this with debug storage
        local r32 t = 0;
        t += ImGui::GetIO().DeltaTime;
        
        local r32 History = 5.0f;
        ImGui::SliderFloat("History", &History, 1, 60, "%.1f s");
        
        u32 RegionIndex = 0;
        
        if(ImPlot::BeginPlot("##Rolling", ImVec2(-1, 500)))
        {        
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, History, ImGuiCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, (r64)(30000000.0f));
            ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
            
            for(u32 EventIndex = 0;
                EventIndex < EventCount;
                ++EventIndex)
            {
                debug_event *Event = EventArray + EventIndex;
                
                if(Event->Type == DebugType_FrameMarker)
                {
                    Log->Add("\n\n\n");
                    Log->Add("%s = %d\n", Event->BlockName, DebugState->FrameCount);
                    Log->Add("\n\n\n");
                    DebugState->CurrentDebugBlock = 0;
                    
                    ++DebugState->FrameCount;
                }
                else if(Event->Type == DebugType_BeginBlock)
                {
                    if(!DebugState->CurrentDebugBlock)
                    {
                        DebugState->CurrentDebugBlock = PushStruct(&DebugState->DebugArena, debug_block);
                        DebugState->CurrentDebugBlock->OpeningEvent = Event;
                    }
                    else
                    {
                        DebugState->CurrentDebugBlock->Next = PushStruct(&DebugState->DebugArena, debug_block);
                        DebugState->CurrentDebugBlock->Next->Prev = DebugState->CurrentDebugBlock;
                        DebugState->CurrentDebugBlock->Next->OpeningEvent = Event;
                        DebugState->CurrentDebugBlock = DebugState->CurrentDebugBlock->Next;
                    }
                    
                    //Log->Add("Begin = %s\n", Event->BlockName);
                    //open_debug_block *DebugBlock = AllocateOpenDebugBlock(DebugState, Element, FrameIndex, Event, &Thread->FirstOpenCodeBlock);
                }
                else if(Event->Type == DebugType_EndBlock)
                {
                    u64 CycleCount = (Event->Clock - DebugState->CurrentDebugBlock->OpeningEvent->Clock);
                    //Log->Add("%s = %lu\n", DebugState->CurrentDebugBlock->OpeningEvent->BlockName, CycleCount);
                    
                    if(DrawRollingPlot)
                    {                    
                        ImGuiHandle->RData[RegionIndex].AddPoint(t, (r32)CycleCount);
                        ImGuiHandle->RData[RegionIndex].Span = History;
                        char BlockNameTextBuffer[256];
                        _snprintf_s(BlockNameTextBuffer, sizeof(BlockNameTextBuffer), "%s", DebugState->CurrentDebugBlock->OpeningEvent->BlockName);
                        ImPlot::PlotLine(BlockNameTextBuffer, 
                                         &ImGuiHandle->RData[RegionIndex].Data[0].x, &ImGuiHandle->RData[RegionIndex].Data[0].y, 
                                         ImGuiHandle->RData[RegionIndex].Data.size(), 0, 0, 2 * sizeof(float));
                    }
                    
                    
                    
                    char *BlockName = DebugState->CurrentDebugBlock->OpeningEvent->BlockName;
                    BlockNameArray[RegionIndex] = BlockName;
                    CycleCountArray[RegionIndex] = (r64)CycleCount;
                    
                    Log->Add("ImGuiHandle->RData[RegionIndex].Data.size() = %d\n", ImGuiHandle->RData[RegionIndex].Data.size());
                    
                    DebugState->CurrentDebugBlock = DebugState->CurrentDebugBlock->Prev;
                    RegionIndex++;
                }
            }
            Assert(RegionIndex == RegionCount);
            ImPlot::EndPlot();
        }
        
        
        if(ShowPieChart)
        {        
            if(ImPlot::BeginPlot("##Pie1", ImVec2(550,550), ImPlotFlags_Equal))
            {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations);
                ImPlot::SetupAxesLimits(0, 1, 0, 1);
                ImPlot::PlotPieChart(BlockNameArray, CycleCountArray, RegionCount, 0.5, 0.5, 0.4, "%.0f", 90, ImPlotPieChartFlags_Normalize);
                ImPlot::EndPlot();
            }
        }
        ImGui::End();
    }
#endif
}

internal void
DEBUGStart(debug_state *DebugState, u32 MainGenerationID)
{
    TIMED_FUNCTION();
    
    if(!DebugState->Initialized)
    {
        memory_index TotalMemorySize = GlobalDebugMemory->DebugStorageSize - sizeof(debug_state);
        InitializeArena(&DebugState->DebugArena, TotalMemorySize, DebugState + 1);
#if 1
        SubArena(&DebugState->PerFrameArena, &DebugState->DebugArena, (TotalMemorySize / 2));
#else
        // NOTE(casey): This is the stress-testing case to make sure the memory
        // recycling works.
        SubArena(&DebugState->PerFrameArena, &DebugState->DebugArena, 8*1024*1024);
#endif
        //DebugState->EmptyDebugArena = DebugState->DebugArena;
        DebugState->TotalFrameCount = 0;
        
        DebugState->FramesTempMemory = BeginTemporaryMemory(&DebugState->DebugArena);
        
        // NOTE(ezexff): Init block stat
        for(u32 Index = 0;
            Index < ArrayCount(DebugState->BlockStatArray);
            ++Index)
        {
            DebugState->BlockStatArray[Index].Min = F64Max;
            DebugState->BlockStatArray[Index].Max = F64Min;
            DebugState->BlockStatArray[Index].Sum  = 0.0f;
            DebugState->BlockStatArray[Index].Avg = 0.0f;
            DebugState->BlockStatArray[Index].Count = 0;
        }
        
        DEBUGInitFrame(DebugState);
        
        DebugState->Initialized = true;
    }
    
    if(!DebugState->Paused)
    {
        DebugState->ViewingFrameOrdinal = DebugState->MostRecentFrameOrdinal;
    }
}

internal void
DEBUGEnd(debug_state *DebugState)
{
    TIMED_FUNCTION();
    
    imgui *ImGuiHandle = &GlobalDebugMemory->ImGuiHandle;
    if(ImGuiHandle->ShowImGuiWindows)
    {
        if(ImGuiHandle->ShowDebugCollationWindow)
        {
            ImGui::Begin("DebugProfile", &ImGuiHandle->ShowDebugCollationWindow);
            ImGui::Text("TotalFrameCount = %d\n", DebugState->TotalFrameCount);
            ImGui::Text("StoredBlockCount = %d\n", DebugState->StoredBlockCount);
            ImGui::Text("DebugFrameIndex = %d\n", DebugState->DebugFrameIndex);
            ImGui::Text("OpenBlockIndex = %d\n", DebugState->OpenBlockIndex);
            
            if(ImGui::Button("ResetStat"))
            {
                for(u32 Index = 0;
                    Index < ArrayCount(DebugState->BlockStatArray);
                    ++Index)
                {
                    DebugState->BlockStatArray[Index].Min = F64Max;
                    DebugState->BlockStatArray[Index].Max = F64Min;
                    DebugState->BlockStatArray[Index].Sum = 0.0f;
                    DebugState->BlockStatArray[Index].Avg = 0.0f;
                    DebugState->BlockStatArray[Index].Count = 0;
                }
            }
            
            if(DebugState->TotalFrameCount > 0)
            {
                s32 PrevFrameIndex = DebugState->DebugFrameIndex - 1;
                
                if(DebugState->DebugFrameIndex == 0)
                {
                    PrevFrameIndex = 255;
                }
                /* 
                            if(PrevFrameIndex <= 0)
                            {
                                int TestFoo = 0;
                            }
                 */
                
                //if(DebugState->DebugFrameIndex > 0)
                //CalcStoredBlockTreeStat(&DebugState->DebugFrameArray[PrevFrameIndex].RootStoredBlock, 0, DebugState->BlockStatArray);
                ImGui::Text("PrevFrameClock = %.3fms", DebugState->DebugFrameArray[PrevFrameIndex].ClockInMs * 1000.0f);
                ImGui::Text("PrevFrameClock = %.0fcycles", DebugState->DebugFrameArray[PrevFrameIndex].ClockInCycles);
                ImGui::Spacing();
                DebugState->TmpBlockCount = 0;
                DrawStoredBlockTree(&DebugState->DebugFrameArray[PrevFrameIndex].RootStoredBlock, 0, DebugState, DebugState->DebugFrameArray[PrevFrameIndex].ClockInCycles);
            }
            
            ImGui::End();
        }
        
#if 0
        ImGui::Begin("DebugProfile", &ImGuiHandle->ShowDebugCollationWindow);
        ImGui::Text("TotalFrameCount = %d\n", DebugState->TotalFrameCount);
        ImGui::Text("StoredBlockCount = %d\n", DebugState->StoredBlockCount);
        ImGui::Text("DebugFrameIndex = %d\n", DebugState->DebugFrameIndex);
        
        if(DebugState->TotalFrameCount > 0)
        {
            s32 PrevFrameIndex = DebugState->DebugFrameIndex - 1;
            /* 
                        if(DebugState->DebugFrameIndex == 0)
                        {
                            PrevFrameIndex = 255;
                        }
             */
            if(DebugState->DebugFrameIndex > 0)
            {
                PrintStoredBlockTree(&DebugState->DebugFrameArray[PrevFrameIndex].RootStoredBlock, 0);
            }
            
            debug_stored_block *NewFirstStoredBlock = DebugState->DebugFrameArray[PrevFrameIndex].FirstStoredBlock;
            u32 RootBlockCount = 0;
            r64 RootClockSum = 0.0f;
            for(debug_stored_block *Node = DebugState->DebugFrameArray[PrevFrameIndex].FirstStoredBlock;
                Node != 0;
                Node = Node->Next)
            {
                if(Node->Parent == DebugState->StoredBlockRoot)
                {
                    debug_statistic *Stat = &DebugState->BlockStatArray[RootBlockCount];
                    Stat->Min = Minimum(Stat->Min, (r64)Node->Clock);
                    Stat->Max = Maximum(Stat->Max, (r64)Node->Clock);
                    Stat->Sum += (r64)Node->Clock;
                    Stat->Count++;
                    Stat->Avg = Stat->Sum / Stat->Count;
                    
                    RootClockSum += (r64)Node->Clock;
                    RootBlockCount++;
                    Assert(ArrayCount(DebugState->BlockStatArray) > RootBlockCount);
                    
                    debug_parsed_name ParsedName = DebugParseName(Node->GUID);
                    ImGui::Text("%s", ParsedName.Name);
                    /* 
if(ImGui::TreeNode((void *)(intptr_t)Node, "%s", ParsedName.Name))
{
for(debug_stored_block *InnerNode = Node->Next;
InnerNode != 0;
InnerNode = InnerNode->Next)
{
debug_parsed_name InnerParsedName = DebugParseName(InnerNode->GUID);

}
ImGui::TreePop();
}
*/
                    /* 
                                        if((Node->Next != 0) && (Node->Next != DebugState->StoredBlockRoot))
                                        {
                                            //while(Node->Next != DebugState->StoredBlockRoot)
                                            {
                                                debug_parsed_name InnerParsedName = DebugParseName(Node->GUID);
                                                ImGui::Text("  %s", InnerParsedName.Name);
                                                Node = Node->Next;
                                            }
                                        }
                     */
                    /* 
                                        for(debug_stored_block *InnerNode = Node->Next;
                                            InnerNode != 0;
                                            InnerNode = InnerNode->Next)
                                        {
                                            if(Node->Parent == InnerNode)
                                            {
                                                debug_parsed_name InnerParsedName = DebugParseName(Node->GUID);
                                                ImGui::Text("  %s", InnerParsedName.Name);
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }
                     */
                }
                else
                {
                    debug_parsed_name ParsedName = DebugParseName(Node->GUID);
                    ImGui::Text("  %s", ParsedName.Name);
                }
            }
            
            ImGui::Text("RootBlockCount = %d\n", RootBlockCount);
            
            temporary_memory TempMem = BeginTemporaryMemory(&DebugState->PerFrameArena);
            r64 *CycleCountArray = PushArray(&DebugState->PerFrameArena, RootBlockCount, r64);
            char **BlockNameArray = PushArray(&DebugState->PerFrameArena, RootBlockCount, char *);
            
            ImGui::SeparatorText("UnsortedBlocks");
            ImGui::Text("PrevFrameClock = %.3fms", DebugState->DebugFrameArray[PrevFrameIndex].Clock * 1000.0f);
            if(ImGui::BeginTable("DebugStatTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Min");
                ImGui::TableSetupColumn("Max");
                ImGui::TableSetupColumn("Avg");
                ImGui::TableSetupColumn("Cycles");
                ImGui::TableSetupColumn("Percent");
                ImGui::TableHeadersRow();
                
                u32 Index = 0;
                for(debug_stored_block *Node = DebugState->DebugFrameArray[PrevFrameIndex].FirstStoredBlock;
                    Node != 0;
                    Node = Node->Next)
                {
                    if(Node->Parent == DebugState->StoredBlockRoot)
                    {
                        debug_parsed_name ParsedName = DebugParseName(Node->GUID);
                        debug_statistic *Stat = &DebugState->BlockStatArray[Index];
                        
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::Text("%s", ParsedName.Name);
                        ImGui::TableNextColumn();
                        ImGui::Text("%.0lf", Stat->Min);
                        ImGui::TableNextColumn();
                        ImGui::Text("%.0f", Stat->Max);
                        ImGui::TableNextColumn();
                        ImGui::Text("%.2f", Stat->Avg);
                        ImGui::TableNextColumn();
                        ImGui::Text("%.0f", (r64)Node->Clock);
                        ImGui::TableNextColumn();
                        ImGui::Text("%.2f", (r64)Node->Clock / RootClockSum * 100.0f);
                        
                        CycleCountArray[Index] = (r64)Node->Clock;
                        BlockNameArray[Index] = ParsedName.Name;
                        
                        Index++;
                    }
                }
                Index++;
                
                ImGui::EndTable();
            }
            
            if(ImPlot::BeginPlot("##Pie1", ImVec2(550,550), ImPlotFlags_Equal))
            {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations, ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_NoDecorations);
                ImPlot::SetupAxesLimits(0, 1, 0, 1);
                ImPlot::PlotPieChart(BlockNameArray, CycleCountArray, RootBlockCount, 0.5, 0.5, 0.4, "%.0f", 90, ImPlotPieChartFlags_Normalize);
                ImPlot::EndPlot();
            }
            
            u32 Count = RootBlockCount;
            for(u32 Outer = 0;
                Outer < Count;
                ++Outer)
            {
                b32 ListIsSorted = true;
                for(u32 Inner = 0;
                    Inner < Count - 1;
                    ++Inner)
                {
                    r64 A = CycleCountArray[Inner];
                    r64 B = CycleCountArray[Inner + 1];
                    if(A > B)
                    {
                        CycleCountArray[Inner] = B;
                        CycleCountArray[Inner + 1] = A;
                        
                        char *A2 = BlockNameArray[Inner];
                        char *B2 = BlockNameArray[Inner + 1];
                        BlockNameArray[Inner] = B2;
                        BlockNameArray[Inner + 1] = A2;
                        
                        ListIsSorted = false;
                    }
                }
                
                if(ListIsSorted)
                {
                    break;
                }
            }
            
            ImGui::SeparatorText("SortedBlocks");
            for(s32 Index = Count - 1;
                Index >= 0;
                Index--)
            {
                ImGui::Text("%s = %f\n", BlockNameArray[Index], CycleCountArray[Index]);
            }
            
            EndTemporaryMemory(TempMem);
            /*             
local r32 History = 15.0f;
ImGui::SliderFloat("History", &History, 1, 60, "%.1f s");

local r32 PlotZoom = 50.0f;
ImGui::SliderFloat("PlotZoom", &PlotZoom, 1, 100, "%.1f s");

if(ImPlot::BeginPlot("##Rolling", ImVec2(-1, 700)))
{
ImPlot::SetupAxisLimits(ImAxis_X1, 0, History, ImGuiCond_Always);
ImPlot::SetupAxisLimits(ImAxis_Y1, 0, (r64)(30000000.0f));
ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");

for(u32 RegionIndex = 0;
RegionIndex < Frame->RegionCount;
++RegionIndex)
{
debug_frame_region *Region = Frame->Regions + RegionIndex;
debug_record *Record = Region->Record;
char BlockNameTextBuffer[256];
_snprintf_s(BlockNameTextBuffer, sizeof(BlockNameTextBuffer), "%s", Record->BlockName);
ImPlot::PlotLine(BlockNameTextBuffer, 
              &RData[RegionIndex].Data[0].x, &RData[RegionIndex].Data[0].y, 
              RData[RegionIndex].Data.size(), 0, 0, 2 * sizeof(float));
}
ImPlot::EndPlot();
}
*/
        }
        
        ImGui::End();
        {
            ImGui::Begin("DebugCollation", &ImGuiHandle->ShowDebugCollationWindow);
            
            u32 MaxFrame = DebugState->FrameCount;
            Log->Add("MaxFrame = %d\n", MaxFrame);
            if(MaxFrame > 10)
            {
                MaxFrame = 10;
            }
            
            temporary_memory TempMem = BeginTemporaryMemory(&DebugState->DebugArena);
            
            // NOTE(ezexff): Realtime plot
            //local int FrameIndex = 0;
            //ImGui::SliderInt("FrameIndex", &FrameIndex, 1, 60);
            
            u32 FrameIndex = 0;
            debug_frame *Frame = DebugState->Frames + DebugState->FrameCount - (FrameIndex + 1);
            u64 FrameCycleCount = Frame->EndClock - Frame->BeginClock;
            ImGui::Text("FrameCycleCount = %lu", FrameCycleCount);
            
            //Assert(Frame->Regions);
            
            if(Frame->Regions)
            {
                local r32 History = 15.0f;
                ImGui::SliderFloat("History", &History, 1, 60, "%.1f s");
                
                local r32 PlotZoom = 50.0f;
                ImGui::SliderFloat("PlotZoom", &PlotZoom, 1, 100, "%.1f s");
                
                Assert(Frame->RegionCount < 32);
                local RollingBuffer RData[32]; // TODO(ezexff): Replace this with debug storage
                local r32 t = 0;
                t += ImGui::GetIO().DeltaTime;
                
                for(u32 RegionIndex = 0;
                    RegionIndex < Frame->RegionCount;
                    ++RegionIndex)
                {
                    debug_frame_region *Region = Frame->Regions + RegionIndex;
                    debug_record *Record = Region->Record;
                    RData[RegionIndex].AddPoint(t, (r32)Region->CycleCount);
                    RData[RegionIndex].Span = History;
                }
                
                if(ImPlot::BeginPlot("##Rolling", ImVec2(-1, 700)))
                {
                    //ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoTickLabels, ImPlotAxisFlags_NoTickLabels);
                    ImPlot::SetupAxisLimits(ImAxis_X1, 0, History, ImGuiCond_Always);
                    //ImPlot::SetupAxisLimits(ImAxis_Y1, 0, (r64)(FrameCycleCount / PlotZoom), ImGuiCond_Always);
                    //r64 TestFrameCycleCount = 3.6f * 1000 * 1000 * 1000 * DebugGlobalInput->dtForFrame;
                    //ImPlot::SetupAxisLimits(ImAxis_Y1, 0, (r64)(TestFrameCycleCount), ImGuiCond_Always);
                    ImPlot::SetupAxisLimits(ImAxis_Y1, 0, (r64)(30000000.0f));
                    ImPlot::SetupAxisFormat(ImAxis_Y1, "%.0f");
                    
                    for(u32 RegionIndex = 0;
                        RegionIndex < Frame->RegionCount;
                        ++RegionIndex)
                    {
                        debug_frame_region *Region = Frame->Regions + RegionIndex;
                        debug_record *Record = Region->Record;
                        char BlockNameTextBuffer[256];
                        _snprintf_s(BlockNameTextBuffer, sizeof(BlockNameTextBuffer), "%s", Record->BlockName);
                        ImPlot::PlotLine(BlockNameTextBuffer, 
                                         &RData[RegionIndex].Data[0].x, &RData[RegionIndex].Data[0].y, 
                                         RData[RegionIndex].Data.size(), 0, 0, 2 * sizeof(float));
                    }
                    ImPlot::EndPlot();
                }
            }
            
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
                            DrawList->AddRectFilled(ImVec2(TooltipLeft, TooltipTop), ImVec2(TooltipRight, TooltipBot), IM_COL32(128, 128, 128, 64));
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
            
            //EndTemporaryMemory(TempMem);
            
            ImGui::End();
        }
#endif
    }
    
    UpdateAndRenderImgui();
}

// TODO(casey): Really want to get rid of main generation ID
internal u32
DEBUGGetMainGenerationID(game_memory *Memory)
{
    u32 Result = 0;
    
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    if(TranState->IsInitialized)
    {
        Result = TranState->MainGenerationID;
    }
    
    return(Result);
}

extern "C" DEBUG_GAME_FRAME_END(DEBUGGameFrameEnd)
{
    GlobalDebugTable = Memory->DebugTable;
    
    GlobalDebugTable->CurrentEventArrayIndex = !GlobalDebugTable->CurrentEventArrayIndex;
    u64 ArrayIndex_EventIndex = AtomicExchangeU64(&GlobalDebugTable->EventArrayIndex_EventIndex,
                                                  (u64)GlobalDebugTable->CurrentEventArrayIndex << 32);
    
    u32 EventArrayIndex = ArrayIndex_EventIndex >> 32;
    Assert(EventArrayIndex <= 1);
    u32 EventCount = ArrayIndex_EventIndex & 0xFFFFFFFF;
    
    debug_state *DebugState = (debug_state *)Memory->DebugStorage;
    if(DebugState)
    {
        DebugState->Paused = Memory->Paused;
        DEBUGStart(DebugState, DEBUGGetMainGenerationID(Memory));
        CollateDebugRecords(DebugState, EventCount, GlobalDebugTable->Events[EventArrayIndex]);
        DEBUGEnd(DebugState);
    }
}