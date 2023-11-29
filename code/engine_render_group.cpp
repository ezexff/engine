internal render_group *                                                //
AllocateRenderGroup(memory_arena *Arena, u32 MaxPushBufferSize,        //
                    r32 CameraPitch, r32 CameraYaw, r32 CameraRenderZ, //
                    b32 IsOrthogonal = false)
{
    render_group *Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (uint8 *)PushSize(Arena, MaxPushBufferSize);

    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize = 0;

    Result->Transform.IsOrthogonal = IsOrthogonal;

    Result->Transform.CameraPitch = CameraPitch;
    Result->Transform.CameraYaw = CameraYaw;
    Result->Transform.CameraRenderZ = CameraRenderZ;

    Result->Transform.OffsetP = V3(0, 0, 0);

    return (Result);
}

#define PushRenderElement(Group, type) (type *)PushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)
inline void * //
PushRenderElement_(render_group *Group, uint32 Size, render_group_entry_type Type)
{
    void *Result = 0;

    Size += sizeof(render_group_entry_header);

    if((Group->PushBufferSize + Size) < Group->MaxPushBufferSize)
    {
        render_group_entry_header *Header =
            (render_group_entry_header *)(Group->PushBufferBase + Group->PushBufferSize);
        Header->Type = Type;
        Result = (uint8 *)Header + sizeof(*Header);
        Group->PushBufferSize += Size;
    }
    else
    {
        InvalidCodePath;
    }

    return (Result);
}

inline void //
Clear(render_group *Group, v4 Color)
{
    render_entry_clear *Entry = PushRenderElement(Group, render_entry_clear);
    if(Entry)
    {
        Entry->Color = Color;
    }
}

inline void //
PushRect(render_group *Group, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    P += Group->Transform.OffsetP;

    render_entry_rectangle *Entry = PushRenderElement(Group, render_entry_rectangle);
    Entry->Color = Color;
    Entry->P = P.xy;
    Entry->Dim = Dim;
}

internal void //
PushRectCollisionVolumes(render_group *RenderGroup, sim_entity *Entity, v4 Color = V4(1, 1, 1, 1))
{
    for(u32 VolumeIndex = 0;                          //
        VolumeIndex < Entity->Collision->VolumeCount; //
        ++VolumeIndex)
    {
        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
        PushRect(RenderGroup, V3(0, 0, 0), Volume->Dim.xy, Color);
    }
}

internal void //
glDrawRect(rectangle2 R, r32 Z, v3 Color)
{
    r32 VRectangle[] = {
        // ground
        R.Min.x, R.Min.y, Z, // 0
        R.Max.x, R.Min.y, Z, // 1
        R.Max.x, R.Max.y, Z, // 2
        R.Min.x, R.Max.y, Z, // 3
    };

    r32 VertColors[] = {
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
        Color.x, Color.y, Color.z, // 2
        Color.x, Color.y, Color.z, // 3
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

inline void //
PushRectOutline(render_group *Group, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1), r32 LineWidth = 1)
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    P += Group->Transform.OffsetP;

    render_entry_rectangle_outline *Entry = PushRenderElement(Group, render_entry_rectangle_outline);
    Entry->Color = Color;
    Entry->P = P.xy;
    Entry->Dim = Dim;
    Entry->LineWidth = LineWidth;
}

internal void                                                                  //
PushRectOutlineCollisionVolumes(render_group *RenderGroup, sim_entity *Entity, //
                                v4 Color = V4(1, 1, 1, 1), r32 LineWidth = 1)
{
    for(u32 VolumeIndex = 0;                          //
        VolumeIndex < Entity->Collision->VolumeCount; //
        ++VolumeIndex)
    {
        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
        PushRectOutline(RenderGroup, V3(0, 0, 0), Volume->Dim.xy, Color, LineWidth);
    }
}

