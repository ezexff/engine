internal void AddEntityToRender(render *Render, entity_envobject *EnvObject)
{
    if(EnvObject->Model)
    {
        if(EnvObject->InstancingCount == 0)
        {
            loaded_model *Model = EnvObject->Model;
            for(u32 i = 0; i < Model->MeshesCount; i++)
            {
                Render->SingleMeshes[Render->SingleMeshCount] = &Model->Meshes[i];
                Render->SinglePositions[Render->SingleMeshCount] = &EnvObject->Position;
                Render->SingleScales[Render->SingleMeshCount] = &EnvObject->Scale;
                Render->SingleRotations[Render->SingleMeshCount] = &EnvObject->Rotate;
                Render->SingleAngles[Render->SingleMeshCount] = &EnvObject->Angle;

                Render->SingleInstancingCounters[Render->SingleMeshCount] = &EnvObject->InstancingCount;
                Render->SingleMeshCount++;
                Assert(Render->SingleMeshCount < 256);
            }
        }
    }
    else
    {
        InvalidCodePath;
    }
}

GLuint LoadShader(char *Path, GLuint Type)
{
    FILE *FileHandle;
    char *Contents;
    u32 FileSize32;

    FileHandle = fopen(Path, "r");
    if(FileHandle == NULL)
    {
        InvalidCodePath;
    }

    fseek(FileHandle, 0L, SEEK_END);
    FileSize32 = ftell(FileHandle);
    fseek(FileHandle, 0L, SEEK_SET);

    Contents = (char *)calloc(FileSize32, sizeof(char));
    if(Contents == NULL)
    {
        InvalidCodePath;
    }

    fread(Contents, sizeof(char), FileSize32, FileHandle);
    fclose(FileHandle);

    GLuint Shader = glCreateShader(Type);
    glShaderSource(Shader, 1, &(char *)Contents, NULL);
    glCompileShader(Shader);
    free(Contents);

    // проверка компиляции шейдера
    GLint Ok;
    GLchar Log[2000];
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Ok);
    if(!Ok)
    {
        glGetShaderInfoLog(Shader, 2000, NULL, Log);
        _snprintf_s(Log, sizeof(Log), "%s\n", Log);
        OutputDebugStringA(Log);
        InvalidCodePath;
    }

    return (Shader);
}

void LinkShaderProgram(render *Render)
{
    // создание шейдерной программы и привязка шейдеров
    Render->ShaderProgram = glCreateProgram();
    for(u32 i = 0; i < ArrayCount(Render->Shaders); i++)
    {
        glAttachShader(Render->ShaderProgram, Render->Shaders[i]);
    }
    glLinkProgram(Render->ShaderProgram);

    // линковка шейдеров
    GLint Ok;
    GLchar Log[2000];
    glGetProgramiv(Render->ShaderProgram, GL_LINK_STATUS, &Ok);
    if(!Ok)
    {
        glGetProgramInfoLog(Render->ShaderProgram, 2000, NULL, Log);
        _snprintf_s(Log, sizeof(Log), "%s\n", Log);
        OutputDebugStringA(Log);
        InvalidCodePath;
    }
}

