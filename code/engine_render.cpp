internal void AddEntityToRender(render *Render, entity_envobject *EnvObject)
{
    if(EnvObject->Model)
    {
        if(EnvObject->InstancingCount == 0)
        {
            loaded_model *Model = EnvObject->Model;
            for(u32 i = 0; i < Model->MeshesCount; i++)
            {
                single_mesh *Mesh = &Model->Meshes[i];
                if(!Mesh->WithAnimations)
                {
                    Render->SStMeshes[Render->SStMeshesCount] = Mesh;
                    Render->SStPositions[Render->SStMeshesCount] = &EnvObject->Position;
                    Render->SStScales[Render->SStMeshesCount] = &EnvObject->Scale;
                    Render->SStRotations[Render->SStMeshesCount] = &EnvObject->Rotate;
                    Render->SStAngles[Render->SStMeshesCount] = &EnvObject->Angle;
                    Render->SStInstancingCounters[Render->SStMeshesCount] = &EnvObject->InstancingCount;
                    Render->SStMeshesCount++;
                }
                else
                {
                }
                Assert(Render->SStMeshesCount < 256);
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
    Render->SStVerticesCountSum = 0;
    // Render->AnimatedVerticesCountSum = 0;

    // получаем общее число вершин и индексов
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];
        Render->SStVerticesCountSum += Mesh->VerticesCount;
        Render->SstVerticesCount[i] += Mesh->VerticesCount;
    }

    // выделение памяти с данными о мешах для отправки в VBO
    v3 *SStPositions = PushArray(WorldArena, Render->SStVerticesCountSum, v3);
    v2 *SStTexCoords = PushArray(WorldArena, Render->SStVerticesCountSum, v2);
    v3 *SStNormals = PushArray(WorldArena, Render->SStVerticesCountSum, v3);
    u32 *SStIndices = PushArray(WorldArena, Render->SStVerticesCountSum, u32);

    // Render->AnimatedVerticesCountSum = Render->AnimatedVerticesCountSum * 4; // по 4 веса и идентификатора на вершину
    // u32 *BoneIDs = PushArray(WorldArena, Render->AnimatedVerticesCountSum, u32);
    // r32 *Weights = PushArray(WorldArena, Render->AnimatedVerticesCountSum, r32);

    u32 VerticesCountTmp = 0;
    u32 IndicesOffset = 0;
    // u32 BoneIDsAndWeightsCountTmp = 0;
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            SStPositions[VerticesCountTmp] = Mesh->Positions[j];
            SStTexCoords[VerticesCountTmp] = Mesh->TexCoords[j];
            SStNormals[VerticesCountTmp] = Mesh->Normals[j];

            u32 Index = Mesh->Indices[j];
            u32 IndexTmp = IndicesOffset + j;
            SStIndices[IndexTmp] = Index + IndicesOffset;

            VerticesCountTmp++;
        }
        /*u32 BoneIDsAndWeightsCount = Mesh->VerticesCount * 4;
        for(u32 j = 0; j < BoneIDsAndWeightsCount; j++)
        {
            BoneIDs[BoneIDsAndWeightsCountTmp] = Mesh->BoneIDs[j];
            Weights[BoneIDsAndWeightsCountTmp] = Mesh->Weights[j];
            BoneIDsAndWeightsCountTmp++;
        }*/
        IndicesOffset += Mesh->VerticesCount; // смещение числа вершин после добавления меша
    }

    // создаем объект VAO
    glGenVertexArrays(1, &Render->SStVAO);
    glBindVertexArray(0);

    // создаем массивы VBO для параметров вершин
    glGenBuffers(1, &Render->SStPosVBO);
    glGenBuffers(1, &Render->SStTexCoordsVBO);
    glGenBuffers(1, &Render->SStNormalsVBO);
    glGenBuffers(1, &Render->SStIndicesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, Render->SStPosVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->SStVerticesCountSum * 3, //
                 SStPositions, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Render->SStTexCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->SStVerticesCountSum * 2, //
                 SStTexCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, Render->SStNormalsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->SStVerticesCountSum * 3, //
                 SStNormals, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->SStIndicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Render->SStVerticesCountSum, //
                 SStIndices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind VBO

    // если есть анимированные меши в списке
    /*if(Render->AnimatedVerticesCountSum > 0)
    {
        glGenBuffers(1, &Render->SingleBoneIDsVBO);
        glGenBuffers(1, &Render->SingleWeightsVBO);

        glBindBuffer(GL_ARRAY_BUFFER, Render->SingleBoneIDsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(u32) * Render->AnimatedVerticesCountSum, //
                     BoneIDs, GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, Render->SingleWeightsVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(r32) * Render->AnimatedVerticesCountSum, //
                     Weights, GL_STATIC_DRAW);
    }*/
}

internal void RenderSingleVBO(GLFWwindow *Window, render *Render, entity_player *Player)
{
    // начинаем использовать шейдерную программу
    glUseProgram(Render->ShaderProgram);

    u32 CurrentVerticesCount = 0;
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];

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
        glTranslatef(Render->SStPositions[i]->x, Render->SStPositions[i]->y, Render->SStPositions[i]->z);
        glScalef(Render->SStScales[i][0], Render->SStScales[i][0], Render->SStScales[i][0]);
        glRotatef(Render->SStAngles[i][0], Render->SStRotations[i]->x, Render->SStRotations[i]->y,
                  Render->SStRotations[i]->z);
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

        // отправка point lights в шейдер
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

        // отправка информации о наличии анимаций у меша в шейдер
        /*glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithAnimations"), Mesh->WithAnimations);

        // преобразования костей меша во время анимации
        if(Mesh->WithAnimations)
        {
            glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "gBones"), //
                               Mesh->BonesCount, GL_TRUE, (const GLfloat *)Mesh->FinalTransforms);
        }*/

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithAnimations"), false);

        // Связывание VBO позиции
        glBindBuffer(GL_ARRAY_BUFFER, Render->SStPosVBO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); // позиция

        // Связывание VBO текстурных координат
        glBindBuffer(GL_ARRAY_BUFFER, Render->SStTexCoordsVBO);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0); // текстурные координаты

        // Связывание VBO нормалей
        glBindBuffer(GL_ARRAY_BUFFER, Render->SStNormalsVBO);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0); // нормали

        /*if(Mesh->WithAnimations)
        {
            // Связывание VBO ID костей
            glBindBuffer(GL_ARRAY_BUFFER, Render->SingleBoneIDsVBO);
            glEnableVertexAttribArray(5);
            glVertexAttribIPointer(5, 4, GL_INT, 0, 0);
            // glVertexAttribPointer(3, 4, GL_UNSIGNED_INT, GL_FALSE, 0, 0);

            // Связывание VBO весов
            glBindBuffer(GL_ARRAY_BUFFER, Render->SingleWeightsVBO);
            glEnableVertexAttribArray(6);
            glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 0, 0);
        }*/
        // Нарисовать модель
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->SStIndicesVBO);
        glDrawArrays(GL_TRIANGLES, CurrentVerticesCount, Render->SstVerticesCount[i]);
        CurrentVerticesCount += Render->SstVerticesCount[i];
        // glDrawElements(GL_TRIANGLES, Render->VerticesCount, GL_UNSIGNED_INT, 0);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        /*if(Mesh->WithAnimations)
        {
            glDisableVertexAttribArray(5);
            glDisableVertexAttribArray(6);
        }*/
    }

    glUseProgram(0);
}