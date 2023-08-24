// TODO(me): Testing Single VBO for Anim Render (1) vs Multiple VBOs for Anim Render (0)
#define SINGLE_VBO_FOR_ANIM_RENDER 1

//
// NOTE(me): Shaders
//
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
    GLchar LogInfo[2000];
    glGetShaderiv(Shader, GL_COMPILE_STATUS, &Ok);
    if(!Ok)
    {
        glGetShaderInfoLog(Shader, 2000, NULL, LogInfo);
        _snprintf_s(LogInfo, sizeof(LogInfo), "%s\n", LogInfo);
        OutputDebugStringA(LogInfo);
        InvalidCodePath;
    }

    return (Shader);
}

u32 LinkShaderProgram(u32 ShaderVert, u32 ShaderFrag)
{
    // создание шейдерной программы, привязка шейдеров, линковка
    u32 ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, ShaderVert);
    glAttachShader(ShaderProgram, ShaderFrag);
    glLinkProgram(ShaderProgram);

    // ошибка линковки шейдера
    GLint Ok;
    GLchar LogInfo[2000];
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Ok);
    if(!Ok)
    {
        glGetProgramInfoLog(ShaderProgram, 2000, NULL, LogInfo);
        _snprintf_s(LogInfo, sizeof(LogInfo), "%s\n", LogInfo);
        OutputDebugStringA(LogInfo);
        InvalidCodePath;
    }

    return (ShaderProgram);
}

internal void LightSourcesToShader(render *Render, u32 ShaderProg)
{
    // отправка directional light в шейдер
    glUniform3fv(glGetUniformLocation(ShaderProg, "gDirectionalLight.Base.Color"), 1, //
                 Render->DirLight.Base.Color.E);
    glUniform1f(glGetUniformLocation(ShaderProg, "gDirectionalLight.Base.AmbientIntensity"), //
                Render->DirLight.Base.AmbientIntensity);
    glUniform1f(glGetUniformLocation(ShaderProg, "gDirectionalLight.Base.DiffuseIntensity"), //
                Render->DirLight.Base.DiffuseIntensity);
    v3 LocalDirection = Render->DirLight.WorldDirection;
    glUniform3fv(glGetUniformLocation(ShaderProg, "gDirectionalLight.Direction"), 1, //
                 LocalDirection.E);

    // отправка point lights в шейдер
    glUniform1i(glGetUniformLocation(ShaderProg, "gNumPointLights"), //
                Render->PointLightsCount);
    for(u32 i = 0; i < Render->PointLightsCount; i++)
    {
        glUniform3fv(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[0]), 1, //
                     Render->PointLights[i].Base.Color.E);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[1]), //
                    Render->PointLights[i].Base.AmbientIntensity);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[2]), //
                    Render->PointLights[i].Base.DiffuseIntensity);                       //

        glUniform3fv(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[3]), 1, //
                     Render->PointLights[i].WorldPosition.E);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[4]), //
                    Render->PointLights[i].Atten.Constant);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[5]), //
                    Render->PointLights[i].Atten.Linear);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->PLVarNames[i].VarNames[6]), //
                    Render->PointLights[i].Atten.Exp);
    }

    // отправка spot lights в шейдер
    glUniform1i(glGetUniformLocation(ShaderProg, "gNumSpotLights"), //
                Render->SpotLightsCount);
    for(u32 i = 0; i < Render->PointLightsCount; i++)
    {
        glUniform3fv(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[0]), 1, //
                     Render->SpotLights[i].Base.Base.Color.E);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[1]), //
                    Render->SpotLights[i].Base.Base.AmbientIntensity);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[2]), //
                    Render->SpotLights[i].Base.Base.DiffuseIntensity);

        glUniform3fv(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[3]), 1, //
                     Render->SpotLights[i].Base.WorldPosition.E);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[4]), //
                    Render->SpotLights[i].Base.Atten.Constant);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[5]), //
                    Render->SpotLights[i].Base.Atten.Linear);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[6]), //
                    Render->SpotLights[i].Base.Atten.Exp);

        glUniform3fv(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[7]), 1, //
                     Render->SpotLights[i].WorldDirection.E);

        glUniform1f(glGetUniformLocation(ShaderProg, Render->SLVarNames[i].VarNames[8]), //
                    Render->SpotLights[i].Cutoff);
    }
}