void InitSingleVBO(memory_arena *WorldArena, render *Render)
{
    Render->SingleVerticesCountSum = 0;

    // получаем общее число вершин и индексов
    for(u32 i = 0; i < Render->SingleMeshCount; i++)
    {
        single_mesh *Mesh = Render->SingleMeshes[i];

        Render->SingleVerticesCountSum += Mesh->VerticesCount;
        Render->SingleVerticesCount[i] += Mesh->VerticesCount;
    }

    // выделение памяти с данными о мешах для отправки в VBO
    v3 *Positions = PushArray(WorldArena, Render->SingleVerticesCountSum, v3);
    v2 *TexCoords = PushArray(WorldArena, Render->SingleVerticesCountSum, v2);
    v3 *Normals = PushArray(WorldArena, Render->SingleVerticesCountSum, v3);
    u32 *Indices = PushArray(WorldArena, Render->SingleVerticesCountSum, u32);

    u32 VerticesCount1 = 0;

    u32 IndicesOffset = 0;
    for(u32 i = 0; i < Render->SingleMeshCount; i++)
    {
        single_mesh *Mesh = Render->SingleMeshes[i];
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            Positions[VerticesCount1] = Mesh->Positions[j];
            TexCoords[VerticesCount1] = Mesh->TexCoords[j];
            Normals[VerticesCount1] = Mesh->Normals[j];

            u32 Index = Mesh->Indices[j];
            u32 IndexTmp = IndicesOffset + j;
            Indices[IndexTmp] = Index + IndicesOffset;
            // Indices[VerticesCount1] = Model->Meshes[j].Indices[k];

            VerticesCount1++;
        }
        IndicesOffset += Mesh->VerticesCount; // смещение числа вершин после добавления меша
        /*loaded_model *Model = Render->SingleModel[i];
        u32 MeshesCount = Model->MeshesCount;

        for(u32 j = 0; j < Model->MeshesCount; j++)
        {
            u32 VerticesCountTmp = Model->Meshes[j].VerticesCount;
            for(u32 k = 0; k < VerticesCountTmp; k++)
            {
                Positions[VerticesCount1] = Model->Meshes[j].Positions[k];
                TexCoords[VerticesCount1] = Model->Meshes[j].TexCoords[k];
                Normals[VerticesCount1] = Model->Meshes[j].Normals[k];

                u32 Index = Model->Meshes[j].Indices[k];
                u32 IndexTmp = IndicesOffset + k;
                Indices[IndexTmp] = Index + IndicesOffset;
                // Indices[VerticesCount1] = Model->Meshes[j].Indices[k];

                VerticesCount1++;
            }
            IndicesOffset += Model->Meshes[j].VerticesCount; // смещение числа вершин после добавления меша
        }*/
    }

    // Render->VerticesCount = VerticesCount1;

    // создаем объект VAO
    glGenVertexArrays(1, &Render->SingleVAO);
    glBindVertexArray(0);

    // создаем массивы VBO для параметров вершин
    glGenBuffers(1, &Render->SinglePosVBO);
    glGenBuffers(1, &Render->SingleTexCoordsVBO);
    glGenBuffers(1, &Render->SingleNormalsVBO);
    glGenBuffers(1, &Render->SingleIndicesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, Render->SinglePosVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->SingleVerticesCountSum * 3, //
                 Positions, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Render->SingleTexCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->SingleVerticesCountSum * 2, //
                 TexCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Render->SingleNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->SingleVerticesCountSum * 3, //
                 Normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->SingleIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Render->SingleVerticesCountSum, //
                 Indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind VBO
}

