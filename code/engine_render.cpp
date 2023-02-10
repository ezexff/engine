internal void AddEnvObjectToRender(render *Render, entity_envobject *EnvObject)
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
                    Render->SAnMeshes[Render->SAnMeshesCount] = Mesh;
                    Render->SAnPositions[Render->SAnMeshesCount] = &EnvObject->Position;
                    Render->SAnScales[Render->SAnMeshesCount] = &EnvObject->Scale;
                    Render->SAnRotations[Render->SAnMeshesCount] = &EnvObject->Rotate;
                    Render->SAnAngles[Render->SAnMeshesCount] = &EnvObject->Angle;
                    Render->SAnInstancingCounters[Render->SAnMeshesCount] = &EnvObject->InstancingCount;
                    Render->SAnMeshesCount++;
                }
                Assert(Render->SStMeshesCount < SINGLE_STATIC_MESHES_MAX);
                Assert(Render->SAnMeshesCount < SINGLE_ANIMATED_MESHES_MAX);
            }
        }
    }
    else
    {
        InvalidCodePath;
    }
}

internal void AddEnvObjectsToRender(render *Render, entity_envobject *EnvObjects[])
{
    for(u32 i = 0; i < ENV_OBJECTS_MAX; i++)
    {
        if(EnvObjects[i]->Model)
        {
            AddEnvObjectToRender(Render, EnvObjects[i]);
        }
        else
        {
            break;
        }
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
    //
    // NOTE(me): Preparing VBO for Single Static Meshes
    //
    Render->SStVerticesCountSum = 0;
    Render->SStIndicesCountSum = 0;

    // получаем общее число вершин и индексов
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];
        Render->SStVerticesCountSum += Mesh->VerticesCount;
        Render->SStIndicesCountSum += Mesh->IndicesCount;
    }

    // выделение памяти под вершины и индексы
    vertex_static *SStVertices = PushArray(WorldArena, Render->SStVerticesCountSum, vertex_static);
    u32 *SStIndices = PushArray(WorldArena, Render->SStIndicesCountSum, u32);

    // заполнение массива вершин
    u32 VerticesCountTmp = 0;
    u32 IndicesCountTmp = 0;
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            SStVertices[VerticesCountTmp].Position = Mesh->Positions[j];
            SStVertices[VerticesCountTmp].Normal = Mesh->Normals[j];
            SStVertices[VerticesCountTmp].TexCoords = Mesh->TexCoords[j];
            VerticesCountTmp++;
        }
        for(u32 j = 0; j < Mesh->IndicesCount; j++)
        {
            SStIndices[IndicesCountTmp] = Mesh->Indices[j];
            IndicesCountTmp++;
        }
    }

    glGenVertexArrays(1, &Render->SStVAO);
    glGenBuffers(1, &Render->SStVBO);
    glGenBuffers(1, &Render->SStEBO);

    glBindVertexArray(Render->SStVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Render->SStVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_static) * Render->SStVerticesCountSum, SStVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->SStEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Render->SStIndicesCountSum, SStIndices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)offsetof(vertex_static, Normal));
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)offsetof(vertex_static, TexCoords));

    glBindVertexArray(0);

    //
    // NOTE(me): Peparing VBO for Single Animated Meshes
    //
    Render->SAnVerticesCountSum = 0;
    Render->SAnIndicesCountSum = 0;

    // получаем общее число вершин и индексов
    for(u32 i = 0; i < Render->SAnMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SAnMeshes[i];
        Render->SAnVerticesCountSum += Mesh->VerticesCount;
        Render->SAnIndicesCountSum += Mesh->IndicesCount;
    }

    // выделение памяти под вершины и индексы
    vertex_animated *SAnVertices = PushArray(WorldArena, Render->SAnVerticesCountSum, vertex_animated);
    u32 *SAnIndices = PushArray(WorldArena, Render->SAnIndicesCountSum, u32);

    // заполнение массива вершин
    VerticesCountTmp = 0;
    IndicesCountTmp = 0;
    for(u32 i = 0; i < Render->SAnMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SAnMeshes[i];
        u32 BoneIDsAndWeightsCount = 0;
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            SAnVertices[VerticesCountTmp].Position = Mesh->Positions[j];
            SAnVertices[VerticesCountTmp].Normal = Mesh->Normals[j];
            SAnVertices[VerticesCountTmp].TexCoords = Mesh->TexCoords[j];
            for(u32 k = 0; k < 4; k++)
            {
                SAnVertices[VerticesCountTmp].BoneIDs[k] = Mesh->BoneIDs[BoneIDsAndWeightsCount];
                SAnVertices[VerticesCountTmp].Weights[k] = Mesh->Weights[BoneIDsAndWeightsCount];
                BoneIDsAndWeightsCount++;
            }
            VerticesCountTmp++;
        }
        for(u32 j = 0; j < Mesh->IndicesCount; j++)
        {
            SAnIndices[IndicesCountTmp] = Mesh->Indices[j];
            IndicesCountTmp++;
        }
    }

    glGenVertexArrays(1, &Render->SAnVAO);
    glGenBuffers(1, &Render->SAnVBO);
    glGenBuffers(1, &Render->SAnEBO);

    glBindVertexArray(Render->SAnVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Render->SAnVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_animated) * Render->SAnVerticesCountSum, SAnVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->SAnEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Render->SAnIndicesCountSum, SAnIndices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_animated), (void *)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_animated), (void *)offsetof(vertex_animated, Normal));
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_animated),
                          (void *)offsetof(vertex_animated, TexCoords));
    // Vertex bones ids
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(vertex_animated), (void *)offsetof(vertex_animated, BoneIDs));
    // Vertex weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_animated),
                          (void *)offsetof(vertex_animated, Weights));

    glBindVertexArray(0);
}