//
// NOTE(me): Environment Objects Rendering System
//
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
                    Render->SStTransformMatrices[Render->SStMeshesCount] = &EnvObject->TransformMatrix;
                    Render->SStMeshesCount++;
                }
                else
                {
                    Render->SAnMeshes[Render->SAnMeshesCount] = Mesh;
                    Render->SAnTransformMatrices[Render->SAnMeshesCount] = &EnvObject->TransformMatrix;
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
                    Render->MStInstancingCounters[Render->MStMeshesCount] = &EnvObject->InstancingCount;
                    Render->MStInstancingTransformMatrices[Render->MStMeshesCount] =
                        EnvObject->InstancingTransformMatrices;
                    Render->MStMeshesCount++;
                }
                else
                {
                    InvalidCodePath;
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
    for(u32 i = 0; i < Render->EnvObjectsCount; i++)
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

void InitEnvVBOs(memory_arena *WorldArena, render *Render)
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
                MStInstancingTransformMatrices[InstancesTmp] = Render->MStInstancingTransformMatrices[i][j];
                InstancesTmp++;
            }
        }
    }

    // Store instance data in an array buffer
    u32 InstanceVBO;
    glGenBuffers(1, &InstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);

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

    // set instance attribute pointers for matrix (4 times vec4)
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

internal void RenderEnvVBOs(render *Render, u32 ShaderProg, entity_player *Player)
{
    //
    // NOTE(me): Preparing shader to render
    //
    glUseProgram(ShaderProg);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, Render->DepthMap);
    glUniform1i(glGetUniformLocation(ShaderProg, "ShadowMap"), 1);

    // матрица проекции (источник света для теней)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-Render->ShadowMapSize, Render->ShadowMapSize, -Render->ShadowMapSize, Render->ShadowMapSize,
            Render->NearPlane, Render->FarPlane);
    r32 MatProjShadows[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProjShadows);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatProjShadows"), 1, GL_FALSE, MatProjShadows);

    // матрица вида (источник света для теней)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-Render->ShadowLightPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-Render->ShadowLightYaw, 0.0f, 0.0f, 1.0f);
    glTranslatef(-Render->ShadowLightPos.x, -Render->ShadowLightPos.y, -Render->ShadowLightPos.z);
    r32 MatViewShadows[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatViewShadows);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatViewShadows"), 1, GL_FALSE, MatViewShadows);

    // матрица проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Render->AspectRatio * Render->FOV, Render->AspectRatio * Render->FOV, //
              -Render->FOV, Render->FOV, Render->FOV * 2, 1000);
    r32 MatProj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatProj"), 1, GL_FALSE, MatProj);

    // матрица вида с камеры
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    OGLSetCameraOnPlayer(Player);
    r32 MatView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatView"), 1, GL_FALSE, MatView);

    // отправка плоскости отсечения в шейдер
    glUniform4fv(glGetUniformLocation(ShaderProg, "CutPlane"), 1, Render->CutPlane.E);

    // отправка позиции камеры (игрока) в шейдер
    glUniform3fv(glGetUniformLocation(ShaderProg, "gCameraWorldPos"), 1, Player->Position.E);

    LightSourcesToShader(Render, ShaderProg);

    //
    // NOTE(me): Rendering Single Static Meshes
    //
    u32 BaseVertex = 0;
    u32 BaseIndex = 0;
    for(u32 i = 0; i < Render->SStMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SStMeshes[i];

        // матрица модели в мировой системе координат
        glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatModel"), 1, GL_FALSE,
                           (const GLfloat *)Render->SStTransformMatrices[i]);

        // отправка материала в шейдер
        if(Mesh->WithMaterial)
        {
            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.AmbientColor"), 1, //
                         Mesh->Material.Ambient.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.DiffuseColor"), 1, //
                         Mesh->Material.Diffuse.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.SpecularColor"), 1, //
                         Mesh->Material.Specular.E);

            if(Mesh->Material.WithTexture)
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), true);
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSampler"), 0);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSamplerSpecularExponent"), 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), false);
            }
        }
        else
        {
            InvalidCodePath;
        }

        glUniform1i(glGetUniformLocation(ShaderProg, "WithAnimations"), false);

        glUniform1i(glGetUniformLocation(ShaderProg, "WithOffset"), false);

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
#if SINGLE_VBO_FOR_ANIM_RENDER
    BaseVertex = 0;
    BaseIndex = 0;
    for(u32 i = 0; i < Render->SAnMeshesCount; i++)
    {
        single_mesh *Mesh = Render->SAnMeshes[i];

        // матрица модели в мировой системе координат
        glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatModel"), 1, GL_FALSE,
                           (const GLfloat *)Render->SAnTransformMatrices[i]);

        // отправка материала в шейдер
        if(Mesh->WithMaterial)
        {
            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.AmbientColor"), 1, //
                         Mesh->Material.Ambient.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.DiffuseColor"), 1, //
                         Mesh->Material.Diffuse.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.SpecularColor"), 1, //
                         Mesh->Material.Specular.E);

            if(Mesh->Material.WithTexture)
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), true);
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSampler"), 0);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSamplerSpecularExponent"), 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), false);
            }
        }
        else
        {
            InvalidCodePath;
        }

        // отправка информации о наличии анимаций у меша в шейдер
        glUniform1i(glGetUniformLocation(ShaderProg, "WithAnimations"), true);

        glUniform1i(glGetUniformLocation(ShaderProg, "WithOffset"), false);

        // преобразования костей меша во время анимации
        glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "gBones"), //
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

        // матрица модели в мировой системе координат
        glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatModel"), 1, GL_FALSE,
                           (const GLfloat *)Render->SAnTransformMatrices[i]);

        // отправка материала в шейдер
        if(Mesh->WithMaterial)
        {
            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.AmbientColor"), 1, //
                         Mesh->Material.Ambient.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.DiffuseColor"), 1, //
                         Mesh->Material.Diffuse.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.SpecularColor"), 1, //
                         Mesh->Material.Specular.E);

            if(Mesh->Material.WithTexture)
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), true);
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSampler"), 0);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSamplerSpecularExponent"), 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), false);
            }
        }
        else
        {
            InvalidCodePath;
        }

        // отправка информации о наличии анимаций у меша в шейдер
        glUniform1i(glGetUniformLocation(ShaderProg, "WithAnimations"), true);

        glUniform1i(glGetUniformLocation(ShaderProg, "WithOffset"), false);

        // преобразования костей меша во время анимации
        glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "gBones"), //
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

        // отправка материала в шейдер
        if(Mesh->WithMaterial)
        {
            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.AmbientColor"), 1, //
                         Mesh->Material.Ambient.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.DiffuseColor"), 1, //
                         Mesh->Material.Diffuse.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.SpecularColor"), 1, //
                         Mesh->Material.Specular.E);

            if(Mesh->Material.WithTexture)
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), true);
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSampler"), 0);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSamplerSpecularExponent"), 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), false);
            }
        }
        else
        {
            InvalidCodePath;
        }

        glUniform1i(glGetUniformLocation(ShaderProg, "WithAnimations"), false);
        glUniform1i(glGetUniformLocation(ShaderProg, "WithOffset"), true);

        // отрисовка меша
        glBindVertexArray(Render->MStVAO);
        glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT,
                                                      (void *)(sizeof(u32) * BaseIndex), //
                                                      Render->MStInstancingCounters[i][0],
                                                      BaseVertex, //
                                                      BaseInstance);
        glBindVertexArray(0);

        // смещение к следующему мешу
        BaseVertex += Mesh->VerticesCount;
        BaseIndex += Mesh->IndicesCount;
        BaseInstance += Render->MStInstancingCounters[i][0];
    }

    glUseProgram(0);
}

