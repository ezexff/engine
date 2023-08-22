#include "engine.h"
#include "engine_entity.cpp"
#include "engine_asset.cpp"
#include "simpleproject_opengl.cpp" // TODO(me): для тестов, удалить
#include "engine_render.cpp"
#include "engine_world.cpp"

internal void EngineUpdateAndRender(GLFWwindow *Window, game_memory *Memory, game_input *Input)
{
    Platform = Memory->PlatformAPI;

    //
    // NOTE(me): Memory init
    //
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;

    if(!GameState->IsInitialized)
    {
        // установка указателя выделенной памяти после структуры game_state
        InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *)Memory->PermanentStorage + sizeof(game_state));
        memory_arena *WorldArena = &GameState->WorldArena;
        //
        // NOTE(me): Push game_state structures in memory
        //
        GameState->Render = PushStruct(WorldArena, render);
        render *Render = GameState->Render;
        Render->SStMeshesCount = 0;
        Render->SAnMeshesCount = 0;
        Render->MStMeshesCount = 0;
        Render->Animator.Timer = 1.0f;

        GameState->Settings = PushStruct(WorldArena, app_settings);
        app_settings *Settings = GameState->Settings;
        Settings->RBFullscreenIsActive = false;
        Settings->RBWindowedIsActive = true;
        Settings->MouseSensitivity = 50.0;
        Settings->NewFrameRate = 60;
        Settings->RBCappedIsActive = true;
        Settings->RBUncappedIsActive = false;
        Settings->RBVSyncIsActive = false;
        Settings->ProcessAnimations = true;

        GameState->Player = PushStruct(WorldArena, entity_player);
        entity_player *Player = GameState->Player;
        Player->Position = V3(5, 5, 0);
        Player->dP = V2(0, 0);
        Player->CameraPitch = 90.0f;
        Player->CameraYaw = -45.0f;
        Player->CameraZOffset = 2.7f;
        Player->Width = 0.5f;
        Player->Height = 0.5f;
        Player->CameraPitchInversed = Player->CameraPitch;

        GameState->Clip = PushStruct(WorldArena, entity_clip);
        entity_clip *PlayerClip = GameState->Clip;
        PlayerClip->CenterPos = V2(-50.0f, 50.0f);
        PlayerClip->Side = 100.0f;

        //
        // NOTE(me): Источники света
        //
        // directional light
        directional_light *DirLight = &Render->DirLight;
        DirLight->Base.Color = V3(0.5f, 0.5f, 0.5f);
        DirLight->Base.AmbientIntensity = 0.1f;
        DirLight->Base.DiffuseIntensity = 1.0f;
        DirLight->WorldDirection = V3(1.0f, 1.0f, -1.0f);

        // point lights
        Render->PointLightsCount = 1;
        Render->PointLights = PushArray(WorldArena, Render->PointLightsCount, point_light);
        point_light *PointLights = Render->PointLights;
        PointLights[0].Base.Color = V3(0.0f, 0.0f, 1.0f);
        PointLights[0].Base.AmbientIntensity = 1.0f;
        PointLights[0].Base.DiffuseIntensity = 4.0f;
        PointLights[0].WorldPosition = V3(15.0f, 14.0f, 5.0f);
        PointLights[0].Atten.Constant = 1.0f;
        PointLights[0].Atten.Linear = 0.1f; // 0.0f
        PointLights[0].Atten.Exp = 0.0f;    // 0.0f
        // имена переменных point lights для оправки в шейдер
        Render->PLVarNames = PushArray(WorldArena, Render->PointLightsCount, point_light_var_names);
        for(u32 i = 0; i < Render->PointLightsCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.Color", i);
            Render->PLVarNames[i].VarNames[0] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.AmbientIntensity", i);
            Render->PLVarNames[i].VarNames[1] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.DiffuseIntensity", i);
            Render->PLVarNames[i].VarNames[2] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].WorldPos", i);
            Render->PLVarNames[i].VarNames[3] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Constant", i);
            Render->PLVarNames[i].VarNames[4] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Linear", i);
            Render->PLVarNames[i].VarNames[5] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Exp", i);
            Render->PLVarNames[i].VarNames[6] = PushStringZ(WorldArena, TmpName);
        }

        // spot lights
        Render->SpotLightsCount = 1;
        Render->SpotLights = PushArray(WorldArena, Render->SpotLightsCount, spot_light);
        spot_light *SpotLights = Render->SpotLights;
        SpotLights[0].Base.Base.Color = V3(1.0f, 0.0f, 0.0f);
        SpotLights[0].Base.Base.AmbientIntensity = 1.0f;
        SpotLights[0].Base.Base.DiffuseIntensity = 1.0f;
        SpotLights[0].Base.WorldPosition = V3(5.5f, 5.0f, 12.0f);
        SpotLights[0].Base.Atten.Constant = 1.0f;
        SpotLights[0].Base.Atten.Linear = 0.01f;
        SpotLights[0].Base.Atten.Exp = 0.0f;
        SpotLights[0].WorldDirection = V3(0.0f, 0.0f, -1.0f);
        SpotLights[0].WorldDirection = Normalize(SpotLights[0].WorldDirection);
        SpotLights[0].Cutoff = 0.9f;
        // имена переменных spot lights для оправки в шейдер
        Render->SLVarNames = PushArray(WorldArena, Render->SpotLightsCount, spot_light_var_names);
        for(u32 i = 0; i < Render->SpotLightsCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.Color", i);
            Render->SLVarNames[i].VarNames[0] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
            Render->SLVarNames[i].VarNames[1] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
            Render->SLVarNames[i].VarNames[2] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.WorldPos", i);
            Render->SLVarNames[i].VarNames[3] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Constant", i);
            Render->SLVarNames[i].VarNames[4] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Linear", i);
            Render->SLVarNames[i].VarNames[5] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Exp", i);
            Render->SLVarNames[i].VarNames[6] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Direction", i);
            Render->SLVarNames[i].VarNames[7] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Cutoff", i);
            Render->SLVarNames[i].VarNames[8] = PushStringZ(WorldArena, TmpName);
        }

        //
        // NOTE(me): Объекты окружения (2d-текстуры и 3d-модели)
        //
        for(u32 i = 0; i < ENV_OBJECTS_MAX; i++)
        {
            GameState->EnvObjects[i] = PushStruct(WorldArena, entity_envobject);
        }

        // Единичные матрицы объектам окружения
        for(u32 i = 0; i < ENV_OBJECTS_MAX; i++)
        {
            GameState->EnvObjects[i]->TranslateMatrix = Identity();
            GameState->EnvObjects[i]->RotateMatrix = Identity();
            GameState->EnvObjects[i]->ScaleMatrix = Identity();
        }

        u32 EnvIndex = 0;
        entity_envobject **EnvObjects = GameState->EnvObjects;

        // Террейн
        EnvObjects[EnvIndex]->Model = CreateTerrainModel(WorldArena);
        EnvIndex++;

        // маркер позиции точечного источника освещения
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90);
        //  EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_cube.spm");
        EnvObjects[EnvIndex]->Model = CreateTexturedSquareModel(WorldArena, "lamp.png");
        EnvIndex++;

        // маркер позиции прожектора
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90);
        EnvObjects[EnvIndex]->Model = EnvObjects[1]->Model;
        EnvIndex++;

        // ваза
        EnvObjects[EnvIndex]->InstancingCount = 100;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(2, 4, 1),                           // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),  // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),  // Rotate Y rand() Min, Max, Precision
                                              V3(0, 0, 0)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_vase.spm");
        EnvIndex++;

        // бочка
        EnvObjects[EnvIndex]->InstancingCount = 50;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(1.0, 2.0, 1),                       // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_barrel.spm");
        EnvIndex++;

        // дерево
        EnvObjects[EnvIndex]->InstancingCount = 10;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.7, 0.7, 1),                       // Scale rand() Min, Max, Precision
                                              V3(90, 0, 0),                          // Rotate X, Y, Z
                                              V3(0, 0, 0),   // Rotate X rand() Min, Max, Precision
                                              V3(0, 360, 1), // Rotate Y rand() Min, Max, Precision
                                              V3(0, 0, 0));  // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_tree.spm");
        EnvIndex++;

        // ковбой (анимированный)
        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(2, 2, 0));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.4f);
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_cowboy.spm");
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(2, 2 + 3, 0));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.4f);
        EnvObjects[EnvIndex]->Model = EnvObjects[6]->Model;
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(2, 2 + 6, 0));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.4f);
        EnvObjects[EnvIndex]->Model = EnvObjects[6]->Model;
        EnvIndex++;

        // страж (анимированный)
        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(5, 10 + 2, 0));
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90.0f);
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.1f);
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_guard.spm");
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(5 + 3, 10 + 2, 0));
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90.0f);
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.1f);
        EnvObjects[EnvIndex]->Model = EnvObjects[9]->Model;
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(5 + 6, 10 + 2, 0));
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90.0f);
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.1f);
        EnvObjects[EnvIndex]->Model = EnvObjects[9]->Model;
        EnvIndex++;

        // clip wall texture
        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(-100.0f, 0.0f, 0.0f));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(100.0f);
        EnvObjects[EnvIndex]->Model = CreateTexturedSquareModel(WorldArena, "clip.png");
        EnvIndex++;

        // clip player texture
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.05f);
        EnvObjects[EnvIndex]->Model = CreateTexturedSquareModel(WorldArena, "clip.png");
        EnvIndex++;

        // трава 1
        EnvObjects[EnvIndex]->InstancingCount = 150;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass1.spm");
        EnvIndex++;

        // трава 2
        EnvObjects[EnvIndex]->InstancingCount = 150;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass2.spm");
        EnvIndex++;

        // трава 3
        EnvObjects[EnvIndex]->InstancingCount = 20000;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass3.spm");
        EnvIndex++;

        // трава 4
        EnvObjects[EnvIndex]->InstancingCount = 500;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass4.spm");
        EnvIndex++;

        // трава 5
        EnvObjects[EnvIndex]->InstancingCount = 50;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass5.spm");
        EnvIndex++;

        // трава 6
        EnvObjects[EnvIndex]->InstancingCount = 20;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass6.spm");
        EnvIndex++;

        // трава 7
        EnvObjects[EnvIndex]->InstancingCount = 50;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass7.spm");
        EnvIndex++;

        // трава 8
        EnvObjects[EnvIndex]->InstancingCount = 50;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass8.spm");
        EnvIndex++;

        // трава 9
        EnvObjects[EnvIndex]->InstancingCount = 20;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass9.spm");
        EnvIndex++;

        // трава 10
        EnvObjects[EnvIndex]->InstancingCount = 20;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1),                      // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_grass10.spm");
        EnvIndex++;

        /*
        // трава (старая версия)
        EnvObjects[EnvIndex]->InstancingCount = 500;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(2, 3, 1),                           // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),  // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),  // Rotate Y rand() Min, Max, Precision
                                              V3(0, 0, 0)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = CreateGrassModel(WorldArena);
        EnvIndex++;
        */

        Assert(EnvIndex <= ENV_OBJECTS_MAX);
        Render->EnvCount = EnvIndex - 1;

        // высота объектов окружения на ландшафте
        for(u32 i = 3; i < Render->EnvCount - 1; i++)
        {
            // EnvObjects[i]->Position.z +=
            //     EnvObjects[i]->Position.z +
            //     TerrainGetHeight(EnvObjects[0], EnvObjects[i]->Position.x, EnvObjects[i]->Position.y);

            EnvObjects[i]->TranslateMatrix.E[2][3] +=
                EnvObjects[i]->TranslateMatrix.E[2][3] + TerrainGetHeight(EnvObjects[0],
                                                                          EnvObjects[i]->TranslateMatrix.E[0][3],
                                                                          EnvObjects[i]->TranslateMatrix.E[1][3]);
        }

        //
        // NOTE(me): Шейдеры и VBO
        //
        u32 Shader1Vert = LoadShader("../code/shaders/Default.vert", GL_VERTEX_SHADER);
        u32 Shader1Frag = LoadShader("../code/shaders/Default.frag", GL_FRAGMENT_SHADER);
        Render->DefaultShaderProgram = LinkShaderProgram(Shader1Vert, Shader1Frag);

        u32 Shader2Vert = LoadShader("../code/shaders/Water.vert", GL_VERTEX_SHADER);
        u32 Shader2Frag = LoadShader("../code/shaders/Water.frag", GL_FRAGMENT_SHADER);
        Render->WaterShaderProgram = LinkShaderProgram(Shader2Vert, Shader2Frag);

        u32 Shader3Vert = LoadShader("../code/shaders/DepthMap.vert", GL_VERTEX_SHADER);
        u32 Shader3Frag = LoadShader("../code/shaders/DepthMap.frag", GL_FRAGMENT_SHADER);
        Render->DepthMapShaderProgram = LinkShaderProgram(Shader3Vert, Shader3Frag);

        // Добавление объектов окружения в рендерер и создание VBO
        AddEnvObjectsToRender(Render, EnvObjects);
        InitEnvVBOs(WorldArena, Render);

        //
        // NOTE(me): Water
        //
        Render->WaterMoveFactor = 0; // only for calc
        Render->ReflWidth = 1920 / 4;
        Render->ReflHeight = 1080 / 4;
        Render->RefrWidth = 1920 / 2;
        Render->RefrHeight = 1080 / 2;
        Render->WaterWaveSpeed = 0.4f;
        Render->WaterTiling = 0.5f;
        Render->WaterWaveStrength = 0.02f;
        Render->WaterShineDamper = 20.0f;
        Render->WaterReflectivity = 0.6f;
        // Render->WaterDUDVTextureName = PushString(WorldArena, "WaterDUDV.png");
        Render->WaterDUDVTextureName = PushString(WorldArena, "NewWaterDUDV.png");
        Render->WaterDUDVTexture = LoadTexture(&Render->WaterDUDVTextureName);
        // Render->WaterNormalMapName = PushString(WorldArena, "WaterNormalMap.png");
        Render->WaterNormalMapName = PushString(WorldArena, "NewWaterNormalMap.png");
        Render->WaterNormalMap = LoadTexture(&Render->WaterNormalMapName);
        InitWaterFBOs(Render);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }

        //
        // NOTE(me): Shadows
        //
        Render->DepthMapWidth = 1920 * 2;
        Render->DepthMapHeight = 1080 * 2;
        Render->ShadowMapSize = 68.0f;
        Render->NearPlane = 50.0f;
        Render->FarPlane = 144.0f;
        Render->ShadowLightPitch = 51.5f;
        Render->ShadowLightYaw = 317.0f;
        Render->ShadowLightPos = V3(-3.0f, -6.0f, 73.5f);
        /*Render->ShadowMapSize = 10.0f;
        Render->NearPlane = 0.0f;
        Render->FarPlane = 100.0f;
        Render->ShadowLightPitch = 65.0f;
        Render->ShadowLightYaw = 315.0f;
        Render->ShadowLightPos = V3(-10.0f, -10.0f, 10.0f);*/
        InitDepthMapFBO(Render);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // ImGui Demo Window
        GameState->ShowDemoWindow = false;
        GameState->ShowAnotherWindow = false;

        // String example
        // string TestStr = PushString(WorldArena, "fdsfsdfss");

        // Assert example
        // int32 xxx = 1;
        // Assert(xxx < 0);

        GameState->IsInitialized = true;
    }

    // Local pointers for game_state sctructs
    render *Render = GameState->Render;
    app_settings *Settings = GameState->Settings;
    entity_player *Player = GameState->Player;
    entity_clip *PlayerClip = GameState->Clip;
    entity_envobject **EnvObjects = GameState->EnvObjects;

    //
    // NOTE(me): Inputs
    //
    if(!Input->ShowMouseCursorMode)
    {
        RotatePlayerCamera(Player, Input->MouseOffsetX, Input->MouseOffsetY, Settings->MouseSensitivity);
    }

    for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);

        // Player
        // r32 SpeedBuff = Buffs.Speed.Time > 0 ? 0.2f : 0;
        r32 PlayerSpeed = 50.0f; //+ SpeedBuff; // m/s^2

        v2 ddP = {};
        if(Controller->MoveUp.EndedDown)
        {
            ddP.y = 1.0f;
        }
        if(Controller->MoveDown.EndedDown)
        {
            ddP.y = -1.0f;
        }
        if(Controller->MoveLeft.EndedDown)
        {
            ddP.x = -1.0f;
        }

        if(Controller->MoveRight.EndedDown)
        {
            ddP.x = 1.0f;
        }

        MovePlayerEOM(Player, PlayerClip, ddP, PlayerSpeed, Input->dtForFrame);

        // x, y, z - система координат камеры
        // Высота игрка на тиррейне
        // Camera->z = TerrainGetHeight(World, Camera->x, Camera->y) + 2.7f;

        /*if(Controller->MoveUp.EndedDown)
        // if(Controller->Key1.EndedDown && Controller->Key1.HalfTransitionCount == 1)
        {
            GameState->ClearColor.x += 0.001;
        }
        if(Controller->MoveDown.EndedDown)
        {
            GameState->ClearColor.x -= 0.001;
        }

        if(Controller->MoveLeft.EndedDown)
        {
            GameState->ClearColor.y += 0.001;
        }
        if(Controller->MoveRight.EndedDown)
        {
            GameState->ClearColor.y -= 0.001;
        }*/
    }

    //
    // NOTE(me): Physics
    //
    // Высота камеры игрока на террейне (ландшафте)
    Player->Position.z =
        TerrainGetHeight(EnvObjects[0], Player->Position.x, Player->Position.y) + Player->CameraZOffset;
    // Player->Position.z = Player->CameraZOffset;
    //  Перемещение маркеров источников света
    EnvObjects[1]->TranslateMatrix = Translation(Render->PointLights[0].WorldPosition);
    EnvObjects[2]->TranslateMatrix = Translation(Render->SpotLights[0].Base.WorldPosition);
    // Перемещение клип текстуры игрока
    v3 ClipTextureNewPos = Player->Position - V3(Player->Width * 0.05f, Player->Height * 0.05f, 0);
    EnvObjects[14]->TranslateMatrix = Translation(ClipTextureNewPos);

    // Формируем матрицы преобразований у объектов окружения
    for(u32 i = 0; i < Render->EnvCount + 1; i++)
    {
        if(EnvObjects[i]->InstancingCount == 0)
        {
            EnvObjects[i]->TransformMatrix =
                EnvObjects[i]->TranslateMatrix * EnvObjects[i]->RotateMatrix * EnvObjects[i]->ScaleMatrix;
            EnvObjects[i]->TransformMatrix = Transpose(EnvObjects[i]->TransformMatrix); // opengl to glsl format
        }
    }

    // Обработка анимаций
    if(Render->Animator.Timer > 0.0f)
    {
        if(Settings->ProcessAnimations)
        {
            for(u32 i = 0; i < Render->SAnMeshesCount; i++)
            {
                single_mesh *Mesh = Render->SAnMeshes[i];
                GetBoneTransforms(Mesh, //
                                  0,    // индекс анимации
                                  Render->Animator.Timer);
            }
        }

        Render->Animator.Timer -= Input->dtForFrame;
        if(Render->Animator.Timer <= 0.0f)
        {
            // Debug->Animator.Timer = 0.0f;
            Render->Animator.Timer = 5.0f; // начинаем проигрывать анимацию заново
        }
    }

    //
    // NOTE(me): ImGui init render
    //

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its
    // code to learn more about Dear ImGui!).
    if(GameState->ShowDemoWindow && Input->ShowMouseCursorMode)
    {
        ImGui::ShowDemoWindow(&GameState->ShowDemoWindow);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    if(Input->ShowMouseCursorMode)
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window",
                        &GameState->ShowDemoWindow); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &GameState->ShowAnotherWindow);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
        // ImGui::ColorEdit3("clear color", (float *)&GameState->ClearColor); // Edit 3 floats representing a color

        if(ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if(GameState->ShowAnotherWindow && Input->ShowMouseCursorMode)
    {
        ImGui::Begin("Another Window",
                     &GameState->ShowAnotherWindow); // Pass a pointer to our bool variable (the window will have a
                                                     // closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if(ImGui::Button("Close Me"))
            GameState->ShowAnotherWindow = false;
        ImGui::End();
    }

    // NOTE(me): Developer Menu
    if(Input->ShowMouseCursorMode)
    {
        ImGui::Begin("Developer Menu");
        ImGui::Text("This is test engine!");
        ImGui::Text("App avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Spacing();

        if(ImGui::CollapsingHeader("Variables"))
        {
            ImGui::InputFloat("Player X", &Player->Position.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("Player Y", &Player->Position.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("CameraZOffset", &Player->CameraZOffset, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("CameraPitch", &Player->CameraPitch, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("CameraYaw", &Player->CameraYaw, 0.5, 2, "%.10f", 0);
            ImGui::Text("CursorPos=%f,%f", Input->MouseX, Input->MouseY);
            ImGui::Text("dtForFrame=%f", Input->dtForFrame);
            ImGui::Text("MOffset=%f,%f", Input->MouseOffsetX, Input->MouseOffsetY);
            ImGui::Text("SStMeshesCount=%d", Render->SStMeshesCount);
            ImGui::Text("SStVerticesCountSum=%d", Render->SStVerticesCountSum);
            ImGui::Text("SAnMeshesCount=%d", Render->SAnMeshesCount);
            ImGui::Text("SAnVerticesCountSum=%d", Render->SAnVerticesCountSum);
            ImGui::Text("MStMeshesCount=%d", Render->MStMeshesCount);
            ImGui::Text("MStVerticesCountSum=%d", Render->MStVerticesCountSum);
            ImGui::Text("EnvCount=%d", Render->EnvCount);
        }

        if(ImGui::CollapsingHeader("Settings"))
        {
            ImGui::Text("Change Display Mode");
            ImGui::SameLine();
            b32 ChangeDisplayMode = false; // менять ли текущий режим отображения окна win32?
            if(ImGui::RadioButton("Fullscreen", Settings->RBFullscreenIsActive))
            {
                if(!Settings->RBFullscreenIsActive) // если rb1 выключена и произошло нажатие
                {
                    Settings->RBFullscreenIsActive = true; // включаем rb1
                    Settings->RBWindowedIsActive = false;  // выключаем rb2
                    ChangeDisplayMode = true;
                }
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("Windowed", Settings->RBWindowedIsActive))
            {
                if(!Settings->RBWindowedIsActive) // если rb2 выключена и произошло нажатие
                {
                    Settings->RBWindowedIsActive = true;    // включаем rb2
                    Settings->RBFullscreenIsActive = false; // выключаем rb2
                    ChangeDisplayMode = true;
                }
            }

            if(ChangeDisplayMode)
            {
                Platform.ToggleFullscreen(Window);
            }
            ImGui::Spacing();

            b32 ToggleFrameRateCap = false;
            b32 ToggleVSync = false;
            ImGui::Text("Change Frame Rate Mode");
            ImGui::SameLine();
            if(ImGui::RadioButton("Capped", Settings->RBCappedIsActive))
            {
                if(!Settings->RBCappedIsActive)
                {
                    if(Settings->RBUncappedIsActive)
                    {
                        ToggleFrameRateCap = true;
                    }
                    if(Settings->RBVSyncIsActive)
                    {
                        ToggleVSync = true;
                        ToggleFrameRateCap = true;
                    }
                    Settings->RBCappedIsActive = true;
                    Settings->RBUncappedIsActive = false;
                    Settings->RBVSyncIsActive = false;
                }
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("Uncapped", Settings->RBUncappedIsActive))
            {
                if(!Settings->RBUncappedIsActive)
                {
                    if(Settings->RBCappedIsActive)
                    {
                        ToggleFrameRateCap = true;
                    }
                    if(Settings->RBVSyncIsActive)
                    {
                        ToggleVSync = true;
                    }
                    Settings->RBCappedIsActive = false;
                    Settings->RBUncappedIsActive = true;
                    Settings->RBVSyncIsActive = false;
                }
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("VSync", Settings->RBVSyncIsActive))
            {
                if(!Settings->RBVSyncIsActive)
                {
                    if(Settings->RBCappedIsActive)
                    {
                        ToggleFrameRateCap = true;
                    }
                    Settings->RBUncappedIsActive = false;
                    Settings->RBCappedIsActive = false;
                    Settings->RBVSyncIsActive = true;
                    ToggleVSync = true;
                }
            }

            if(ToggleVSync)
            {
                Platform.ToggleVSync();
            }
            if(ToggleFrameRateCap)
            {
                Platform.ToggleFrameRateCap();
            }
            ImGui::Spacing();

            // TODO(me): vsync/unlimited fps/custom fps???
            if(Settings->RBCappedIsActive)
            {
                ImGui::Text("Set FrameRate");
                ImGui::SliderInt("##FrameRate", &Settings->NewFrameRate, 60, 240);
                Platform.SetFrameRate(Settings->NewFrameRate);
            }
            ImGui::Spacing();

            ImGui::Text("Change Mouse Sensitivity");
            ImGui::SliderFloat("##MouseSensitivity", &Settings->MouseSensitivity, 0.0f, 100.0f);
            ImGui::Spacing();

            ImGui::Checkbox("Process Animations", &Settings->ProcessAnimations);
        }

        if(ImGui::CollapsingHeader("Memory"))
        {
            ImGui::Text("PermanentStorage");
            ImGui::Text("Size=%d (Kb)", GameState->WorldArena.Size / 1024);
            ImGui::Text("Used=%d (Kb)", GameState->WorldArena.Used / 1024);
        }

        if(ImGui::CollapsingHeader("Water"))
        {
            ImGui::InputFloat("WaterWaveSpeed", &Render->WaterWaveSpeed, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("WaterTiling", &Render->WaterTiling, 0.1, 2, "%.10f", 0);
            ImGui::InputFloat("WaterWaveStrength", &Render->WaterWaveStrength, 0.1, 2, "%.10f", 0);
            ImGui::InputFloat("WaterShineDamper", &Render->WaterShineDamper, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("WaterReflectivity", &Render->WaterReflectivity, 0.1, 2, "%.10f", 0);
            ImGui::Text("WaterReflTexture");
            ImGui::Image((void *)(intptr_t)Render->WaterReflTexture, ImVec2(1920 / 4, 1080 / 4), //
                         ImVec2(0, 0), ImVec2(1, -1));
            ImGui::Text("WaterRefrTexture");
            ImGui::Image((void *)(intptr_t)Render->WaterRefrTexture, ImVec2(1920 / 4, 1080 / 4), //
                         ImVec2(0, 0), ImVec2(1, -1));
        }

        if(ImGui::CollapsingHeader("Shadows"))
        {
            ImGui::InputFloat("ShadowMapSize", &Render->ShadowMapSize, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("NearPlane", &Render->NearPlane, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("FarPlane", &Render->FarPlane, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowLightPitch", &Render->ShadowLightPitch, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowLightYaw", &Render->ShadowLightYaw, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowLightPosX", &Render->ShadowLightPos.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowLightPosY", &Render->ShadowLightPos.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowLightPosZ", &Render->ShadowLightPos.z, 0.5, 2, "%.10f", 0);
            ImGui::Text("ShadowMap");
            ImGui::Image((void *)(intptr_t)Render->DepthMap, ImVec2(1920 / 4, 1080 / 4), ImVec2(0, 0), ImVec2(1, -1));
        }

        if(ImGui::CollapsingHeader("Light"))
        {
            /*
            ImGui::Text("Directional");
            ImGui::SliderFloat("DLPosX", &Render->DirLight.WorldDirection.x, -50.0f, 50.0f);
            ImGui::SliderFloat("DLPosY", &Render->DirLight.WorldDirection.y, -50.0f, 50.0f);
            ImGui::SliderFloat("DLPosZ", &Render->DirLight.WorldDirection.z, -50.0f, 50.0f);
            ImGui::SliderFloat("DLAmbientIntensity", &Render->DirLight.Base.AmbientIntensity, 0.0f, 1.0f);
            ImGui::SliderFloat("DLDiffuseIntensity", &Render->DirLight.Base.DiffuseIntensity, 0.0f, 1.0f);
            ImGui::ColorEdit3("DLColor", (float *)&Render->DirLight.Base.Color.E);
            ImGui::Spacing();
            ImGui::Text("Point");
            ImGui::SliderFloat("PLPosX", &Render->PointLights[0].WorldPosition.x, -50.0f, 50.0f);
            ImGui::SliderFloat("PLPosY", &Render->PointLights[0].WorldPosition.y, -50.0f, 50.0f);
            ImGui::SliderFloat("PLPosZ", &Render->PointLights[0].WorldPosition.z, -50.0f, 50.0f);
            ImGui::Spacing();
            ImGui::Text("Spot");
            ImGui::SliderFloat("SLPosX", &Render->SpotLights[0].Base.WorldPosition.x, -50.0f, 50.0f);
            ImGui::SliderFloat("SLPosY", &Render->SpotLights[0].Base.WorldPosition.y, -50.0f, 50.0f);
            ImGui::SliderFloat("SLPosZ", &Render->SpotLights[0].Base.WorldPosition.z, -50.0f, 50.0f);
            */

            ImGui::Text("Directional");
            ImGui::InputFloat("DLPosX", &Render->DirLight.WorldDirection.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLPosY", &Render->DirLight.WorldDirection.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLPosZ", &Render->DirLight.WorldDirection.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLAmbientIntensity", &Render->DirLight.Base.AmbientIntensity, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLDiffuseIntensity", &Render->DirLight.Base.DiffuseIntensity, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("DLColor", (float *)&Render->DirLight.Base.Color.E);
            ImGui::Spacing();

            ImGui::Text("Point");
            ImGui::InputFloat("PLPosX", &Render->PointLights[0].WorldPosition.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLPosY", &Render->PointLights[0].WorldPosition.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLPosZ", &Render->PointLights[0].WorldPosition.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAmbientIntensity", &Render->PointLights[0].Base.AmbientIntensity, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLDiffuseIntensity", &Render->PointLights[0].Base.DiffuseIntensity, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLConstant", &Render->PointLights[0].Atten.Constant, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLLinear", &Render->PointLights[0].Atten.Linear, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLExp", &Render->PointLights[0].Atten.Exp, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("PLColor", (float *)&Render->PointLights[0].Base.Color.E);
            ImGui::Spacing();

            ImGui::Text("Spot");
            ImGui::InputFloat("SLPosX", &Render->SpotLights[0].Base.WorldPosition.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SLPosY", &Render->SpotLights[0].Base.WorldPosition.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SLPosZ", &Render->SpotLights[0].Base.WorldPosition.z, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("SLColor", (float *)&Render->SpotLights[0].Base.Base.Color.E);
        }

        ImGui::End();
    }

    //
    // NOTE(me): Game render
    //
    glfwGetFramebufferSize(Window, &Render->DisplayWidth, &Render->DisplayHeight);

#if 0
    s32 DisplayWidth, DisplayHeight;
    glfwGetFramebufferSize(Window, &DisplayWidth, &DisplayHeight);
    glViewport(0, 0, DisplayWidth, DisplayHeight);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // проекция (перспективная)
    r32 AspectRatio = (r32)DisplayWidth / (r32)DisplayHeight;
    r32 FOV = 0.1f; // поле зрения камеры
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-AspectRatio * FOV, AspectRatio * FOV, -FOV, FOV, FOV * 2, 1000);

    // вид с камеры
    OGLSetCameraOnPlayer(Player);

    glDisable(GL_TEXTURE_2D);
    glPushMatrix();
    OGLDrawColoredCube();
    OGLDrawLinesOXYZ(V3(0, 0, 1), 1); // World Start Point OXZY
    glPopMatrix();

    single_mesh *Mesh = &EnvObjects[0]->Model->Meshes[0];
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, Mesh->Positions);
    glTexCoordPointer(2, GL_FLOAT, 0, Mesh->TexCoords);
    glColor3f(0.7f, 0.7f, 0.7f); // понижаем яркость текстуры
    // glColor3f(0.0f, 1.0f, 0.0f); // понижаем яркость текстуры
    glNormalPointer(GL_FLOAT, 0, Mesh->Normals);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture); // TODO(me): поменять?
    glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, Mesh->Indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
#else

    glEnable(GL_CLIP_DISTANCE0);

    // Render Reflection Texture
    glViewport(0, 0, Render->ReflWidth, Render->ReflHeight);
    r32 WaterZ = 0.0f;
    r32 Distance = 2 * (Player->Position.z - WaterZ);
    Player->Position.z -= Distance;
    r32 NormalCameraPitch = Player->CameraPitch;
    Player->CameraPitch = Player->CameraPitchInversed;
    Render->CutPlane = V4(0, 0, 1, -WaterZ);
    glBindFramebuffer(GL_FRAMEBUFFER, Render->WaterReflFBO);
    RenderScene(Window, Render, Render->DefaultShaderProgram, Player, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Player->Position.z += Distance;
    Player->CameraPitch = NormalCameraPitch;

    // Render Refraction Texture
    glViewport(0, 0, (u32)(Render->RefrWidth), (u32)(Render->RefrHeight));
    Render->CutPlane = V4(0, 0, -1, WaterZ);
    glBindFramebuffer(GL_FRAMEBUFFER, Render->WaterRefrFBO);
    RenderScene(Window, Render, Render->DefaultShaderProgram, Player, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_CLIP_DISTANCE0);
    Render->CutPlane = V4(0, 0, -1, 100000);

    // Render Depth Map
    glViewport(0, 0, (u32)(Render->DepthMapWidth), (u32)(Render->DepthMapHeight));
    glBindFramebuffer(GL_FRAMEBUFFER, Render->DepthMapFBO);
    glCullFace(GL_FRONT);
    RenderScene(Window, Render, Render->DepthMapShaderProgram, Player, GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // single_mesh *Mesh = &EnvObjects[0]->Model->Meshes[0];
    //  DrawTexturedSquare(Mesh->Material.Texture);
    glViewport(0, 0, Render->DisplayWidth, Render->DisplayHeight);
    RenderScene(Window, Render, Render->DefaultShaderProgram, Player, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderWater(Window, Render, Player, Input->dtForFrame, WaterZ);
    RenderDebugElements(Render, Player, PlayerClip);
    // DrawTexturedSquare(Window, Render, Render->WaterReflTexture, 320, 180, V2(340, 200));
    // DrawTexturedSquare(Window, Render, Render->WaterRefrTexture, 320, 180, V2(1000, 200));
    // DrawTexturedSquare(Window, Render, Render->DepthMap, 320, 180, V2(1550, 200));

#endif
    // ImGui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(Window);
}