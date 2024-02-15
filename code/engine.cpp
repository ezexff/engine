#include "engine.h"

#include "engine_renderer.cpp"
#include "engine_mode_test.cpp"
#include "engine_mode_world.cpp"

extern "C" UPDATE_AND_RENDER_FUNC(UpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!GameState->IsInitialized)
    {
        InitializeArena(&GameState->ConstArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (u8 *)Memory->PermanentStorage + sizeof(game_state));
        memory_arena *ConstArena = &GameState->ConstArena;
        
        // NOTE(ezexff): Init Log App
        {
#if ENGINE_INTERNAL
            imgui *ImGuiHandle = &Memory->Frame.ImGuiHandle;
            ImGui::SetCurrentContext(ImGuiHandle->Context);
            ImGui::SetAllocatorFunctions(ImGuiHandle->AllocFunc, ImGuiHandle->FreeFunc, ImGuiHandle->UserData);
            Log = &Memory->Frame.ImGuiHandle.Log;
            Log->Add("[test]: Hello %d world\n", 123);
            Log->Add("[test]: 567657657\n");
            Log->Add("[test]: 1\n");
            Log->Add("[test]: 2\n");
            
            ImGuiHandle->LogMouseInput = false;
            ImGuiHandle->LogKeyboardInput = false;
#endif
        }
        
        // NOTE(ezexff): Init game mode
        GameState->GameMode = GameMode_World;
        
        // NOTE(ezexff): Init sound mixer
        {
            GameState->ToneHz = 512;
            GameState->ToneVolume = 100;
            GameState->tSine = 0;
            GameState->SampleIndex = 0;
            
            GameState->LoadedSound = LoadFirstWAV(&GameState->ConstArena, &Memory->PlatformAPI);
#if ENGINE_INTERNAL
            if(!GameState->LoadedSound.SampleCount == 0)
            {
                Log->Add("[asset]: Sound successfully loaded\n");
            }
            else
            {
                Log->Add("[asset]: Sound loading failed\n");
            }
#endif
        }
        
        GameState->IsInitialized = true;
    }
    
    Assert(sizeof(tran_state) <= Memory->TransientStorageSize);
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(tran_state), (u8 *)Memory->TransientStorage + sizeof(tran_state));
        memory_arena *TranArena = &TranState->TranArena;
        
        TranState->IsInitialized = true;
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
    
    // NOTE(ezexff): ImGui game window
    {
#if ENGINE_INTERNAL
        renderer_frame *Frame = &Memory->Frame;
        imgui *ImGuiHandle = &Frame->ImGuiHandle;
        if(ImGuiHandle->ShowImGuiWindows && ImGuiHandle->ShowGameWindow)
        {
            ImGui::Begin("Game");
            ImGui::Text("Debug window for game layer...");
            
            ImGui::SeparatorText("Camera");
            camera *Camera = &Frame->Camera;
            ImGui::Text("Pos: %.3f %.3f %.3f", Camera->P.x, Camera->P.y, Camera->P.z);
            ImGui::Text("Ang: %.3f %.3f %.3f", Camera->Angle.x, Camera->Angle.y, Camera->Angle.z);
            ImGui::Text("Vel: %.3f", 0);
            
            ImGui::SeparatorText("Subsystems");
            if(ImGui::CollapsingHeader("Memory"))
            {
                ImGui::SeparatorText("ConstArena");
                ImGui::Text("Size = %d MB or %d KB or %d bytes",
                            GameState->ConstArena.Size / Megabytes(1),
                            GameState->ConstArena.Size / Kilobytes(1),
                            GameState->ConstArena.Size);
                ImGui::Text("Used = %d MB or %d KB or %d bytes",
                            GameState->ConstArena.Used / Megabytes(1),
                            GameState->ConstArena.Used / Kilobytes(1),
                            GameState->ConstArena.Used);
                ImGui::Spacing();
                ImGui::SeparatorText("TranArena");
                ImGui::Text("Size = %d MB or %d KB or %d bytes",
                            TranState->TranArena.Size / Megabytes(1),
                            TranState->TranArena.Size / Kilobytes(1),
                            TranState->TranArena.Size);
                ImGui::Text("Used = %d MB or %d KB or %d bytes",
                            TranState->TranArena.Used / Megabytes(1),
                            TranState->TranArena.Used / Kilobytes(1),
                            TranState->TranArena.Used);
                ImGui::Text("TempCount = %d", TranState->TranArena.TempCount);
            }
            
            if(ImGui::CollapsingHeader("Audio"))
            {
                ImGui::SeparatorText("Sine wave mixer");
                ImGui::SliderInt("tone hz", &GameState->ToneHz, 20, 20000);
                int ToneVolume = (int)GameState->ToneVolume;
                ImGui::SliderInt("tone volume", &ToneVolume, 0, 20000);
                GameState->ToneVolume = (s16)ToneVolume;
                
                ImGui::SeparatorText("Loaded sound");
                if (ImGui::Button("Play"))
                {
                    GameState->SampleIndex = 0;
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
                ImGui::Text("Pos");
                ImGui::InputFloat("x", &Camera->P.x, 0.01f, 1.0f, "%.3f");
                ImGui::InputFloat("y", &Camera->P.y, 0.01f, 1.0f, "%.3f");
                ImGui::InputFloat("z", &Camera->P.z, 0.01f, 1.0f, "%.3f");
                ImGui::Text("Angle");
                ImGui::InputFloat("Pitch", &Camera->Angle.x, 0.01f, 1.0f, "%.3f");
                ImGui::InputFloat("Yaw", &Camera->Angle.y, 0.01f, 1.0f, "%.3f");
                ImGui::InputFloat("Roll", &Camera->Angle.z, 0.01f, 1.0f, "%.3f");
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
                        Log->Add("[game input]: Scroll delta = %d\n", Input->MouseDelta.z);
                    }
                    
                    if(Input->MouseButtons[PlatformMouseButton_Left].EndedDown && Input->MouseButtons[PlatformMouseButton_Left].HalfTransitionCount == 1)
                    {
                        Log->Add("[input]: VK_LBUTTON was pressed\n");
                    }
                    if(Input->MouseButtons[PlatformMouseButton_Middle].EndedDown && Input->MouseButtons[PlatformMouseButton_Middle].HalfTransitionCount == 1)
                    {
                        Log->Add("[input]: VK_MBUTTON was pressed\n");
                    }
                    if(Input->MouseButtons[PlatformMouseButton_Right].EndedDown && Input->MouseButtons[PlatformMouseButton_Right].HalfTransitionCount == 1)
                    {
                        Log->Add("[input]: VK_RBUTTON was pressed\n");
                    }
                    if(Input->MouseButtons[PlatformMouseButton_Extended0].EndedDown && Input->MouseButtons[PlatformMouseButton_Extended0].HalfTransitionCount == 1)
                    {
                        Log->Add("[input]: VK_XBUTTON1 was pressed\n");
                    }
                    if(Input->MouseButtons[PlatformMouseButton_Extended1].EndedDown && Input->MouseButtons[PlatformMouseButton_Extended1].HalfTransitionCount == 1)
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
                        
                        if(Controller->MoveUp.EndedDown && Controller->MoveUp.HalfTransitionCount == 1)
                        {
                            Log->Add("[input]: MoveUp was pressed\n");
                        }
                        if(Controller->MoveDown.EndedDown && Controller->MoveDown.HalfTransitionCount == 1)
                        {
                            Log->Add("[input]: MoveDown was pressed\n");
                        }
                        if(Controller->MoveLeft.EndedDown && Controller->MoveLeft.HalfTransitionCount == 1)
                        {
                            Log->Add("[input]: MoveLeft was pressed\n");
                        }
                        if(Controller->MoveRight.EndedDown && Controller->MoveRight.HalfTransitionCount == 1)
                        {
                            Log->Add("[input]: MoveRight was pressed\n");
                        }
                    }
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
                
                switch(GameState->GameMode)
                {
                    case GameMode_Test:
                    {
                        ImGui::SeparatorText("Background");
                        ImGui::ColorEdit4("clear color", (float*)&GameState->Test.ClearColor);
                    } break;
                    
                    case GameMode_World:
                    {
                        ImGui::SeparatorText("Background");
                        ImGui::ColorEdit4("clear color", (float*)&GameState->World.ClearColor);
                    } break;
                    
                    InvalidDefaultCase;
                }
            }
            
            ImGui::End();
        }
#endif
    }
}

extern "C" GET_SOUND_SAMPLES_FUNC(GetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    loaded_sound *LoadedSound = &GameState->LoadedSound;
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
    }
}