//
// NOTE(me): Grass Objects Rendering System
//
internal void AddGrassObjectToRender(render *Render, entity_grassobject *GrassObject)
{
    if(GrassObject->Model)
    {
        if(GrassObject->InstancingCount > 0)
        {
            loaded_model *Model = GrassObject->Model;
            for(u32 i = 0; i < Model->MeshesCount; i++)
            {
                single_mesh *Mesh = &Model->Meshes[i];
                if(!Mesh->WithAnimations)
                {
                    Render->GrMeshes[Render->GrMeshesCount] = Mesh;
                    Render->GrInstancingCounters[Render->GrMeshesCount] = &GrassObject->InstancingCount;
                    Render->GrInstancingTransformMatrices[Render->GrMeshesCount] =
                        GrassObject->InstancingTransformMatrices;
                    Render->GrMeshesCount++;
                }
                else
                {
                    InvalidCodePath;
                }
                Assert(Render->GrMeshesCount < GRASS_MESHES_MAX);
            }
        }
        else
        {
            InvalidCodePath;
        }
    }
    else
    {
        InvalidCodePath;
    }
}

internal void AddGrassObjectsToRender(render *Render, entity_grassobject *GrassObjects[])
{
    for(u32 i = 0; i < Render->GrassObjectsCount; i++)
    {
        if(GrassObjects[i]->Model)
        {
            AddGrassObjectToRender(Render, GrassObjects[i]);
        }
        else
        {
            break;
        }
    }
}

