// TODO(me): Testing Single VBO for Anim Render (1) vs Multiple VBOs for Anim Render (0)
#define SINGLE_VBO_FOR_ANIM_RENDER 1

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
        else if(EnvObject->InstancingCount > 0)
        {
            loaded_model *Model = EnvObject->Model;
            for(u32 i = 0; i < Model->MeshesCount; i++)
            {
                single_mesh *Mesh = &Model->Meshes[i];
                if(!Mesh->WithAnimations)
                {
                    Render->MStMeshes[Render->MStMeshesCount] = Mesh;
                    Render->MStPositions[Render->MStMeshesCount] = &EnvObject->Position;
                    Render->MStScales[Render->MStMeshesCount] = &EnvObject->Scale;
                    Render->MStRotations[Render->MStMeshesCount] = &EnvObject->Rotate;
                    Render->MStAngles[Render->MStMeshesCount] = &EnvObject->Angle;
                    Render->MStInstancingCounters[Render->MStMeshesCount] = &EnvObject->InstancingCount;
                    // Render->MStInstancingTranslations[Render->MStMeshesCount] = EnvObject->InstancingTranslations;
                    Render->MStInstancingTransformMatrices[Render->MStMeshesCount] =
                        EnvObject->InstancingTransformMatrices;
                    Render->MStMeshesCount++;
                }
                else
                {
                }
                Assert(Render->MStMeshesCount < MULTIPLE_STATIC_MESHES_MAX);
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

void InitVBOs(memory_arena *WorldArena, render *Render)
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
#if SINGLE_VBO_FOR_ANIM_RENDER
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

#else
    // выделение память под VAO, VBO, EBO
    Render->TestSAnVAO = PushArray(WorldArena, Render->SAnMeshesCount, u32);
    Render->TestSAnVBO = PushArray(WorldArena, Render->SAnMeshesCount, u32);
    Render->TestSAnEBO = PushArray(WorldArena, Render->SAnMeshesCount, u32);

    for(u32 i = 0; i < Render->SAnMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SAnMeshes[i];

        // выделение памяти под вершины и индексы
        vertex_animated *SAnVertices = PushArray(WorldArena, Mesh->VerticesCount, vertex_animated);
        u32 *SAnIndices = PushArray(WorldArena, Mesh->IndicesCount, u32);

        // заполнение массива вершин
        u32 BoneIDsAndWeightsCount = 0;
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            SAnVertices[j].Position = Mesh->Positions[j];
            SAnVertices[j].Normal = Mesh->Normals[j];
            SAnVertices[j].TexCoords = Mesh->TexCoords[j];
            for(u32 k = 0; k < 4; k++)
            {
                SAnVertices[j].BoneIDs[k] = Mesh->BoneIDs[BoneIDsAndWeightsCount];
                SAnVertices[j].Weights[k] = Mesh->Weights[BoneIDsAndWeightsCount];
                BoneIDsAndWeightsCount++;
            }
        }
        for(u32 j = 0; j < Mesh->IndicesCount; j++)
        {
            SAnIndices[j] = Mesh->Indices[j];
        }

        glGenVertexArrays(1, &Render->TestSAnVAO[i]);
        glGenBuffers(1, &Render->TestSAnVBO[i]);
        glGenBuffers(1, &Render->TestSAnEBO[i]);

        glBindVertexArray(Render->TestSAnVAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, Render->TestSAnVBO[i]);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_animated) * Mesh->VerticesCount, SAnVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->TestSAnEBO[i]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Mesh->IndicesCount, SAnIndices, GL_STATIC_DRAW);

        // Vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_animated), (void *)0);
        // Vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_animated),
                              (void *)offsetof(vertex_animated, Normal));
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
#endif
    //
    // NOTE(me): Preparing VBO for Multiple Static Meshes
    //
    Render->MStVerticesCountSum = 0;
    Render->MStIndicesCountSum = 0;
    Render->MStInstancesCountSum = 0;

    // получаем общее число вершин и индексов
    for(u32 i = 0; i < Render->MStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->MStMeshes[i];
        Render->MStVerticesCountSum += Mesh->VerticesCount;
        Render->MStIndicesCountSum += Mesh->IndicesCount;
        Render->MStInstancesCountSum += Render->MStInstancingCounters[i][0];
    }

    // выделение памяти под вершины и индексы
    vertex_static *MStVertices = PushArray(WorldArena, Render->MStVerticesCountSum, vertex_static);
    u32 *MStIndices = PushArray(WorldArena, Render->MStIndicesCountSum, u32);
    // v3 *MStInstancingTranslations = PushArray(WorldArena, Render->MStInstancesCountSum, v3);
    m4x4 *MStInstancingTransformMatrices = PushArray(WorldArena, Render->MStInstancesCountSum, m4x4);

    // заполнение массива вершин
    VerticesCountTmp = 0;
    IndicesCountTmp = 0;
    u32 InstancesTmp = 0;
    for(u32 i = 0; i < Render->MStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->MStMeshes[i];
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            MStVertices[VerticesCountTmp].Position = Mesh->Positions[j];
            MStVertices[VerticesCountTmp].Normal = Mesh->Normals[j];
            MStVertices[VerticesCountTmp].TexCoords = Mesh->TexCoords[j];
            VerticesCountTmp++;
        }
        for(u32 j = 0; j < Mesh->IndicesCount; j++)
        {
            MStIndices[IndicesCountTmp] = Mesh->Indices[j];
            IndicesCountTmp++;
        }
        if(Render->MStInstancingCounters[i][0] > 0)
        {
            for(u32 j = 0; j < Render->MStInstancingCounters[i][0]; j++)
            {
                // MStInstancingTranslations[InstancesTmp] = Render->MStInstancingTranslations[i][j];
                MStInstancingTransformMatrices[InstancesTmp] = Render->MStInstancingTransformMatrices[i][j];
                InstancesTmp++;
            }
        }
    }

    // Store instance data in an array buffer
    u32 InstanceVBO;
    glGenBuffers(1, &InstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);
    // TODO(me): сделать не только для нулевого элемента (объединить массивы MStInstancingTranslations в один буфер)
    glBufferData(GL_ARRAY_BUFFER, sizeof(m4x4) * Render->MStInstancesCountSum, MStInstancingTransformMatrices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vertex data and attributes
    glGenVertexArrays(1, &Render->MStVAO);
    glGenBuffers(1, &Render->MStVBO);
    glGenBuffers(1, &Render->MStEBO);

    glBindVertexArray(Render->MStVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Render->MStVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_static) * Render->MStVerticesCountSum, MStVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->MStEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Render->MStIndicesCountSum, MStIndices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)offsetof(vertex_static, Normal));
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)offsetof(vertex_static, TexCoords));
    // Set instance data
    /*
    glEnableVertexAttribArray(5);
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO); // this attribute comes from a different vertex buffer
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(v3), (void *)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribDivisor(5, 1); // tell OpenGL this is an instanced vertex attribute.
    */
    // set attribute pointers for matrix (4 times vec4)
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO); // this attribute comes from a different vertex buffer

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)0);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)(1 * sizeof(v4)));

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)(2 * sizeof(v4)));

    glEnableVertexAttribArray(8);
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)(3 * sizeof(v4)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindVertexArray(0);
}