internal void RenderSingleVBO(GLFWwindow *Window, render *Render, entity_player *Player)
{
    //
    // NOTE(me): Rendering Single Static Meshes
    //
    glUseProgram(Render->ShaderProgram);

    // область отображения рендера
    s32 DisplayWidth, DisplayHeight;
    glfwGetFramebufferSize(Window, &DisplayWidth, &DisplayHeight);
    glViewport(0, 0, DisplayWidth, DisplayHeight);
    r32 AspectRatio = (r32)DisplayWidth / (r32)DisplayHeight;
    r32 FOV = 0.1f; // поле зрения камеры

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
    for(u32 i = 0; i < Render->PointLightsCount; i++)
    {
        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[0]), 1, //
                     Render->PointLights[i].Base.Color.E);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[1]), //
                    Render->PointLights[i].Base.AmbientIntensity);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[2]), //
                    Render->PointLights[i].Base.DiffuseIntensity);                                  //

        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[3]), 1, //
                     Render->PointLights[i].WorldPosition.E);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[4]), //
                    Render->PointLights[i].Atten.Constant);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[5]), //
                    Render->PointLights[i].Atten.Linear);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->PLVarNames[i].VarNames[6]), //
                    Render->PointLights[i].Atten.Exp);
    }

    // отправка spot lights в шейдер
    glUniform1i(glGetUniformLocation(Render->ShaderProgram, "gNumSpotLights"), //
                Render->SpotLightsCount);

    for(u32 i = 0; i < Render->PointLightsCount; i++)
    {
        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[0]), 1, //
                     Render->SpotLights[i].Base.Base.Color.E);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[1]), //
                    Render->SpotLights[i].Base.Base.AmbientIntensity);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[2]), //
                    Render->SpotLights[i].Base.Base.DiffuseIntensity);

        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[3]), 1, //
                     Render->SpotLights[i].Base.WorldPosition.E);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[4]), //
                    Render->SpotLights[i].Base.Atten.Constant);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[5]), //
                    Render->SpotLights[i].Base.Atten.Linear);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[6]), //
                    Render->SpotLights[i].Base.Atten.Exp);

        glUniform3fv(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[7]), 1, //
                     Render->SpotLights[i].WorldDirection.E);

        glUniform1f(glGetUniformLocation(Render->ShaderProgram, Render->SLVarNames[i].VarNames[8]), //
                    Render->SpotLights[i].Cutoff);
    }

    u32 BaseVertex = 0;
    u32 BaseIndex = 0;
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];

        // матрица проекции
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

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithAnimations"), false);

        // отрисовка меша
        glBindVertexArray(Render->SStVAO);
        glDrawElementsBaseVertex(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, (void *)(sizeof(u32) * BaseIndex),
                                 BaseVertex);
        glBindVertexArray(0);

        // смещение к следующему мешу
        BaseVertex += Mesh->VerticesCount;
        BaseIndex += Mesh->IndicesCount;
    }

    //
    // NOTE(me): Rendering Single Animated Meshes
    //
    // glfwGetFramebufferSize(Window, &DisplayWidth, &DisplayHeight);
    // glViewport(0, 0, DisplayWidth, DisplayHeight);
    // AspectRatio = (r32)DisplayWidth / (r32)DisplayHeight;
    // FOV = 0.1f; // поле зрения камеры

    BaseVertex = 0;
    BaseIndex = 0;
    // Render->SAnMeshesCount
    for(u32 i = 0; i < Render->SAnMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SAnMeshes[i];

        // матрица проекции
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
        glTranslatef(Render->SAnPositions[i]->x, Render->SAnPositions[i]->y, Render->SAnPositions[i]->z);
        glScalef(Render->SAnScales[i][0], Render->SAnScales[i][0], Render->SAnScales[i][0]);
        glRotatef(Render->SAnAngles[i][0], Render->SAnRotations[i]->x, Render->SAnRotations[i]->y,
                  Render->SAnRotations[i]->z);
        r32 MatModel[16];
        glGetFloatv(GL_MODELVIEW_MATRIX, MatModel);
        glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "MatModel"), 1, GL_FALSE, MatModel);

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
        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithAnimations"), true);

        // преобразования костей меша во время анимации
        glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "gBones"), //
                           Mesh->BonesCount, GL_TRUE, (const GLfloat *)Mesh->FinalTransforms);

        // отрисовка меша
        glBindVertexArray(Render->SAnVAO);
        glDrawElementsBaseVertex(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, (void *)(sizeof(u32) * BaseIndex),
                                 BaseVertex);
        glBindVertexArray(0);

        // смещение к следующему мешу
        BaseVertex += Mesh->VerticesCount;
        BaseIndex += Mesh->IndicesCount;
    }
    glUseProgram(0);
}