void InitGrassVBO(memory_arena *WorldArena, render *Render)
{
    //
    // NOTE(me): Preparing VBO for Grass Meshes
    //
    Render->GrVerticesCountSum = 0;
    Render->GrIndicesCountSum = 0;
    Render->GrInstancesCountSum = 0;

    // получаем общее число вершин и индексов
    for(u32 i = 0; i < Render->GrMeshesCount; i++)
    {
        single_mesh *Mesh = Render->GrMeshes[i];
        Render->GrVerticesCountSum += Mesh->VerticesCount;
        Render->GrIndicesCountSum += Mesh->IndicesCount;
        Render->GrInstancesCountSum += Render->GrInstancingCounters[i][0];
    }

    // выделение памяти под вершины и индексы
    vertex_static *GrVertices = PushArray(WorldArena, Render->GrVerticesCountSum, vertex_static);
    u32 *GrIndices = PushArray(WorldArena, Render->GrIndicesCountSum, u32);
    m4x4 *GrInstancingTransformMatrices = PushArray(WorldArena, Render->GrInstancesCountSum, m4x4);

    // заполнение массива вершин
    u32 VerticesCountTmp = 0;
    u32 IndicesCountTmp = 0;
    u32 InstancesTmp = 0;
    for(u32 i = 0; i < Render->GrMeshesCount; i++)
    {
        single_mesh *Mesh = Render->GrMeshes[i];
        for(u32 j = 0; j < Mesh->VerticesCount; j++)
        {
            GrVertices[VerticesCountTmp].Position = Mesh->Positions[j];
            GrVertices[VerticesCountTmp].Normal = Mesh->Normals[j];
            GrVertices[VerticesCountTmp].TexCoords = Mesh->TexCoords[j];
            VerticesCountTmp++;
        }
        for(u32 j = 0; j < Mesh->IndicesCount; j++)
        {
            GrIndices[IndicesCountTmp] = Mesh->Indices[j];
            IndicesCountTmp++;
        }
        if(Render->GrInstancingCounters[i][0] > 0)
        {
            for(u32 j = 0; j < Render->GrInstancingCounters[i][0]; j++)
            {
                GrInstancingTransformMatrices[InstancesTmp] = Render->GrInstancingTransformMatrices[i][j];
                InstancesTmp++;
            }
        }
    }

    // Store instance data in an array buffer
    u32 InstanceVBO;
    glGenBuffers(1, &InstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(m4x4) * Render->GrInstancesCountSum, GrInstancingTransformMatrices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Vertex data and attributes
    glGenVertexArrays(1, &Render->GrVAO);
    glGenBuffers(1, &Render->GrVBO);
    glGenBuffers(1, &Render->GrEBO);

    glBindVertexArray(Render->GrVAO);
    glBindBuffer(GL_ARRAY_BUFFER, Render->GrVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_static) * Render->GrVerticesCountSum, GrVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render->GrEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * Render->GrIndicesCountSum, GrIndices, GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)0);
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)offsetof(vertex_static, Normal));
    // Vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_static), (void *)offsetof(vertex_static, TexCoords));

    // set instance attribute pointers for matrix (4 times vec4)
    glBindBuffer(GL_ARRAY_BUFFER, InstanceVBO); // this attribute comes from a different vertex buffer

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)0);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)(1 * sizeof(v4)));

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)(2 * sizeof(v4)));

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(m4x4), (void *)(3 * sizeof(v4)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // tell OpenGL this is an instanced vertex attribute.
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
}

internal void RenderGrassVBO(render *Render, entity_player *Player)
{
    //
    // NOTE(me): Preparing shader to render
    //
    u32 ShaderProg = Render->GrassShaderProgram;
    glUseProgram(ShaderProg);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, Render->DepthMap);
    glUniform1i(glGetUniformLocation(ShaderProg, "ShadowMap"), 1);

    // матрица проекции (источник света для теней)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-Render->ShadowMapSize, Render->ShadowMapSize, -Render->ShadowMapSize, Render->ShadowMapSize,
            Render->NearPlane, Render->FarPlane);
    r32 MatProjShadows[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProjShadows);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatProjShadows"), 1, GL_FALSE, MatProjShadows);

    // матрица вида (источник света для теней)
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glRotatef(-Render->ShadowLightPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-Render->ShadowLightYaw, 0.0f, 0.0f, 1.0f);
    glTranslatef(-Render->ShadowLightPos.x, -Render->ShadowLightPos.y, -Render->ShadowLightPos.z);
    r32 MatViewShadows[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatViewShadows);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatViewShadows"), 1, GL_FALSE, MatViewShadows);

    // матрица проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Render->AspectRatio * Render->FOV, Render->AspectRatio * Render->FOV, //
              -Render->FOV, Render->FOV, Render->FOV * 2, 1000);
    r32 MatProj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatProj"), 1, GL_FALSE, MatProj);

    // матрица вида с камеры
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    OGLSetCameraOnPlayer(Player);
    r32 MatView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatView"), 1, GL_FALSE, MatView);

    // отправка позиции камеры (игрока) в шейдер
    glUniform3fv(glGetUniformLocation(ShaderProg, "gCameraWorldPos"), 1, Player->Position.E);

    LightSourcesToShader(Render, ShaderProg);

    //
    // NOTE(me): Rendering Multiple Single Meshes
    //

    u32 BaseVertex = 0;
    u32 BaseIndex = 0;
    u32 BaseInstance = 0;
    for(u32 i = 0; i < Render->GrMeshesCount; i++)
    {
        single_mesh *Mesh = Render->GrMeshes[i];

        // отправка материала в шейдер
        if(Mesh->WithMaterial)
        {
            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.AmbientColor"), 1, //
                         Mesh->Material.Ambient.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.DiffuseColor"), 1, //
                         Mesh->Material.Diffuse.E);

            glUniform3fv(glGetUniformLocation(ShaderProg, "gMaterial.SpecularColor"), 1, //
                         Mesh->Material.Specular.E);

            if(Mesh->Material.WithTexture)
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), true);
                glActiveTexture(GL_TEXTURE0 + 0);
                glBindTexture(GL_TEXTURE_2D, Mesh->Material.Texture);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSampler"), 0);
                glUniform1i(glGetUniformLocation(ShaderProg, "gSamplerSpecularExponent"), 0);
            }
            else
            {
                glUniform1i(glGetUniformLocation(ShaderProg, "gWithTexture"), false);
            }
        }
        else
        {
            InvalidCodePath;
        }

        // отрисовка меша
        glBindVertexArray(Render->GrVAO);
        glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, Mesh->IndicesCount, GL_UNSIGNED_INT,
                                                      (void *)(sizeof(u32) * BaseIndex), //
                                                      Render->GrInstancingCounters[i][0],
                                                      BaseVertex, //
                                                      BaseInstance);
        glBindVertexArray(0);

        // смещение к следующему мешу
        BaseVertex += Mesh->VerticesCount;
        BaseIndex += Mesh->IndicesCount;
        BaseInstance += Render->GrInstancingCounters[i][0];
    }

    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
}