internal void RenderSingleVBO(GLFWwindow *Window, render *Render, entity_player *Player)
{
    // начинаем использовать шейдерную программу
    glUseProgram(Render->ShaderProgram);

    u32 CurrentVerticesCount = 0;
    for(u32 i = 0; i < Render->SingleMeshCount; i++)
    {
        // матрица проекции
        s32 DisplayWidth, DisplayHeight;
        glfwGetFramebufferSize(Window, &DisplayWidth, &DisplayHeight);
        glViewport(0, 0, DisplayWidth, DisplayHeight);
        r32 AspectRatio = (r32)DisplayWidth / (r32)DisplayHeight;
        r32 FOV = 0.1f; // поле зрения камеры
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glFrustum(-AspectRatio * FOV, AspectRatio * FOV, -FOV, FOV, FOV * 2, 1000);
        r32 MatProj[16];
        glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
        glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "MatProj"), 1, GL_FALSE, MatProj);

        // матрица вида с камеры
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        OGLSetCameraOnPlayer(Player);
        r32 MatView[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
        glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "MatView"), 1, GL_FALSE, MatView);

        // матрица модели в мировой системе координат
        glLoadIdentity();
        glTranslatef(Render->SinglePositions[i]->x, Render->SinglePositions[i]->y, Render->SinglePositions[i]->z);
        glScalef(Render->SingleScales[i][0], Render->SingleScales[i][0], Render->SingleScales[i][0]);
        glRotatef(Render->SingleAngles[i][0], Render->SingleRotations[i]->x, Render->SingleRotations[i]->y,
                  Render->SingleRotations[i]->z);
        r32 MatModel[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatModel);
        glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "MatModel"), 1, GL_FALSE, MatModel);

        // отправка позиции камеры (игрока) в шейдер
        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, "gCameraWorldPos"), 1, Player->Position.E);

        // отправка directional light в шейдер
        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, "gDirectionalLight.Base.Color"), 1, //
                     Render->DirLight.Base.Color.E);
        glUniform1f(glGetUniformLocation(Render->ShaderProgram, "gDirectionalLight.Base.AmbientIntensity"), //
                    Render->DirLight.Base.AmbientIntensity);
        glUniform1f(glGetUniformLocation(Render->ShaderProgram, "gDirectionalLight.Base.DiffuseIntensity"), //
                    Render->DirLight.Base.DiffuseIntensity);
        v3 LocalDirection = Render->DirLight.WorldDirection;
        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, "gDirectionalLight.Direction"), 1, //
                     LocalDirection.E);

        // передача point lights в шейдер
        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gNumPointLights"), //
                    Render->PointLightsCount);
        for(u32 j = 0; j < Render->PointLightsCount; j++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.Color", j);
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, TmpName), 1, //
                         Render->PointLights[0].Base.Color.E);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.AmbientIntensity", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->PointLights[0].Base.AmbientIntensity);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.DiffuseIntensity", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->PointLights[0].Base.DiffuseIntensity);        //

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].LocalPos", j);
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, TmpName), 1, //
                         Render->PointLights[0].WorldPosition.E);
            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Constant", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->PointLights[0].Atten.Constant);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Linear", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->PointLights[0].Atten.Linear);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Exp", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->PointLights[0].Atten.Exp);
        }

        // отправка spot lights в шейдер
        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gNumSpotLights"), //
                    Render->SpotLightsCount);

        for(u32 j = 0; j < Render->PointLightsCount; j++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.Color", j);
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, TmpName), 1, //
                         Render->SpotLights[0].Base.Base.Color.E);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.AmbientIntensity", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->SpotLights[0].Base.Base.AmbientIntensity);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.DiffuseIntensity", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->SpotLights[0].Base.Base.DiffuseIntensity);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.LocalPos", j);
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, TmpName), 1, //
                         Render->SpotLights[0].Base.WorldPosition.E);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Constant", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->SpotLights[0].Base.Atten.Constant);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Linear", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->SpotLights[0].Base.Atten.Linear);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Exp", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->SpotLights[0].Base.Atten.Exp);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Direction", j);
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, TmpName), 1, //
                         Render->SpotLights[0].WorldDirection.E);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Cutoff", j);
            glUniform1f(glGetUniformLocation(Render->ShaderProgram, TmpName), //
                        Render->SpotLights[0].Cutoff);
        }

        // отправка материала в шейдер
        single_mesh *Mesh = Render->SingleMeshes[i];
        if(Mesh->WithMaterial)
        {
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, "gMaterial.AmbientColor"), 1, //
                         Mesh->Material.Ambient.E);

            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, "gMaterial.DiffuseColor"), 1, //
                         Mesh->Material.Diffuse.E);

            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, "gMaterial.SpecularColor"), 1, //
                         Mesh->Material.Specular.E);

            if(Mesh->Material.WithTexture)
            {
                glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gWithTexture"), true);
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
                glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gSampler"), 0);
                glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gSamplerSpecularExponent"), 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gWithTexture"), false);
            }
        }
        else
        {
            InvalidCodePath;
        }

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithAnimations"), false);

        // Связывание VBO позиции
        glBindBuffer(GL_ARRAY_BUFFER, Render->SinglePosVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // позиция

        // Связывание VBO текстурных координат
        glBindBuffer(GL_ARRAY_BUFFER, Render->SingleTexCoordsVBO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // текстурные координаты

        // Связывание VBO нормалей
        glBindBuffer(GL_ARRAY_BUFFER, Render->SingleNormalsVBO);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0); // нормали

        // Нарисовать модель
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->SingleIndicesVBO);
        glDrawArrays(GL_TRIANGLES, CurrentVerticesCount, Render->SingleVerticesCount[i]);
        CurrentVerticesCount += Render->SingleVerticesCount[i];
        // glDrawElements(GL_TRIANGLES, Render->VerticesCount, GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
    }

    glUseProgram(0);
}