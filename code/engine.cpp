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
                v3 Pos = V3(0, 0, 0);
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
    
    Log->Add("[test_work] TestWorkWithMemory\n");
    
    EndTaskWithMemory(Work->Task);
}

void
OpenglCompileShader(opengl *Opengl, GLuint Type, opengl_shader *Shader)
{
    if(Shader->ID != 0)
    {
        Opengl->glDeleteShader(Shader->ID);
    }
    
    Shader->ID = Opengl->glCreateShader(Type);
    u8 *ShaderText = Shader->Text;
    Opengl->glShaderSource(Shader->ID, 1, &(GLchar *)ShaderText, NULL);
    Opengl->glCompileShader(Shader->ID);
    
    char *TypeStr = (Type == GL_VERTEX_SHADER) ? "vert" : "frag";
    
    GLint NoErrors;
    GLchar LogInfo[2000];
    Opengl->glGetShaderiv(Shader->ID, GL_COMPILE_STATUS, &NoErrors);
    if(!NoErrors)
    {
        Opengl->glGetShaderInfoLog(Shader->ID, 2000, NULL, LogInfo);
        Opengl->glDeleteShader(Shader->ID);
        Shader->ID = 0;
        Log->Add("[opengl]: %s shader compilaton error (info below):\n%s", TypeStr, LogInfo);
    }
    else
    {
        //Log->Add("[opengl]: %s shader successfully compiled\n", TypeStr);
    }
}