//
// NOTE(me): Other
//
void DrawOrthoTexturedRectangle(render *Render, u32 Texture, r32 TextureWidth, s32 TextureHeight, v2 Offset)
{
    // glDisable(GL_DEPTH_TEST);

    r32 VRectangle[] = {
        -1, -1, //
        0,  1,  //
        -1, 0,  //
        1,  1,  //
        0,  -1, //
        1,  0   //
    };

    r32 TexRectangle[] = {
        0, 1, //
        1, 1, //
        1, 0, //
        0, 0  //
    };

    // Ортогональная проекция и единичная матрица модели
    // glViewport(0, 0, Render->DisplayWidth, Render->DisplayHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Render->DisplayWidth - 1, Render->DisplayHeight - 1, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glPushMatrix();
    glTranslatef(Offset.x, Offset.y, 0);

    glScalef((r32)TextureWidth, (r32)TextureHeight, 0);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glColor3f(1, 1, 1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glTexCoordPointer(2, GL_FLOAT, 0, TexRectangle);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

internal void OGLDrawLinesOXYZ(v3 Normal, r32 LineWidth, r32 LineMin = -0.5, r32 LineMax = 0.5, r32 Offset = 0.001)
{
    // TODO(me): Add arrows?
    r32 LineVertices[] = {
        LineMin, Offset,  Offset,  // 0 - x
        LineMax, Offset,  Offset,  // 1 - x
        Offset,  LineMin, Offset,  // 0 - y
        Offset,  LineMax, Offset,  // 1 - y
        Offset,  Offset,  LineMin, // 0 - z
        Offset,  Offset,  LineMax  // 1 - z

    };
    r32 LineColors[] = {
        1, 0, 0, // 0 - red
        1, 0, 0, // 1 - red
        0, 0, 1, // 0 - green
        0, 0, 1, // 1 - green
        0, 1, 0, // 0 - blue
        0, 1, 0, // 1 - blue
    };

    glNormal3f(Normal.x, Normal.y, Normal.z);
    glLineWidth(LineWidth);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, LineVertices);
    glColorPointer(3, GL_FLOAT, 0, LineColors);
    glDrawArrays(GL_LINES, 0, 6);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glLineWidth(20);
    r32 ArrowVertices[] = {
        LineMax,
        Offset,
        Offset, // 0 - x
        LineMax + 0.25f * LineMax,
        Offset,
        Offset, // 1 - x
        Offset,
        LineMax,
        Offset, // 0 - y
        Offset,
        LineMax + 0.25f * LineMax,
        Offset, // 1 - y
        Offset,
        Offset,
        LineMax, // 0 - z
        Offset,
        Offset,
        LineMax + 0.25f * LineMax // 1 - z

    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, ArrowVertices);
    glColorPointer(3, GL_FLOAT, 0, LineColors);
    glDrawArrays(GL_LINES, 0, 6);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

internal void DrawRectangularParallelepiped(r32 MinX, r32 MinY, r32 MaxX, r32 MaxY, //
                                            r32 MinZ, r32 MaxZ,                     //
                                            v3 Color)
{
    r32 VertPositions[] = {
        // bot
        MinX, MinY, MinZ, // 0
        MaxX, MinY, MinZ, // 1
        MaxX, MaxY, MinZ, // 2
        MinX, MaxY, MinZ, // 3
        // top
        MinX, MinY, MaxZ, // 4
        MaxX, MinY, MaxZ, // 5
        MaxX, MaxY, MaxZ, // 6
        MinX, MaxY, MaxZ, // 7
    };

    r32 VertColors[] = {
        // bot
        Color.x, Color.y, Color.z, // 0
        Color.x, Color.y, Color.z, // 1
        Color.x, Color.y, Color.z, // 2
        Color.x, Color.y, Color.z, // 3
        // top
        Color.x, Color.y, Color.z, // 4
        Color.x, Color.y, Color.z, // 5
        Color.x, Color.y, Color.z, // 6
        Color.x, Color.y, Color.z  // 7
    };

    u32 Indices[] = {
        0, 1, 1, 2, 2, 3, 3, 0, // bot
        4, 5, 5, 6, 6, 7, 7, 4, // top
        0, 4, 3, 7, 1, 5, 2, 6, // side
    };

    s32 IndicesCount = ArrayCount(Indices);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, VertPositions);
    glColorPointer(3, GL_FLOAT, 0, VertColors);
    glDrawElements(GL_LINES, IndicesCount, GL_UNSIGNED_INT, Indices);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
}

void DrawTexturedRectangle(r32 VRectangle[], u32 Texture, r32 Repeat)
{
    r32 TexRectangle[] = {
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

    glVertexPointer(3, GL_FLOAT, 0, VRectangle);
    glTexCoordPointer(2, GL_FLOAT, 0, TexRectangle);
    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);
}

internal void RenderPlayerClipZones(render *Render, entity_clip *PlayerClip)
{
    r32 MinX = PlayerClip->CenterPos.x - 0.5f * PlayerClip->Side;
    r32 MinY = PlayerClip->CenterPos.y - 0.5f * PlayerClip->Side;
    r32 MaxX = PlayerClip->CenterPos.x + 0.5f * PlayerClip->Side;
    r32 MaxY = PlayerClip->CenterPos.y + 0.5f * PlayerClip->Side;
    r32 MinZ = 0;

    r32 VRectangle[] = {
        // bot
        MinX, MinY, MinZ, // 0
        MaxX, MinY, MinZ, // 1
        MaxX, MaxY, MinZ, // 2
        MinX, MaxY, MinZ  // 3
    };

    // текстура клипа на плоскости пола
    DrawTexturedRectangle(VRectangle, Render->ClipTexture, PlayerClip->Side);

    // коробка клипа
    glLineWidth(5);
    glEnable(GL_LINE_SMOOTH);
    DrawRectangularParallelepiped(MinX, MinY, MaxX, MaxY, MinZ, 2.7f, V3(1, 0, 0));
    glDisable(GL_LINE_SMOOTH);
}

internal void RenderLightingPositions(render *Render)
{
    point_light *PointLights = Render->PointLights;
    r32 MinX = PointLights[0].WorldPosition.x;
    r32 MinY = PointLights[0].WorldPosition.y - 0.5f;
    r32 MaxX = PointLights[0].WorldPosition.x + 1.0f;
    r32 MaxY = PointLights[0].WorldPosition.y + 0.5f;
    r32 MinZ = PointLights[0].WorldPosition.z;
    r32 MaxZ = PointLights[0].WorldPosition.z + 1.0f;

    r32 CenterY = MinY + (MaxY - MinY) / 2.0f;
    r32 PLVRectangle[] = {
        // bot
        MinX, CenterY, MinZ, // 0
        MaxX, CenterY, MinZ, // 1
        MaxX, CenterY, MaxZ, // 2
        MinX, CenterY, MaxZ  // 3
    };

    // куб в позиции источника света
    glLineWidth(5);
    glEnable(GL_LINE_SMOOTH);
    DrawRectangularParallelepiped(MinX, MinY, MaxX, MaxY, MinZ, MaxZ, V3(1, 1, 0));
    glDisable(GL_LINE_SMOOTH);

    // текстура в позиции точечного источника света
    DrawTexturedRectangle(PLVRectangle, Render->LightTexture, 1);

    spot_light *SpotLights = Render->SpotLights;
    MinX = SpotLights[0].Base.WorldPosition.x;
    MinY = SpotLights[0].Base.WorldPosition.y - 0.5f;
    MaxX = SpotLights[0].Base.WorldPosition.x + 1.0f;
    MaxY = SpotLights[0].Base.WorldPosition.y + 0.5f;
    MinZ = SpotLights[0].Base.WorldPosition.z;
    MaxZ = SpotLights[0].Base.WorldPosition.z + 1.0f;

    CenterY = MinY + (MaxY - MinY) / 2.0f;
    r32 SLVRectangle[] = {
        // bot
        MinX, CenterY, MinZ, // 0
        MaxX, CenterY, MinZ, // 1
        MaxX, CenterY, MaxZ, // 2
        MinX, CenterY, MaxZ  // 3
    };

    // куб в позиции прожектора
    glLineWidth(5);
    glEnable(GL_LINE_SMOOTH);
    DrawRectangularParallelepiped(MinX, MinY, MaxX, MaxY, MinZ, MaxZ, V3(1, 1, 0));
    glDisable(GL_LINE_SMOOTH);

    // текстура в позиции точечного источника света
    DrawTexturedRectangle(SLVRectangle, Render->LightTexture, 1);
}

/*
internal void DrawColoredRectangle(r32 MinX, r32 MinY, r32 MaxX, r32 MaxY, r32 Z, v3 Color)
{
    r32 RectangleVertices[] = {
        MinX, MinY, Z, // 0
        MaxX, MinY, Z, // 1
        MaxX, MaxY, Z, // 2
        MinX, MaxY, Z, // 3
    };

    u32 RectangleIndices[] = {
        0, 1, 3, // 1st triangle
        1, 2, 3, // 2nd triangle
    };
    s32 RectangleIndicesCount = ArrayCount(RectangleIndices);

    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, RectangleVertices);
    glColor3f(Color.x, Color.y, Color.z);
    glDrawElements(GL_TRIANGLES, RectangleIndicesCount, GL_UNSIGNED_INT, RectangleIndices);

    glDisableClientState(GL_VERTEX_ARRAY);
}
*/

internal void RenderWater(render *Render, entity_player *Player, r32 dtForFrame, r32 WaterZ)
{
#if 1
    u32 ShaderProg = Render->WaterShaderProgram;
    glUseProgram(ShaderProg);

    Render->WaterMoveFactor += Render->WaterWaveSpeed * dtForFrame;
    if(Render->WaterMoveFactor >= 1)
    {
        Render->WaterMoveFactor = 0;
    }

    glUniform1f(glGetUniformLocation(ShaderProg, "Reflectivity"), Render->WaterReflectivity);

    glUniform1f(glGetUniformLocation(ShaderProg, "ShineDamper"), Render->WaterShineDamper);

    glUniform1f(glGetUniformLocation(ShaderProg, "WaveStrength"), Render->WaterWaveStrength);

    glUniform1f(glGetUniformLocation(ShaderProg, "Tiling"), Render->WaterTiling);

    glUniform3fv(glGetUniformLocation(ShaderProg, "LightPosition"), 1, Render->DirLight.WorldDirection.E);

    glUniform3fv(glGetUniformLocation(ShaderProg, "LightColor"), 1, Render->DirLight.Base.Color.E);

    glUniform3fv(glGetUniformLocation(ShaderProg, "CameraPosition"), 1, Player->Position.E);

    glUniform1f(glGetUniformLocation(ShaderProg, "MoveFactor"), Render->WaterMoveFactor);

    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, Render->WaterReflTexture);
    glUniform1i(glGetUniformLocation(ShaderProg, "ReflectionTexture"), 0);

    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_2D, Render->WaterRefrTexture);
    glUniform1i(glGetUniformLocation(ShaderProg, "RefractionTexture"), 1);

    glActiveTexture(GL_TEXTURE0 + 2);
    glBindTexture(GL_TEXTURE_2D, Render->WaterDUDVTexture);
    glUniform1i(glGetUniformLocation(ShaderProg, "DUDVMap"), 2);

    glActiveTexture(GL_TEXTURE0 + 3);
    glBindTexture(GL_TEXTURE_2D, Render->WaterNormalMap);
    glUniform1i(glGetUniformLocation(ShaderProg, "NormalMap"), 3);

    glActiveTexture(GL_TEXTURE0 + 4);
    glBindTexture(GL_TEXTURE_2D, Render->WaterRefrDepthTexture);
    glUniform1i(glGetUniformLocation(ShaderProg, "DepthMap"), 4);

    // матрица проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Render->AspectRatio * Render->FOV, Render->AspectRatio * Render->FOV, //
              -Render->FOV, Render->FOV, Render->FOV * 2, 1000);
    r32 MatProj[16];
    glGetFloatv(GL_PROJECTION_MATRIX, MatProj);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatProj"), 1, GL_FALSE, MatProj);

    // матрица вида с камеры
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    OGLSetCameraOnPlayer(Player);
    r32 MatView[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, MatView);
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatView"), 1, GL_FALSE, MatView);

    // матрица модели
    m4x4 MatModel = Identity();
    // m4x4 TranslateMatrix = Translation(V3(1, 1, 0));
    // MatModel = Identity() * TranslateMatrix;
    MatModel = Transpose(MatModel); // opengl to glsl format
    glUniformMatrix4fv(glGetUniformLocation(ShaderProg, "MatModel"), 1, GL_FALSE, (const GLfloat *)&MatModel);