internal void RenderVBOs(GLFWwindow *Window, render *Render, entity_player *Player)
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

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithOffset"), false);

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
#if SINGLE_VBO_FOR_ANIM_RENDER
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

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithOffset"), false);

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

#else

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

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithOffset"), false);

        // преобразования костей меша во время анимации
        glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "gBones"), //
                           Mesh->BonesCount, GL_TRUE, (const GLfloat *)Mesh->FinalTransforms);

        // отрисовка меша
        glBindVertexArray(Render->TestSAnVAO[i]);
        glDrawElements(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

#endif

    //
    // NOTE(me): Rendering Multiple Single Meshes
    //
    BaseVertex = 0;
    BaseIndex = 0;
    u32 BaseInstance = 0;
    for(u32 i = 0; i < Render->MStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->MStMeshes[i];

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
        // TODO(me): translation*rotation*scale

        //glScalef(Render->MStScales[i][0], Render->MStScales[i][0], Render->MStScales[i][0]);
        /*
        glRotatef(Render->MStAngles[i][0], Render->MStRotations[i]->x, Render->MStRotations[i]->y,
                  Render->MStRotations[i]->z);
        glTranslatef(Render->MStPositions[i]->x, Render->MStPositions[i]->y, Render->MStPositions[i]->z);
        */
        /*m4x4 Test1;
        m4x4 Test2;
        m4x4 Test3;
        for(u32 j = 0; j < 4; j++)
        {
            for(u32 k = 0; k < 4; k++)
            {
                Test1.E[i][j] = 0;
                Test2.E[i][j] = 0;
            }
        }
        Test1.E[0][0] = Render->MStScales[i][0];
        Test1.E[1][1] = Render->MStScales[i][0];
        Test1.E[2][2] = Render->MStScales[i][0];
        Test1.E[3][3] = 1;

        Test2.E[0][0] = 1;
        Test2.E[1][1] = 1;
        Test2.E[2][2] = 1;
        Test2.E[0][3] = Render->MStPositions[i]->x;
        Test2.E[1][3] = Render->MStPositions[i]->y;
        Test2.E[2][3] = Render->MStPositions[i]->z;
        Test2.E[3][3] = 1;

        Test3 = Test1 * Test2;*/

        // glUniformMatrix4fv(glGetUniformLocation(Render->ShaderProgram, "MatModel"), 1, GL_FALSE,
        //                   (const GLfloat *)Test3.E);
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

        glUniform1i(glGetUniformLocation(Render->ShaderProgram, "WithOffset"), true);
        // for(u32 j = 0; j < Render->MStInstancingCounters[i][0]; j++)
        /*for(u32 j = 0; j < 200; j++)
        {
            glUniform3fv(glGetUniformLocation(Render->ShaderProgram, Render->InstancingVarNames[j].VarName), 1, //
                         Render->MStInstancingTranslations[i][j].E);
        }*/

        // отрисовка меша
        glBindVertexArray(Render->MStVAO);
        /*glDrawElementsBaseVertex(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT, (void *)(sizeof(u32) * BaseIndex),
                                 BaseVertex);
                                 */
        glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT,
                                                      (void *)(sizeof(u32) * BaseIndex), //
                                                      // Render->MStInstancesCountSum, // instancing count
                                                      Render->MStInstancingCounters[i][0],
                                                      BaseVertex, //
                                                      BaseInstance);

        // glDrawArraysInstanced(GL_TRIANGLES, 0, Mesh->VerticesCount, 200);
        glBindVertexArray(0);

        // смещение к следующему мешу
        BaseVertex += Mesh->VerticesCount;
        BaseIndex += Mesh->IndicesCount;
        BaseInstance += Render->MStInstancingCounters[i][0];
    }

    glUseProgram(0);
}