internal void //
glDrawRectOutline(rectangle2 R, r32 Z, v3 Color, r32 LineWidth)
{
    r32 VRectangle[] = {
        // ground
        R.Min.x, R.Min.y, Z, // 0
        R.Max.x, R.Min.y, Z, // 1
        R.Max.x, R.Max.y, Z, // 2
        R.Min.x, R.Max.y, Z, // 3
    };

    r32 VertColors[] = {
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
        Color.x, Color.y, Color.z, // 2
        Color.x, Color.y, Color.z, // 3
    };

    glLineWidth(LineWidth);
    glEnable(GL_LINE_SMOOTH);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glLineWidth(1);
    glDisable(GL_LINE_SMOOTH);
}

inline void //
PushTexture(render_group *Group, v3 Offset, v2 Dim, u32 Texture, b32 FlipVertically = false, r32 Repeat = 1.0f)
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    P += Group->Transform.OffsetP;

    render_entry_texture *Entry = PushRenderElement(Group, render_entry_texture);
    Entry->Texture = Texture;
    Entry->P = P.xy;
    Entry->Dim = Dim;
    Entry->FlipVertically = FlipVertically;
    Entry->Repeat = Repeat;
}

internal void //
glDrawTexture(rectangle2 R, r32 Z, u32 Texture, b32 FlipVertically, r32 Repeat)
{
    r32 VRect[] = {
        // ground
        R.Min.x, R.Min.y, Z, // 0
        R.Max.x, R.Min.y, Z, // 1
        R.Max.x, R.Max.y, Z, // 2
        R.Min.x, R.Max.y, Z, // 3
    };

    r32 UVRect[] = {
        0,      0,      // 0
        Repeat, 0,      // 1
        Repeat, Repeat, // 2
        0,      Repeat  // 3
    };

    r32 UVRectFlippedVertically[] = {
        0,      Repeat, // 0
        Repeat, Repeat, // 1
        Repeat, 0,      // 2
        0,      0       // 3
    };

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, Texture);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, VRect);
    if(FlipVertically)
    {
        glTexCoordPointer(2, GL_FLOAT, 0, UVRectFlippedVertically);
    }
    else
    {
        glTexCoordPointer(2, GL_FLOAT, 0, UVRect);
    }
    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);
}

inline void //
PushModel(render_group *Group, v3 Offset, v2 Dim, loaded_model *Model, b32 IsFill = true)
{
    v3 P = (Offset - V3(0.5f * Dim, 0));
    P += Group->Transform.OffsetP;

    render_entry_model *Entry = PushRenderElement(Group, render_entry_model);
    Entry->Model = Model;
    Entry->P = P.xy;
    Entry->Dim = Dim;
    Entry->IsFill = IsFill;
}

internal void //
glDrawModel(rectangle2 R, r32 Z, loaded_model *Model, b32 IsFill)
{
    // Log.AddLog("glDrawModel\n");

    glTranslatef(R.Min.x, R.Min.y, 0);

    if(!IsFill)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    single_mesh *Mesh = &Model->Meshes[0];

    if(Mesh->Material.WithTexture)
    {
        glEnable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture.ID);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    // glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, Mesh->Positions);
    glTexCoordPointer(2, GL_FLOAT, 0, Mesh->TexCoords);
    // glColorPointer(3, GL_FLOAT, 0, VertColors);

    glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, Mesh->Indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);

    glTranslatef(-R.Min.x, -R.Min.y, 0);
    // glDisableClientState(GL_COLOR_ARRAY);

    /*glPolygonMode(GL_FRONT_AND_BACK, IsFill ? GL_FILL : GL_LINE);

    // single_mesh *Mesh = &Model->Meshes[0];
    for(u32 MeshIndex = 0;              //
        MeshIndex < Model->MeshesCount; //
        MeshIndex++)
    {
        single_mesh *Mesh = Model->Meshes + MeshIndex;

        glEnableClientState(GL_VERTEX_ARRAY);
        // glEnableClientState(GL_COLOR_ARRAY);

        glVertexPointer(3, GL_FLOAT, 0, Mesh->Positions);
        // glColorPointer(3, GL_FLOAT, 0, VertColors);

        glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, Mesh->Indices);

        glDisableClientState(GL_VERTEX_ARRAY);
        // glDisableClientState(GL_COLOR_ARRAY);
    }

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);*/
}