#else

    glDisable(GL_TEXTURE_2D);
    glLoadIdentity();

    // матрица проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Render->AspectRatio * Render->FOV, Render->AspectRatio * Render->FOV, //
              -Render->FOV, Render->FOV, Render->FOV * 2, 1000);

    // вид с камеры
    OGLSetCameraOnPlayer(Player);
#endif

    r32 PitRadiusDiv2 = 5;
    r32 MinX = 20 - PitRadiusDiv2;
    r32 MinY = 10 - PitRadiusDiv2;
    r32 MaxX = 20 + PitRadiusDiv2;
    r32 MaxY = 10 + PitRadiusDiv2;

    r32 RectangleVertices[] = {
        MinX, MinY, WaterZ, // 0
        MaxX, MinY, WaterZ, // 1
        MaxX, MaxY, WaterZ, // 2
        MinX, MaxY, WaterZ, // 3
    };
    glEnableClientState(GL_VERTEX_ARRAY);

    glVertexPointer(3, GL_FLOAT, 0, RectangleVertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);

    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);
}

void InitWaterFBOs(render *Render)
{
    //
    // NOTE(me): Init Reflection Frame Buffer
    //

    u32 REFLECTION_WIDTH = (u32)(Render->ReflWidth);
    u32 REFLECTION_HEIGHT = (u32)(Render->ReflHeight);

    glGenFramebuffers(1, &Render->WaterReflFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Render->WaterReflFBO);

    // Create Texture Attachment
    glGenTextures(1, &Render->WaterReflTexture);
    glBindTexture(GL_TEXTURE_2D, Render->WaterReflTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, REFLECTION_WIDTH, REFLECTION_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Render->WaterReflTexture, 0);

    // Create Depth Buffer Attachment
    glGenRenderbuffers(1, &Render->WaterReflDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, Render->WaterReflDepthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, REFLECTION_WIDTH, REFLECTION_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, Render->WaterReflDepthRBO);

    //
    // NOTE(me): Init Refraction Frame Buffer
    //

    u32 REFRACTION_WIDTH = (u32)(Render->RefrWidth);
    u32 REFRACTION_HEIGHT = (u32)(Render->RefrHeight);

    glGenFramebuffers(1, &Render->WaterRefrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Render->WaterRefrFBO);

    // Create Texture Attachment
    glGenTextures(1, &Render->WaterRefrTexture);
    glBindTexture(GL_TEXTURE_2D, Render->WaterRefrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, REFRACTION_WIDTH, REFRACTION_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Render->WaterRefrTexture, 0);

    // Create Depth Texture Attachment
    glGenTextures(1, &Render->WaterRefrDepthTexture);
    glBindTexture(GL_TEXTURE_2D, Render->WaterRefrDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, REFRACTION_WIDTH, REFRACTION_HEIGHT, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Render->WaterRefrDepthTexture, 0);
}