void
OpenglLinkProgram(opengl *Opengl, opengl_program *Program,
                  opengl_shader *VertShader, opengl_shader *FragShader)
{
    if(Program->ID != 0)
    {
        Opengl->glDeleteProgram(Program->ID);
    }
    
    Program->ID = Opengl->glCreateProgram();
    
    if(VertShader->ID != 0)
    {
        Opengl->glAttachShader(Program->ID, VertShader->ID);
    }
    else
    {
        Log->Add("[program error]: vert shader null\n");
    }
    
    if(FragShader->ID != 0)
    {
        Opengl->glAttachShader(Program->ID, FragShader->ID);
    }
    else
    {
        Log->Add("[program error]: frag shader null\n");
    }
    
    Opengl->glLinkProgram(Program->ID);
    
    GLint NoErrors;
    GLchar LogInfo[2000];
    Opengl->glGetProgramiv(Program->ID, GL_LINK_STATUS, &NoErrors);
    if(!NoErrors)
    {
        Opengl->glGetProgramInfoLog(Program->ID, 2000, NULL, LogInfo);
        Log->Add("[program error]: program linking error (info below):\n%s", LogInfo);
    }
    else
    {
        Log->Add("[program]: program successfully linked\n");
    }
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
        
        // NOTE(ezexff): Init game mode
        {
            GameState->GameMode = GameMode_World;
        }
        
#if ENGINE_INTERNAL
        // NOTE(ezexff): Init sound mixer
        {
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
            
            Log->Add("[program]: frame\n");
            OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->Vert);
            OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->Frag);
            OpenglLinkProgram(Opengl, &Frame->Program, &Frame->Vert, &Frame->Frag);
            Log->Add("\n");
            
            Log->Add("[program]: skybox\n");
            OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->SkyboxVert);
            OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->SkyboxFrag);
            OpenglLinkProgram(Opengl, &Frame->SkyboxProgram, &Frame->SkyboxVert, &Frame->SkyboxFrag);
            Log->Add("\n");
        }
        
        // NOTE(ezexff): Set default frame frag effect
        Frame->FragEffect = EFFECT_NORMAL;
        
        GameState->IsInitialized = true;
    }
    
    Assert(sizeof(tran_state) <= Memory->TransientStorageSize);
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(tran_state), (u8 *)Memory->TransientStorage + sizeof(tran_state));
        memory_arena *TranArena = &TranState->TranArena;
        
        
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
        
        TranState->Assets = AllocateGameAssets(TranArena, Megabytes(128), TranState);
        Frame->InitializeSkyboxTexture = true;
        
        // TODO(ezexff): Only for test
        task_with_memory *Task = BeginTaskWithMemory(TranState);
        if(Task)
        {
            test_work *Work = PushStruct(&Task->Arena, test_work);
            Work->Task = Task;
            
            Platform.AddEntry(TranState->LowPriorityQueue, TestWork, Work);
        }
        
        // NOTE(ezexff): Init ground buffers
        TranState->GroundBufferCount = 64;
        TranState->GroundBuffers = PushArray(TranArena, TranState->GroundBufferCount, ground_buffer);
        for(u32 GroundBufferIndex = 0;
            GroundBufferIndex < TranState->GroundBufferCount;
            ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
            GroundBuffer->P = NullPosition();
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
    
    asset_type *Type = TranState->Assets->AssetTypes + Asset_Skybox;
    u32 SkyboxBitmapCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
    if(SkyboxBitmapCount >= 6)
    {
        u32 Index = 0;
        for(u32 AssetIndex = Type->FirstAssetIndex;
            AssetIndex < Type->FirstAssetIndex + 6;
            AssetIndex++)
        {
            asset *Asset = TranState->Assets->Assets + AssetIndex;
            eab_bitmap EABBitmap = Asset->EAB.Bitmap;
            
            bitmap_id ID = {};
            ID.Value = AssetIndex;
            if(Asset->State == AssetState_Loaded)
            {
                loaded_bitmap *Bitmap = GetBitmap(TranState->Assets, ID, true);
                Frame->Skybox[Index] = *Bitmap;
            }
            else if(Asset->State == AssetState_Unloaded)
            {
                LoadBitmap(TranState->Assets, ID, true);
                Frame->MissingResourceCount++;
            }
            Index++;
        }
    }
    
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
    
    // NOTE(ezexff): ImGui windows
#if ENGINE_INTERNAL
    imgui *ImGuiHandle = &Frame->ImGuiHandle;
    
    if(ImGuiHandle->ShowImGuiWindows)
    {
        // NOTE(ezexff): ImGui game window
        if(ImGuiHandle->ShowGameWindow)
        {
            ImGui::Begin("Game", &ImGuiHandle->ShowGameWindow);
            ImGui::Text("Debug window for game layer...");
            
            ImGui::Checkbox("Sim Region Window", &ImGuiHandle->ShowSimRegionWindow);
            
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
                ImGui::SeparatorText("PlayingSoundInfo");
                playing_sound *PlayingSound = GameState->PlayingSound;
                if(PlayingSound)
                {
                    ImGui::BulletText("CurrentVolume = %f %f", 
                                      PlayingSound->CurrentVolume.x, PlayingSound->CurrentVolume.y);
                    ImGui::BulletText("dCurrentVolume = %f %f", 
                                      PlayingSound->dCurrentVolume.x, PlayingSound->dCurrentVolume.y);
                    ImGui::BulletText("TargetVolume = %f %f", PlayingSound->TargetVolume.x, PlayingSound->TargetVolume.y);
                    ImGui::BulletText("dSample = %f", PlayingSound->dSample);
                    ImGui::BulletText("dSample = %f", PlayingSound->dSample);
                    ImGui::BulletText("sound_id = %d", PlayingSound->ID.Value);
                    ImGui::BulletText("SamplesPlayed = %f", PlayingSound->SamplesPlayed);
                }
                else
                {
                    ImGui::BulletText("Sound isn't playing");
                }
                
                
                ImGui::Checkbox("TestSineWave", &GameState->IsTestSineWave);
                
                ImGui::SeparatorText("Sine wave mixer");
                ImGui::SliderInt("tone hz", &GameState->ToneHz, 20, 20000);
                int ToneVolume = (int)GameState->ToneVolume;
                ImGui::SliderInt("tone volume", &ToneVolume, 0, 20000);
                GameState->ToneVolume = (s16)ToneVolume;
                
                ImGui::SeparatorText("Loaded sound");
                if (ImGui::Button("Play"))
                {
                    GameState->PlayingSound = PlaySound(&GameState->AudioState, GetFirstSoundFrom(TranState->Assets, Asset_Bloop));
                    //GameState->SampleIndex = 0;
                    Log->Add("[test]: Play sound button pressed!\n");
                }
            }
            
            if(ImGui::CollapsingHeader("Frame"))
            {
                ImGui::SeparatorText("Push buffer");
                ImGui::Text("MaxSize = %d", Frame->MaxPushBufferSize);
                ImGui::Text("Size = %d", Frame->PushBufferSize);
                
                ImGui::SeparatorText("Camera");
                camera *Camera = &Frame->Camera;
                ImGui::Text("Pos: %.3f %.3f %.3f", Camera->P.x, Camera->P.y, Camera->P.z);
                ImGui::Text("Ang: %.3f %.3f %.3f", Camera->Angle.x, Camera->Angle.y, Camera->Angle.z);
                ImGui::Text("Vel: %.3f", 0);
                
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
                
                ImGui::SeparatorText("Frag effects");
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
                
                ImGui::SeparatorText("Shaders editor");
                ImGui::Checkbox("Visibility", &ImGuiHandle->ShowFrameShadersEditorWindow);
                
                ImGui::SeparatorText("Variables");
                ImGui::Text("RenderTexture ID = %d", Frame->ColorTexture);
                ImGui::Text("DepthTexture ID = %d", Frame->DepthTexture);
                ImGui::Text("FBO ID = %d", Frame->FBO);
                ImGui::Text("Vert ID = %d", Frame->Vert.ID);
                ImGui::Text("Frag ID = %d", Frame->Frag.ID);
                ImGui::Text("Program ID = %d", Frame->Program.ID);
                ImGui::Text("Effect ID = %d", Frame->FragEffect);
            }
            
            if(ImGui::CollapsingHeader("Input"))
            {
                // NOTE(ezexff): Mouse input
                ImGui::SeparatorText("Mouse");
                ImGui::Text("Pos: %d %d", Input->MouseP.x, Input->MouseP.y);
                ImGui::Text("Delta: %d %d", Input->MouseDelta.x, Input->MouseDelta.y);
                if(Input->MouseDelta.z != 0)
                {
                    ImGui::SameLine();
                    ImGui::Text(" %d", Input->MouseDelta.z);
                }
                ImGui::Checkbox("Log mouse input", &ImGuiHandle->LogMouseInput);
                
                if(ImGuiHandle->LogMouseInput)
                {
                    if(Input->MouseDelta.z != 0)
                    {
                        Log->Add("[input]: Scroll delta = %d\n", Input->MouseDelta.z);
                    }
                    
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
                    {
                        Log->Add("[input]: VK_LBUTTON was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Middle]))
                    {
                        Log->Add("[input]: VK_MBUTTON was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
                    {
                        Log->Add("[input]: VK_RBUTTON was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Extended0]))
                    {
                        Log->Add("[input]: VK_XBUTTON1 was pressed\n");
                    }
                    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Extended1]))
                    {
                        Log->Add("[input]: VK_XBUTTON2 was pressed\n");
                    }
                }
                
                // NOTE(ezexff): Keyboard input
                ImGui::SeparatorText("Keyboard");
                ImGui::Checkbox("Log keyboard input", &ImGuiHandle->LogKeyboardInput);
                
                if(ImGuiHandle->LogKeyboardInput)
                {
                    for(int ControllerIndex = 0;
                        ControllerIndex < ArrayCount(Input->Controllers);
                        ++ControllerIndex)
                    {
                        game_controller_input *Controller = GetController(Input, ControllerIndex);
                        
                        if(WasPressed(Controller->MoveUp))
                        {
                            Log->Add("[input]: MoveUp was pressed\n");
                        }
                        if(WasPressed(Controller->MoveDown))
                        {
                            Log->Add("[input]: MoveDown was pressed\n");
                        }
                        if(WasPressed(Controller->MoveLeft))
                        {
                            Log->Add("[input]: MoveLeft was pressed\n");
                        }
                        if(WasPressed(Controller->MoveRight))
                        {
                            Log->Add("[input]: MoveRight was pressed\n");
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
                
                //ImGui::SeparatorText("Memory blocks");
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
                /*
ImGui::BulletText("Size = %d MB or %d KB or %d bytes",
                                  Assets->Arena.Size / Megabytes(1),
                                  Assets->Arena.Size / Kilobytes(1),
                                  Assets->Arena.Size);
                ImGui::BulletText("Used = %d MB or %d KB or %d bytes",
                                  Assets->Arena.Used / Megabytes(1),
                                  Assets->Arena.Used / Kilobytes(1),
                                  Assets->Arena.Used);
                
                
                ImGui::SeparatorText("Variables");
                ImGui::Text("TargetMemoryUsed = %d MB or %d KB or %d bytes", 
                            Assets->TargetMemoryUsed / Megabytes(1),
                            Assets->TargetMemoryUsed / Kilobytes(1),
                            Assets->TargetMemoryUsed);
                ImGui::Text("TotalMemoryUsed = %d MB or %d KB or %d bytes",
                            Assets->TotalMemoryUsed / Megabytes(1),
                            Assets->TotalMemoryUsed / Kilobytes(1),
                            Assets->TotalMemoryUsed);
*/
                
                ImGui::SeparatorText("Types");
                
                // NOTE(ezexff): asset_type_id
                char *TypeNames[] =
                {
                    "None",
                    "Grass",
                    "Tuft",
                    "Stone",
                    "Clip",
                    "Ground",
                    "Skybox",
                    "Bloop",
                    "Music",
                    "Font",
                    "FontGlyph"
                };
                
                for(u32 TypeID = 1;
                    TypeID < Asset_Count;
                    TypeID++)
                {
                    
                    asset_type *Type = Assets->AssetTypes + TypeID;
                    u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                    if(AssetCount > 0)
                    {
                        ImGui::BulletText("%sCount = %d", TypeNames[TypeID], AssetCount);
                    }
                }
                
                ImGui::SeparatorText("EAB tree");
                if(ImGui::TreeNode("Bitmaps"))
                {
                    for(u32 TypeID = Asset_Grass;
                        TypeID <= Asset_Skybox;
                        TypeID++)
                    {
                        asset_type *Type = Assets->AssetTypes + TypeID;
                        u32 AssetCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
                        
                        if(TypeID == Asset_Skybox)
                        {
                            int fsdfdsf = 0;
                        }
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
                        //LoadFont(TranState->Assets, FirstFont);
                        font_id FirstFont = GetFirstFontFrom(TranState->Assets, Asset_Font);
                        
                        u32 FirstFontAsset = GetFirstAssetFrom(TranState->Assets, Asset_Font);
                        asset *Asset = Assets->Assets + FirstFontAsset;
                        
                        if(Asset->State == AssetState_Loaded)
                        {
                            loaded_font *Font = GetFont(Assets, FirstFont, true);
                            int dsg32432 = 54645;
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
                                    
                                    char *TagNames[] = 
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
                
                ImGui::SeparatorText("Camera");
                camera *Camera = 0;
                switch(GameState->GameMode)
                {
                    case GameMode_Test:
                    {
                        Camera = &GameState->ModeTest.Camera;
                    } break;
                    
                    case GameMode_World:
                    {
                        //Camera = &GameState->ModeWorld.Camera;
                    } break;
                    
                    InvalidDefaultCase;
                }
                if(Camera != 0)
                {
                    ImGui::Text("Pos");
                    ImGui::InputFloat("x", &Camera->P.x, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("y", &Camera->P.y, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("z", &Camera->P.z, 0.01f, 1.0f, "%.3f");
                    ImGui::Text("Angle");
                    ImGui::InputFloat("Pitch", &Camera->Angle.x, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("Yaw", &Camera->Angle.y, 0.01f, 1.0f, "%.3f");
                    ImGui::InputFloat("Roll", &Camera->Angle.z, 0.01f, 1.0f, "%.3f");
                }
                
                float *ClearColor = 0;
                switch(GameState->GameMode)
                {
                    case GameMode_Test:
                    {
                        ClearColor = (float*)&GameState->ModeTest.ClearColor;
                    } break;
                    
                    case GameMode_World:
                    {
                        ClearColor = (float*)&GameState->ModeWorld.ClearColor;
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
                OpenglCompileShader(Opengl, GL_VERTEX_SHADER, &Frame->Vert);
                OpenglCompileShader(Opengl, GL_FRAGMENT_SHADER, &Frame->Frag);
                OpenglLinkProgram(Opengl, &Frame->Program, &Frame->Vert, &Frame->Frag);
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
            //ImGui::Text("gkdfjglfdg");
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