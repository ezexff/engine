#include "engine.h"

#include "engine_entity.cpp"

#include "engine_world.cpp"

// TODO(me): для тестов, удалить
#include "simpleproject_opengl.cpp"

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

        GameState->Settings = PushStruct(&GameState->WorldArena, game_settings);
        game_settings *Settings = GameState->Settings;

        GameState->Player = PushStruct(&GameState->WorldArena, entity_player);
        entity_player *Player = GameState->Player;

        //
        // NOTE(me): Init game_state structures
        //

        Settings->MouseSensitivity = 0.05f;

        // Display Mode radio buttons init
        Settings->RBFullscreenIsActive = false;
        Settings->RBWindowedIsActive = false;

        Player->Position = V3(0, 0, 0);
        Player->dP = V2(0, 0);
        Player->CameraXRot = 90.0f;
        Player->CameraZRot = -45.0f;
        Player->CameraYOffset = 0.27f;

        //
        // TODO(me): для тестов, избавиться от всего, что ниже
        //

        // ImGui Demo Window
        GameState->ShowDemoWindow = true;
        GameState->ShowAnotherWindow = false;
        // GameState->ClearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

        // String example
        // string TestStr = PushString(&GameState->WorldArena, "fdsfsdfss");

        // Assert example
        // int32 xxx = 1;
        // Assert(xxx < 0);

        GameState->IsInitialized = true;
    }

    // Local pointers for game_state sctructs
    game_settings *Settings = GameState->Settings;
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
        r32 PlayerSpeed = 10.0f; //+ SpeedBuff; // m/s^2

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
            ImGui::Text("PlayerPos=%f,%f,%f", Player->Position.x, Player->Position.y, Player->Position.z);
            ImGui::Text("CursorPos=%f,%f", Input->MouseX, Input->MouseY);
            ImGui::Text("dtForFrame=%f", Input->dtForFrame);
            ImGui::Text("MOffset=%f,%f", Input->MouseOffsetX, Input->MouseOffsetY);
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

            ImGui::Text("Change Mouse Sensitivity");
            ImGui::SliderFloat("##MouseSensitivity", &Settings->MouseSensitivity, 0.0f, 1.0f);
        }

        ImGui::End();
    }

    //
    // NOTE(me): Game render
    //

    // TODO(me): удалить clearcolor
    s32 DisplayWidth, DisplayHeight;
    glfwGetFramebufferSize(Window, &DisplayWidth, &DisplayHeight);
    glViewport(0, 0, DisplayWidth, DisplayHeight);
    // glClearColor(0, 0, 0, 0);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // проекция (перспективная)
    r32 AspectRatio = (r32)DisplayWidth / (r32)DisplayHeight;
    r32 FOV = 0.1f; // поле зрения камеры
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-AspectRatio * FOV, AspectRatio * FOV, -FOV, FOV, FOV * 2, 1000);
    r32 MatProj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProj);

    // вид с камеры
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
    OGLSetCameraOnPlayer(Player);
    r32 MatView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatView);

    // сохраняем мировые матрицы преобразований для передачи в шейдер
    m4x4 MatProjection;
    m4x4 MatView1;
    for(u32 i = 0; i < 4; i++)
    {
        for(u32 j = 0; j < 4; j++)
        {
            MatProjection.E[i][j] = MatProj[i * 4 + j];
            MatView1.E[i][j] = MatView[i * 4 + j];
        }
    }

    m4x4 WorldMatrix = MatProjection * MatView1;
    // glLoadMatrixf((const GLfloat *)WorldMatrix.E);

    glPushMatrix();
    OGLDrawColoredCube(GameState);
    OGLDrawLinesOXYZ(V3(0, 0, 1), 1); // World Start Point OXZY
    glPopMatrix();

    // ImGui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(Window);
}