void InitDepthMapFBO(render *Render)
{
    u32 DEPTH_MAP_WIDTH = (u32)(Render->DepthMapWidth);
    u32 DEPTH_MAP_HEIGHT = (u32)(Render->DepthMapHeight);

    glGenFramebuffers(1, &Render->DepthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, Render->DepthMapFBO);

    // Create Texture Attachment
    glGenTextures(1, &Render->DepthMap);
    glBindTexture(GL_TEXTURE_2D, Render->DepthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, DEPTH_MAP_WIDTH, DEPTH_MAP_HEIGHT, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Attach Depth Texture to Framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, Render->DepthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Render->DepthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

void RenderScene(render *Render, u32 ShaderProg, entity_player *Player, GLbitfield glClearMask)
{
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glEnable(GL_DEPTH_TEST);
    glClear(glClearMask);

    // glEnable(GL_ALPHA_TEST);
    // glAlphaFunc(GL_GREATER, 0.01f);
    // glEnable(GL_NORMALIZE);

    RenderEnvVBOs(Render, ShaderProg, Player);
    // glDisable(GL_ALPHA_TEST);
    // glDisable(GL_NORMALIZE);
}

void RenderDebugElements(render *Render, entity_player *Player, entity_clip *PlayerClip)
{
    glLoadIdentity();
    // матрица проекции
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Render->AspectRatio * Render->FOV, Render->AspectRatio * Render->FOV, //
              -Render->FOV, Render->FOV, Render->FOV * 2, 1000);

    // вид с камеры
    OGLSetCameraOnPlayer(Player);

    RenderPlayerClipZones(Render, PlayerClip);
    RenderLightingPositions(Render);

    glPushMatrix();
    glScalef(5, 5, 5);
    OGLDrawLinesOXYZ(V3(0, 0, 1), 1); // World Start Point OXYZ
    glPopMatrix();
}