internal void                                                                //
RenderGroupToOutput(render_group *RenderGroup, loaded_texture *OutputTarget) //, loaded_bitmap *OutputTarget)
{
    BEGIN_TIMED_BLOCK(RenderGroupToOutput);

    /*v2 ScreenCenter = {0.5f * (r32)OutputTarget->Width, //
                       0.5f * (r32)OutputTarget->Height};*/

    glBindFramebuffer(GL_FRAMEBUFFER, OutputTarget->FBO);

    // Create Render Texture Attachment
    /*glBindTexture(GL_TEXTURE_2D, Buffer->RenderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Buffer->Width, Buffer->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Buffer->RenderTexture, 0);

    // Create Depth Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Buffer->DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Buffer->Width, Buffer->Height, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Buffer->DepthTexture, 0);*/

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(0, 0, OutputTarget->Width, OutputTarget->Height);
    if(RenderGroup->Transform.IsOrthogonal)
    {
        glDisable(GL_DEPTH_TEST);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, OutputTarget->Width, 0, OutputTarget->Height, 0, 1);
        glMatrixMode(GL_MODELVIEW);
    }
    else
    {
        r32 AspectRatio = (r32)OutputTarget->Width / (r32)OutputTarget->Height;
        r32 FOV = 0.1f;
        glEnable(GL_DEPTH_TEST);
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-AspectRatio * FOV, AspectRatio * FOV, //
                  -FOV, FOV, FOV * 2, 1000);

        // set camera
        glRotatef(-RenderGroup->Transform.CameraPitch, 1.0f, 0.0f, 0.0f);
        glRotatef(-RenderGroup->Transform.CameraYaw, 0.0f, 0.0f, 1.0f);
        // TODO(me): think about real z for camera
        v2 WorldOrigin1 = {};
        glTranslatef(-WorldOrigin1.x, -WorldOrigin1.y, -RenderGroup->Transform.CameraRenderZ);
    }

    for(u32 BaseAddress = 0;                       //
        BaseAddress < RenderGroup->PushBufferSize; //
    )
    {
        render_group_entry_header *Header = (render_group_entry_header *) //
            (RenderGroup->PushBufferBase + BaseAddress);
        BaseAddress += sizeof(*Header);

        void *Data = (uint8 *)Header + sizeof(*Header);
        switch(Header->Type)
        {
        case RenderGroupEntryType_render_entry_clear: //
        {
            render_entry_clear *Entry = (render_entry_clear *)Data;

            glClearColor(Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            BaseAddress += sizeof(*Entry);
        }
        break;

        case RenderGroupEntryType_render_entry_model: //
        {
            render_entry_model *Entry = (render_entry_model *)Data;
            Assert(Entry->Model);
            glDrawModel({Entry->P, Entry->P + Entry->Dim}, 0.0f, Entry->Model, Entry->IsFill);

            BaseAddress += sizeof(*Entry);
        }
        break;

        case RenderGroupEntryType_render_entry_texture: //
        {
            render_entry_texture *Entry = (render_entry_texture *)Data;
            glDrawTexture({Entry->P, Entry->P + Entry->Dim}, 0.0f, Entry->Texture, Entry->FlipVertically,
                          Entry->Repeat);

            BaseAddress += sizeof(*Entry);
        }
        break;

        case RenderGroupEntryType_render_entry_rectangle: //
        {
            render_entry_rectangle *Entry = (render_entry_rectangle *)Data;
            glDrawRect({Entry->P, Entry->P + Entry->Dim}, 0.0f, V3(Entry->Color.x, Entry->Color.y, Entry->Color.z));

            BaseAddress += sizeof(*Entry);
        }
        break;

        case RenderGroupEntryType_render_entry_rectangle_outline: //
        {
            render_entry_rectangle_outline *Entry = (render_entry_rectangle_outline *)Data;
            glDrawRectOutline({Entry->P, Entry->P + Entry->Dim}, 0.0f,            //
                              V3(Entry->Color.x, Entry->Color.y, Entry->Color.z), //
                              Entry->LineWidth);

            BaseAddress += sizeof(*Entry);
        }
        break;

            /*case RenderGroupEntryType_render_entry_coordinate_system: {
                render_entry_coordinate_system *Entry = (render_entry_coordinate_system *)Data;

                v2 vMax = (Entry->Origin + Entry->XAxis + Entry->YAxis);
                DrawRectangleSlowly(OutputTarget, Entry->Origin, Entry->XAxis, Entry->YAxis, Entry->Color,
            Entry->Texture, Entry->NormalMap, Entry->Top, Entry->Middle, Entry->Bottom, 1.0f /
            RenderGroup->MetersToPixels);

                v4 Color = {1, 1, 0, 1};
                v2 Dim = {2, 2};
                v2 P = Entry->Origin;
                DrawRectangle(OutputTarget, P - Dim, P + Dim, Color);

                P = Entry->Origin + Entry->XAxis;
                DrawRectangle(OutputTarget, P - Dim, P + Dim, Color);

                P = Entry->Origin + Entry->YAxis;
                DrawRectangle(OutputTarget, P - Dim, P + Dim, Color);

                DrawRectangle(OutputTarget, vMax - Dim, vMax + Dim, Color);

                BaseAddress += sizeof(*Entry);
            }
            break;
            */

            InvalidDefaultCase;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    END_TIMED_BLOCK(RenderGroupToOutput);
}

internal void                                                  //
RenderImGui(game_input *Input,                                 //
            game_state *GameState, transient_state *TranState, //
            game_offscreen_buffer *Buffer,                     //
            sim_region *SimRegion)
{
    game_debug *Debug = GameState->Debug;
    app_settings *Settings = GameState->Settings;
    render *Render = GameState->Render;

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
        ImGui::Text("CursorPos=%f,%f", Input->MouseX, Input->MouseY);
        ImGui::Text("dtForFrame=%f", Input->dtForFrame);
        ImGui::Text("MouseOffset=%f,%f", Input->MouseOffsetX, Input->MouseOffsetY);
        ImGui::Text("Camera Chunk V3(%d,%d,%d)", //
                    GameState->CameraP.ChunkX,   //
                    GameState->CameraP.ChunkY,   //
                    GameState->CameraP.ChunkZ);
        ImGui::Text("Camera Offset V3(%f,%f,%f)", //
                    GameState->CameraP.Offset_.x, //
                    GameState->CameraP.Offset_.y, //
                    GameState->CameraP.Offset_.z);
        ImGui::Text("Camera Pitch & Yaw (%f,%f)", //
                    GameState->CameraPitch,       //
                    GameState->CameraYaw);
        ImGui::Spacing();

        ImGui::SetNextItemOpen(true);
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
                Platform.ToggleFullscreen();
            }
            ImGui::Spacing();

            ImGui::Text("Change Game Update Hz");
            ImGui::SameLine();
            if(ImGui::RadioButton("Capped", Settings->RBCappedIsActive))
            {
                Settings->RBCappedIsActive = true;
                Settings->RBVSyncIsActive = false;
                Platform.ToggleVSync(false, Settings->NewGameUpdateHz);
            }
            ImGui::SameLine();

            if(ImGui::RadioButton("VSync", Settings->RBVSyncIsActive))
            {
                Settings->RBCappedIsActive = false;
                Settings->RBVSyncIsActive = true;
                Platform.ToggleVSync(true, Settings->NewGameUpdateHz);
            }

            ImGui::Spacing();

            // TODO(me): vsync/unlimited fps/custom fps???
            if(Settings->RBCappedIsActive)
            {
                ImGui::Text("Set FPS");
                s32 NewGameUpdateHz = (s32)Settings->NewGameUpdateHz;
                ImGui::SliderInt("##FPS", &NewGameUpdateHz, 15, 2048);
                Settings->NewGameUpdateHz = (r32)NewGameUpdateHz;
                Platform.ToggleVSync(false, Settings->NewGameUpdateHz);
            }
            ImGui::Spacing();

            ImGui::Text("Change Mouse Sensitivity");
            ImGui::SliderFloat("##MouseSensitivity", &Settings->MouseSensitivity, 0.0f, 100.0f);
            ImGui::Spacing();

            ImGui::Checkbox("Process Animations", &Debug->ProcessAnimations);

            ImGui::Checkbox("Log Cycle Counters", &Debug->LogCycleCounters);
        }

        if(ImGui::CollapsingHeader("Audio"))
        {
            ImGui::Text("1st line (previous frames):");
            ImGui::Text("White - PlayCursor");
            ImGui::Text("Red - WriteCursor");
            ImGui::Text("Purple - FlipPlayCursor+480*BytesPerSample");
            ImGui::Text("2nd line (prepare audio for writing):");
            ImGui::Text("White - PlayCursor");
            ImGui::Text("Red - WriteCursor");
            ImGui::Text("3rd line:");
            ImGui::Text("White - ByteToLock");
            ImGui::Text("Red - BTL + BytesToWrite");
            ImGui::Text("4th line (current frame):");
            ImGui::Text("White - PlayCursor");
            ImGui::Text("Red - WriteCursor");
            ImGui::Text("Yellow - ExpectedFlipPlayCursor");
            ImGui::Spacing();
        }

        if(ImGui::CollapsingHeader("Memory"))
        {
            ImGui::Text("PermanentStorage");
            ImGui::Text("   Size %d(KB), %d(MB)", //
                        GameState->WorldArena.Size / 1024, GameState->WorldArena.Size / 1024 / 1024);
            ImGui::Text("   Used %d(KB), %d(MB)", //
                        GameState->WorldArena.Used / 1024, GameState->WorldArena.Used / 1024 / 1024);

            ImGui::Text("TransientStorage");
            ImGui::Text("   TempCount=%d", TranState->TranArena.TempCount);
            ImGui::Text("   Size %d(KB), %d(MB)", //
                        TranState->TranArena.Size / 1024, TranState->TranArena.Size / 1024 / 1024);
            ImGui::Text("   Used %d(KB), %d(MB)", //
                        TranState->TranArena.Used / 1024, TranState->TranArena.Used / 1024 / 1024);
        }

        if(ImGui::CollapsingHeader("World"))
        {
            // ImGui::Text("ChunkDimInMeters=%d", World->ChunkDimInMeters);
            // ImGui::Text("ChunksCount=%d", World->ChunksCount);
            ImGui::Checkbox("DrawSimRegionBounds", &Debug->DrawSimRegionBounds);
            ImGui::Checkbox("DrawSimRegionUpdatableBounds", &Debug->DrawSimRegionUpdatableBounds);
            ImGui::Checkbox("DrawSimChunks", &Debug->DrawSimChunks);
            ImGui::Checkbox("DrawChunkWhereCamera", &Debug->DrawChunkWhereCamera);
            ImGui::Checkbox("DrawPlayerHitbox", &Debug->DrawPlayerHitbox);
            /*world_position PlayerP;
            PlayerP.ChunkX = FloorReal32ToInt32(Player->Position.x / World->ChunkDimInMeters);
            PlayerP.ChunkY = FloorReal32ToInt32(Player->Position.y / World->ChunkDimInMeters);
            ImGui::Text("ChunkX=%d", PlayerP.ChunkX);
            ImGui::Text("ChunkY=%d", PlayerP.ChunkY);
            PlayerP.Offset_.x = Player->Position.x                         //
                                - PlayerP.ChunkX * World->ChunkDimInMeters // chunk in meters
                                - World->ChunkDimInMeters * 0.5f;          // to center
            PlayerP.Offset_.y = Player->Position.y                         //
                                - PlayerP.ChunkY * World->ChunkDimInMeters // chunk in meters
                                - World->ChunkDimInMeters * 0.5f;          // to center
            ImGui::Text("Offset_.x=%f", PlayerP.Offset_.x);
            ImGui::Text("Offset_.y=%f", PlayerP.Offset_.y);

            world_position TestWallP;
            TestWallP.ChunkX = -1;
            TestWallP.ChunkY = -1;
            TestWallP.Offset_ = V2(0, 0);
            v2 CameraSP = GetCameraSpaceP(World, &PlayerP, &TestWallP);

            ImGui::Text("TestWallP");
            ImGui::Text("CameraSPX=%f", CameraSP.x);
            ImGui::Text("CameraSPY=%f", CameraSP.y);*/
        }

        if(ImGui::CollapsingHeader("Camera"))
        {
            ImGui::Text("CameraP");
            ImGui::InputInt("ChunkX", &GameState->CameraP.ChunkX, 1, 2, 0);
            ImGui::InputInt("ChunkX", &GameState->CameraP.ChunkY, 1, 2, 0);
            ImGui::InputInt("ChunkX", &GameState->CameraP.ChunkZ, 1, 2, 0);
            ImGui::InputFloat("OffsetX", &GameState->CameraP.Offset_.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("OffsetY", &GameState->CameraP.Offset_.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("OffsetZ", &GameState->CameraP.Offset_.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("Pitch", &GameState->CameraPitch, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("Yaw", &GameState->CameraYaw, 0.5, 2, "%.10f", 0);
        }

        if(ImGui::CollapsingHeader("Entities"))
        {
            ImGui::Text("SimEntities");
            sim_entity *Entity = SimRegion->Entities;
            for(u32 EntityIndex = 0;                  //
                EntityIndex < SimRegion->EntityCount; //
                ++EntityIndex, ++Entity)
            {
                if(Entity->Updatable)
                {
                    char *EntityType = "EntityType";
                    if(Entity->Type == EntityType_Null)
                    {
                        EntityType = "Null";
                    }
                    else if(Entity->Type == EntityType_Space)
                    {
                        EntityType = "Space";
                    }
                    else if(Entity->Type == EntityType_Hero)
                    {
                        EntityType = "Hero";
                    }
                    else if(Entity->Type == EntityType_Wall)
                    {
                        EntityType = "Wall";
                    }
                    else if(Entity->Type == EntityType_Familiar)
                    {
                        EntityType = "Familiar";
                    }
                    else if(Entity->Type == EntityType_Monstar)
                    {
                        EntityType = "Monstar";
                    }
                    else if(Entity->Type == EntityType_Sword)
                    {
                        EntityType = "Sword";
                    }
                    else if(Entity->Type == EntityType_Stairwell)
                    {
                        EntityType = "Stairwell";
                    }

                    // if(ImGui::TreeNode("SimRegion->Entities"))
                    if(ImGui::TreeNode((void *)(intptr_t)EntityIndex, "Entity%d(%s)", EntityIndex, EntityType))
                    {
                        ImGui::Text("Type=%s", EntityType);
                        ImGui::Text("P=(%f,%f,%f)", Entity->P.x, Entity->P.y, Entity->P.z);
                        ImGui::Text("dP=(%f,%f,%f)", Entity->dP.x, Entity->dP.y, Entity->dP.z);
                        ImGui::Text("DistanceLimit=%f", Entity->DistanceLimit);
                        ImGui::Text("HitPointMax=%u", Entity->HitPointMax);
                        ImGui::TreePop();
                    }
                }
            }
            /*
            ImGui::Text("EnvObjectsCount=%d", Render->EnvObjectsCount);
            if(ImGui::TreeNode("EnvObjects"))
            {
                for(u32 i = 0; i < Render->EnvObjectsCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)EnvObjects[i]->Model->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("GrassObjectsCount=%d", Render->GrassObjectsCount);
            if(ImGui::TreeNode("GrassObjects"))
            {
                for(u32 i = 0; i < Render->GrassObjectsCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)GrassObjects[i]->Model->Name.Data);
                }
                ImGui::TreePop();
            }*/
        }

        if(ImGui::CollapsingHeader("Render"))
        {
            ImGui::InputFloat("CameraRenderZ", &GameState->CameraRenderZ, 0.5, 2, "%.10f", 0);

            ImGui::Text("Test Render Texture");
            ImGui::Image((void *)(intptr_t)Buffer->DrawTexture, //
                         ImVec2((r32)Buffer->Width / 4, (r32)Buffer->Height / 4), ImVec2(0, 0), ImVec2(1, -1));

            ImGui::Text("Test Depth Texture");
            ImGui::Image((void *)(intptr_t)Buffer->DepthTexture, //
                         ImVec2((r32)Buffer->Width / 4, (r32)Buffer->Height / 4), ImVec2(0, 0), ImVec2(1, -1));

            ImGui::InputInt("GroundBufferIndex", &Debug->GroundBufferIndex, 1, 1, 0);
            ground_buffer *GroundBuffer = TranState->GroundBuffers + Debug->GroundBufferIndex;
            ImGui::Text("Test Ground Texture");
            /*ImGui::Image((void *)(intptr_t)GroundBuffer->DrawBuffer.Texture,                                //
                         ImVec2((r32)GroundBuffer->DrawBuffer.Width, (r32)GroundBuffer->DrawBuffer.Height), //
                         ImVec2(0, 0), ImVec2(1, -1));*/
            ImGui::Image((void *)(intptr_t)GroundBuffer->DrawBuffer.ID,                                     //
                         ImVec2((r32)GroundBuffer->DrawBuffer.Width, (r32)GroundBuffer->DrawBuffer.Height), //
                         ImVec2(0, 1), ImVec2(1, 0));

            ImGui::Text("Environment Objects Rendering System");
            ImGui::Text("SStMeshesCount=%d", Render->SStMeshesCount);
            if(ImGui::TreeNode("SStMeshes"))
            {
                for(u32 i = 0; i < Render->SStMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->SStMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("SStVerticesCountSum=%d", Render->SStVerticesCountSum);
            ImGui::Text("SAnMeshesCount=%d", Render->SAnMeshesCount);
            if(ImGui::TreeNode("SAnMeshes"))
            {
                for(u32 i = 0; i < Render->SAnMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->SAnMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("SAnVerticesCountSum=%d", Render->SAnVerticesCountSum);
            ImGui::Text("MStMeshesCount=%d", Render->MStMeshesCount);
            if(ImGui::TreeNode("MStMeshes"))
            {
                for(u32 i = 0; i < Render->MStMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->MStMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("MStVerticesCountSum=%d", Render->MStVerticesCountSum);
            ImGui::Text("MStInstancesCountSum=%d", Render->MStInstancesCountSum);
            ImGui::Spacing();
            ImGui::Text("Grass Objects Rendering System");
            ImGui::Text("GrMeshesCount=%d", Render->GrMeshesCount);
            if(ImGui::TreeNode("GrMeshes"))
            {
                for(u32 i = 0; i < Render->GrMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->GrMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("GrVerticesCountSum=%d", Render->GrVerticesCountSum);
            ImGui::Text("GrInstancesCountSum=%d", Render->GrInstancesCountSum);
        }

        if(ImGui::CollapsingHeader("Light Sources"))
        {
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
            ImGui::InputFloat("PLAttConstant", &Render->PointLights[0].Atten.Constant, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAttLinear", &Render->PointLights[0].Atten.Linear, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAttExp", &Render->PointLights[0].Atten.Exp, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("PLColor", (float *)&Render->PointLights[0].Base.Color.E);
            ImGui::Spacing();

            ImGui::Text("Spot");
            ImGui::InputFloat("SLPosX", &Render->SpotLights[0].Base.WorldPosition.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SLPosY", &Render->SpotLights[0].Base.WorldPosition.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SLPosZ", &Render->SpotLights[0].Base.WorldPosition.z, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("SLColor", (float *)&Render->SpotLights[0].Base.Base.Color.E);
        }

        if(ImGui::CollapsingHeader("Shadows"))
        {
            ImGui::InputFloat("ShadowMapSize", &Render->ShadowMapSize, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("NearPlane", &Render->NearPlane, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("FarPlane", &Render->FarPlane, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowCameraPitch", &Render->ShadowCameraPitch, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowCameraYaw", &Render->ShadowCameraYaw, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SunPosX", &Render->SunPos.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SunPosY", &Render->SunPos.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SunPosZ", &Render->SunPos.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("Bias", &Render->Bias, 0.001, 2, "%.10f", 0);
            ImGui::Text("ShadowMap");
            ImGui::Image((void *)(intptr_t)Render->DepthMap, ImVec2(1920 / 4, 1080 / 4), ImVec2(0, 0), ImVec2(1, -1));
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

        ImGui::End();
    }

    // NOTE(me): Draw Log App
    if(Input->ShowMouseCursorMode)
    {
        Log.Draw("Log");

        if(Debug->LogCycleCounters)
        {
            LogCycleCounters(DebugGlobalMemory);
        }
    }

    // ImGui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}