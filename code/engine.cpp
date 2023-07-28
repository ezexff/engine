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

        //
        // NOTE(me): Push game_state structures in memory
        //
        GameState->Render = PushStruct(&GameState->WorldArena, render);
        render *Render = GameState->Render;
        Render->SStMeshesCount = 0;
        Render->SAnMeshesCount = 0;
        Render->MStMeshesCount = 0;
        Render->Animator.Timer = 1.0f;

        GameState->Settings = PushStruct(&GameState->WorldArena, app_settings);
        app_settings *Settings = GameState->Settings;
        Settings->RBFullscreenIsActive = false;
        Settings->RBWindowedIsActive = true;
        Settings->MouseSensitivity = 50.0;
        Settings->NewFrameRate = 60;
        Settings->RBCappedIsActive = true;
        Settings->RBUncappedIsActive = false;
        Settings->RBVSyncIsActive = false;

        GameState->Player = PushStruct(&GameState->WorldArena, entity_player);
        entity_player *Player = GameState->Player;
        Player->Position = V3(-5, 5, 0);
        Player->dP = V2(0, 0);
        Player->CameraXRot = 90.0f;
        Player->CameraZRot = -45.0f;
        Player->CameraYOffset = 0.27f;

        //
        // NOTE(me): Источники света
        //
        // directional light
        Render->DirLight.Base.Color = V3(0.01f, 0.01f, 0.01f);
        Render->DirLight.Base.AmbientIntensity = 0.1f;
        Render->DirLight.Base.DiffuseIntensity = 1.0f;
        Render->DirLight.WorldDirection = V3(1.0f, 0.0f, -1.0f);

        // point lights
        Render->PointLightsCount = 1;
        Render->PointLights = PushArray(&GameState->WorldArena, Render->PointLightsCount, point_light);
        Render->PointLights[0].Base.Color = V3(0.0f, 0.0f, 1.0f);
        Render->PointLights[0].Base.AmbientIntensity = 1.0f;
        Render->PointLights[0].Base.DiffuseIntensity = 4.0f;
        Render->PointLights[0].WorldPosition = V3(3.0f, 3.0f, 1.0f);
        Render->PointLights[0].Atten.Constant = 1.0f;
        Render->PointLights[0].Atten.Linear = 0.1f; // 0.0f
        Render->PointLights[0].Atten.Exp = 0.0f;    // 0.0f
        // имена переменных point lights для оправки в шейдер
        Render->PLVarNames = PushArray(&GameState->WorldArena, Render->PointLightsCount, point_light_var_names);
        for(u32 i = 0; i < Render->PointLightsCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.Color", i);
            Render->PLVarNames[i].VarNames[0] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.AmbientIntensity", i);
            Render->PLVarNames[i].VarNames[1] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.DiffuseIntensity", i);
            Render->PLVarNames[i].VarNames[2] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].LocalPos", i);
            Render->PLVarNames[i].VarNames[3] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Constant", i);
            Render->PLVarNames[i].VarNames[4] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Linear", i);
            Render->PLVarNames[i].VarNames[5] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Exp", i);
            Render->PLVarNames[i].VarNames[6] = PushStringZ(&GameState->WorldArena, TmpName);
        }

        // spot lights
        Render->SpotLightsCount = 1;
        Render->SpotLights = PushArray(&GameState->WorldArena, Render->SpotLightsCount, spot_light);
        // Render->SpotLights[0].Base.Base.Color = V3(253.0f/255.0f, 208.0f/255.0f, 35.0f/255.0f);
        Render->SpotLights[0].Base.Base.Color = V3(1.0f, 0.0f, 0.0f);
        Render->SpotLights[0].Base.Base.AmbientIntensity = 1.0f;
        Render->SpotLights[0].Base.Base.DiffuseIntensity = 1.0f;
        Render->SpotLights[0].Base.WorldPosition = V3(3.0f, 10.0f, 2.0f);
        Render->SpotLights[0].Base.Atten.Constant = 1.0f;
        Render->SpotLights[0].Base.Atten.Linear = 0.01f;
        Render->SpotLights[0].Base.Atten.Exp = 0.0f;
        Render->SpotLights[0].WorldDirection = V3(0.0f, 0.0f, -1.0f);
        Render->SpotLights[0].WorldDirection = Normalize(Render->SpotLights[0].WorldDirection);
        Render->SpotLights[0].Cutoff = 0.9f;
        // имена переменных spot lights для оправки в шейдер
        Render->SLVarNames = PushArray(&GameState->WorldArena, Render->SpotLightsCount, spot_light_var_names);
        for(u32 i = 0; i < Render->SpotLightsCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.Color", i);
            Render->SLVarNames[i].VarNames[0] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
            Render->SLVarNames[i].VarNames[1] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
            Render->SLVarNames[i].VarNames[2] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.LocalPos", i);
            Render->SLVarNames[i].VarNames[3] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Constant", i);
            Render->SLVarNames[i].VarNames[4] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Linear", i);
            Render->SLVarNames[i].VarNames[5] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Exp", i);
            Render->SLVarNames[i].VarNames[6] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Direction", i);
            Render->SLVarNames[i].VarNames[7] = PushStringZ(&GameState->WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Cutoff", i);
            Render->SLVarNames[i].VarNames[8] = PushStringZ(&GameState->WorldArena, TmpName);
        }

        //
        // NOTE(me): Объекты окружения (3d-модели)
        //
        for(u32 i = 0; i < ENV_OBJECTS_MAX; i++)
        {
            GameState->EnvObjects[i] = PushStruct(&GameState->WorldArena, entity_envobject);
        }

        // Террейн
        u32 EnvIndex = 0;
        GameState->EnvObjects[EnvIndex]->Position = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 1.0f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = CreateTerrainModel(&GameState->WorldArena);
        EnvIndex++;

        // маркер позиции точечного источника освещения
        GameState->EnvObjects[EnvIndex]->Position = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 1.0f;
        GameState->EnvObjects[EnvIndex]->Angle = 90.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(1, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        // GameState->EnvObjects[EnvIndex]->Model = LoadModel(&GameState->WorldArena, "assets/test_cube.spm");
        GameState->EnvObjects[EnvIndex]->Model = CreateTexturedSquareModel(&GameState->WorldArena, "lamp.png");
        EnvIndex++;

        // маркер позиции прожектора
        GameState->EnvObjects[EnvIndex]->Position = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 1.0f;
        GameState->EnvObjects[EnvIndex]->Angle = 90.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(1, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = GameState->EnvObjects[1]->Model;
        EnvIndex++;

        // ваза
        GameState->EnvObjects[EnvIndex]->Position = V3(-7.5, 7.5, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 4.0f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = LoadModel(&GameState->WorldArena, "assets/test_vase.spm");
        EnvIndex++;

        // бочка
        GameState->EnvObjects[EnvIndex]->Position = V3(-10, -10, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 2.0f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = LoadModel(&GameState->WorldArena, "assets/test_barrel.spm");
        EnvIndex++;

        // дерево
        GameState->EnvObjects[EnvIndex]->Position = V3(7.5, 7.5, 0.1f);
        GameState->EnvObjects[EnvIndex]->Scale = 0.4f;
        GameState->EnvObjects[EnvIndex]->Angle = 90.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(1, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = LoadModel(&GameState->WorldArena, "assets/test_tree.spm");
        EnvIndex++;

        // ковбой (анимированный)
        GameState->EnvObjects[EnvIndex]->Position = V3(7.5, -7.5, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 0.4f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = LoadModel(&GameState->WorldArena, "assets/test_cowboy.spm");
        EnvIndex++;

        GameState->EnvObjects[EnvIndex]->Position = V3(2, 2 + 3, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 0.4f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = GameState->EnvObjects[6]->Model;
        EnvIndex++;

        GameState->EnvObjects[EnvIndex]->Position = V3(2, 2 + 6, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 0.4f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = GameState->EnvObjects[6]->Model;
        EnvIndex++;

        // страж (анимированный)
        GameState->EnvObjects[EnvIndex]->Position = V3(5, 2, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 0.1f;
        GameState->EnvObjects[EnvIndex]->Angle = 90.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(1, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = LoadModel(&GameState->WorldArena, "assets/test_guard.spm");
        EnvIndex++;

        GameState->EnvObjects[EnvIndex]->Position = V3(5 + 3, 2, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 0.1f;
        GameState->EnvObjects[EnvIndex]->Angle = 90.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(1, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = GameState->EnvObjects[9]->Model;
        EnvIndex++;

        GameState->EnvObjects[EnvIndex]->Position = V3(5 + 6, 2, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 0.1f;
        GameState->EnvObjects[EnvIndex]->Angle = 90.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(1, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = GameState->EnvObjects[9]->Model;
        EnvIndex++;

        // трава
        GameState->EnvObjects[EnvIndex]->Position = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->Scale = 1.0f;
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 2000;
        GameState->EnvObjects[EnvIndex]->InstancingTranslations = CreateInstancingTranslations(
            &GameState->WorldArena, GameState->EnvObjects[0], GameState->EnvObjects[EnvIndex]->Position,
            GameState->EnvObjects[EnvIndex]->InstancingCount);
        GameState->EnvObjects[EnvIndex]->Model = CreateGrassModel(&GameState->WorldArena);
        EnvIndex++;

        // clip wall texture
        GameState->EnvObjects[EnvIndex]->Position = V3(-50.0f, 50.0f, 0.0f);
        GameState->EnvObjects[EnvIndex]->Scale = 98.0f; // diameter
        // GameState->EnvObjects[EnvIndex]->Position = V3(-10.0f, -10.0f, 0.0f);
        // GameState->EnvObjects[EnvIndex]->Scale = 10.0f; // diameter
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = CreateTexturedSquareModel(&GameState->WorldArena, "clip.png");
        EnvIndex++;

        // clip player texture
        // GameState->EnvObjects[EnvIndex]->Position = V3(-10.0f, -10.0f, 0.0f);
        GameState->EnvObjects[EnvIndex]->Position = V3(0.0f, 0.0f, 0.0f);
        GameState->EnvObjects[EnvIndex]->Scale = 0.05f; // diameter
        GameState->EnvObjects[EnvIndex]->Angle = 0.0f;
        GameState->EnvObjects[EnvIndex]->Rotate = V3(0, 0, 0);
        GameState->EnvObjects[EnvIndex]->InstancingCount = 0;
        GameState->EnvObjects[EnvIndex]->Model = CreateTexturedSquareModel(&GameState->WorldArena, "clip.png");
        EnvIndex++;

        // высота объектов окружения на ландшафте
        // for(u32 i = 3; i < ENV_OBJECTS_MAX; i++)
        for(u32 i = 3; i < ENV_OBJECTS_MAX; i++)
        {
            GameState->EnvObjects[i]->Position.z +=
                GameState->EnvObjects[i]->Position.z + TerrainGetHeight(GameState->EnvObjects[0],
                                                                        GameState->EnvObjects[i]->Position.x,
                                                                        GameState->EnvObjects[i]->Position.y);
        }

        // максимальное число строк instancing смещений для отправки в шейдер
        Render->MaxInstancingCount = 0;
        for(u32 i = 3; i < ENV_OBJECTS_MAX; i++)
        {
            if(GameState->EnvObjects[i]->InstancingCount > Render->MaxInstancingCount)
            {
                Render->MaxInstancingCount = GameState->EnvObjects[i]->InstancingCount;
            }
        }

        Render->InstancingVarNames = PushArray(&GameState->WorldArena, Render->MaxInstancingCount, offset_var_name);
        for(u32 i = 0; i < Render->MaxInstancingCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "Offsets[%d]", i);
            Render->InstancingVarNames[i].VarName = PushStringZ(&GameState->WorldArena, TmpName);
        }

        //
        // NOTE(me): Шейдеры и VBO
        //
        Render->Shaders[0] = LoadShader("../code/shaders/AnimatedModel.vert", GL_VERTEX_SHADER);
        Render->Shaders[1] = LoadShader("../code/shaders/AnimatedModel.frag", GL_FRAGMENT_SHADER);
        LinkShaderProgram(Render);

        AddEnvObjectsToRender(Render, GameState->EnvObjects);
        InitVBOs(&GameState->WorldArena, Render);

        // ImGui Demo Window
        GameState->ShowDemoWindow = false;
        GameState->ShowAnotherWindow = false;

        // String example
        // string TestStr = PushString(&GameState->WorldArena, "fdsfsdfss");

        // Assert example
        // int32 xxx = 1;
        // Assert(xxx < 0);

        GameState->IsInitialized = true;
    }

    // Local pointers for game_state sctructs
    render *Render = GameState->Render;
    app_settings *Settings = GameState->Settings;
    entity_player *Player = GameState->Player;

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

        MovePlayerEOM(Player, ddP, PlayerSpeed, Input->dtForFrame);

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
    // Высота на террейне (ландшафте)
    Player->Position.z = TerrainGetHeight(GameState->EnvObjects[0], Player->Position.x, Player->Position.y) + 2.7f;
    // Перемещение кубов-маркеров в положение источников света
    GameState->EnvObjects[1]->Position = Render->PointLights[0].WorldPosition;
    GameState->EnvObjects[2]->Position = Render->SpotLights[0].Base.WorldPosition;
    GameState->EnvObjects[14]->Position = Player->Position;

    // Обработка анимаций
    if(Render->Animator.Timer > 0.0f)
    {
#if 1
        for(u32 i = 0; i < Render->SAnMeshesCount; i++)
        {
            single_mesh *Mesh = Render->SAnMeshes[i];
            GetBoneTransforms(Mesh, //
                              0,    // индекс анимации
                              Render->Animator.Timer);
        }
#endif

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
            ImGui::InputFloat("Player Z", &Player->Position.z, 0.5, 2, "%.10f", 0);
            ImGui::Text("CursorPos=%f,%f", Input->MouseX, Input->MouseY);
            ImGui::Text("dtForFrame=%f", Input->dtForFrame);
            ImGui::Text("MOffset=%f,%f", Input->MouseOffsetX, Input->MouseOffsetY);
            ImGui::Text("SStMeshesCount=%d", Render->SStMeshesCount);
            ImGui::Text("SStVerticesCountSum=%d", Render->SStVerticesCountSum);
            ImGui::Text("SAnMeshesCount=%d", Render->SAnMeshesCount);
            ImGui::Text("SAnVerticesCountSum=%d", Render->SAnVerticesCountSum);
            ImGui::Text("MStMeshesCount=%d", Render->MStMeshesCount);
            ImGui::Text("MStVerticesCountSum=%d", Render->MStVerticesCountSum);
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

    single_mesh *Mesh = &GameState->EnvObjects[0]->Model->Meshes[0];
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
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER, 0.01f);

    glEnable(GL_NORMALIZE);

    RenderVBOs(Window, Render, Player);
#endif
    // ImGui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(Window);
}