void CalcNormal(v3 A, v3 B, v3 C, v3 *Result)
{
    // vector 1 = B-A; vector 2 = C-A
    v3 V1, V2;
    r32 Magnitude;

    V1.x = A.x - B.x;
    V1.y = A.y - B.y;
    V1.z = A.z - B.z;
    V2.x = B.x - C.x;
    V2.y = B.y - C.y;
    V2.z = B.z - C.z;

    // N= (B-A)x(C-A)
    Result->x = (V1.y * V2.z - V1.z * V2.y);
    Result->y = (V1.z * V2.x - V1.x * V2.z);
    Result->z = (V1.x * V2.y - V1.y * V2.x);

    Magnitude = SquareRoot(Square(Result->x) + Square(Result->y) + Square(Result->z));
    Result->x /= Magnitude;
    Result->y /= Magnitude;
    Result->z /= Magnitude;
}

b32 IsPosOnTerrain(r32 x, r32 y)
{
    b32 Result;

    Result = ((x >= 0) &&    //
              (x < TMapW) && //
              (y >= 0) &&    //
              (y < TMapH));

    return (Result);
}

r32 TerrainGetHeight(entity_envobject *Terrain, r32 x, r32 y)
{
    // алгоритм нахождения приблизительной высоты на карте
    // 1. если позиция камеры вне карты, то высоту не меняем
    // 2. находим X и Y индексы клетки в массиве Terrain Map
    // 3. находим BaseOffset смещение от нулевой позиции в клетке (Camera.x - X) и (Camera.y - Y)
    // 4. находим первый вес h1 для текущей позиции камеры и позиции, смещённой на 1 по оси x
    // по формуле: h1 = ((1 - BaseOffsetX) * TMap[X][Y].z + BaseOffsetX * TMap[X + 1][Y].z)
    // 5. находим второй вес h2 для текущей позиции камеры, смещённой на 1 по оси y
    // и позиции, смещённой на 1 по осям x и y
    // по формуле: h2 = ((1 - BaseOffsetX) * TMap[X][Y + 1].z + BaseOffsetX * TMap[X + 1][Y + 1].z)
    // 6. находим приблизительную высоту по формуле:
    // Result = (1 - BaseOffsetY) * h1 + BaseOffsetY * h2

    r32 Result;

    // GameState->EnvObjects[0]->Model->Meshes[0]
    v3 *Positions = Terrain->Model->Meshes[0].Positions;

    if(!IsPosOnTerrain(x, y))
    {
        return 0.0f;
    }

    int32 X = (int32)x;
    int32 Y = (int32)y;
    r32 BaseOffsetX = x - X;
    r32 BaseOffsetY = y - Y;

    u32 Index0 = X * TMapH + Y;       // [i][j]
    u32 Index1 = (X + 1) * TMapH + Y; // [i+1][j]
    r32 h1 = ((1 - BaseOffsetX) * Positions[Index0].z + BaseOffsetX * Positions[Index1].z);

    u32 Index2 = X * TMapH + Y + 1;       // [i][j+1]
    u32 Index3 = (X + 1) * TMapH + Y + 1; // [i+1][j+1]
    r32 h2 = ((1 - BaseOffsetX) * Positions[Index2].z + BaseOffsetX * Positions[Index3].z);

    Result = (1 - BaseOffsetY) * h1 + BaseOffsetY * h2;

    return (Result);
}

void CreateHill(v3 *Positions, s32 PosX, s32 PosY, s32 PosZ, s32 Radius)
{
    for(s32 i = PosX - Radius; i <= PosX + Radius; i++)
    {
        for(s32 j = PosY - Radius; j <= PosY + Radius; j++)
        {
            u32 TmpIndex = i * TMapH + j;

            if(IsPosOnTerrain((r32)i, (r32)j))
            {
                // TODO(me): избавиться от math.z
                r32 t1 = (r32)(PosX - i);
                r32 t2 = (r32)(PosY - j);
                r32 Length = SquareRoot(Square(t1) + Square(t2));
                if(Length < Radius)
                {
                    Length = Length / Radius * (r32)Pi32_2;
                    Positions[TmpIndex].z += Cos(Length) * PosZ;
                }
            }
        }
    }
}

internal loaded_model *CreateTerrainModel(memory_arena *WorldArena)
{
    loaded_model *Result = PushStruct(WorldArena, loaded_model);

    Result->Name = PushString(WorldArena, "TerrainModel");

    Result->MeshesCount = 1;

    Result->Meshes = PushArray(WorldArena, Result->MeshesCount, single_mesh);

    single_mesh *Mesh = &Result->Meshes[0];

    Mesh->Name = PushString(WorldArena, "TerrainMesh");

    Mesh->VerticesCount = TMapW * TMapH;

    Mesh->Positions = PushArray(WorldArena, Mesh->VerticesCount, v3);
    Mesh->TexCoords = PushArray(WorldArena, Mesh->VerticesCount, v2);

    u32 VerticesCountTmp = 0;
    for(u32 i = 0; i < TMapW; i++)
    {
        for(u32 j = 0; j < TMapH; j++)
        {
            Mesh->Positions[VerticesCountTmp] = V3((r32)i, (r32)j, (rand() % 10) * 0.02f);
            Mesh->TexCoords[VerticesCountTmp] = V2((r32)i, (r32)j);
            VerticesCountTmp++;
        }
    }

    Mesh->IndicesCount = (TMapW - 1) * (TMapH - 1) * 6;
    Mesh->Indices = PushArray(WorldArena, Mesh->IndicesCount, u32);

    for(u32 i = 0; i < TMapW - 1; i++)
    {
        u32 Pos = i * TMapH; // номер ячейки массива использующий сквозную нумерацию
        for(u32 j = 0; j < TMapH - 1; j++)
        {
            // Flat[ x * TMapH * depth + y * depth + z ] = elements[x][y][z]
            u32 TmpIndex = i * (TMapH - 1) * 6 + j * 6;

            // первый треугольник на плоскости (левая верхняя часть квадрата)
            Mesh->Indices[TmpIndex + 0] = Pos;
            Mesh->Indices[TmpIndex + 1] = Pos + 1; // переход к следующей вершине (перемещение по оси y)
            Mesh->Indices[TmpIndex + 2] =
                Pos + 1 + TMapH; // переход к вершине во второй размерности (перемещение по оси x)

            // второй треугольник на плоскости (правая нижняя часть квадрата)
            Mesh->Indices[TmpIndex + 3] = Pos + 1 + TMapH;
            Mesh->Indices[TmpIndex + 4] = Pos + TMapH;
            Mesh->Indices[TmpIndex + 5] = Pos;
            Pos++;
        }
    }

    // касательные и бикасательные для маппинга нормалей
    // Mesh->Tangents = PushArray(WorldArena, Mesh->VerticesCount, v3);
    // fread(Mesh->Tangents, sizeof(v3) * Mesh->VerticesCount, 1, In);

    // создание холмов
    for(u32 i = 0; i < 10; i++)
    {
        u32 HillX = rand() % TMapW;
        u32 HillY = rand() % TMapH;
        u32 HillZ = rand() % 10;
        u32 HillRadius = rand() % 50;
        CreateHill(Mesh->Positions, HillX, HillY, HillZ, HillRadius);
    }

    // создание ямы
    u32 PitX = 20;
    u32 PitY = 10;
    s32 PitZ = -5;
    u32 PitRadius = 5;
    CreateHill(Mesh->Positions, PitX, PitY, PitZ, PitRadius);

    // заполнение карты нормалей террейна
    Mesh->Normals = PushArray(WorldArena, Mesh->VerticesCount, v3);
    for(u32 i = 0; i < TMapW; i++)
    {
        for(u32 j = 0; j < TMapH; j++)
        {
            u32 TmpIndex = i * TMapH + j;
            u32 TmpIndex1 = (i + 1) * TMapH + j; // [i+1][j]
            u32 TmpIndex2 = i * TMapH + j + 1;   // [i][j+1]

            // 3 соседние вершины дают нормаль, направленную вверх
            CalcNormal(Mesh->Positions[TmpIndex],  //
                       Mesh->Positions[TmpIndex1], //
                       Mesh->Positions[TmpIndex2], //
                       &Mesh->Normals[TmpIndex]);  // полученная нормаль
        }
    }

    Mesh->Material.Ambient = V4(0.2f, 0.2f, 0.2f, 1.0f);
    Mesh->Material.Diffuse = V4(0.8f, 0.8f, 0.8f, 1.0f);
    Mesh->Material.Specular = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.0f;

    Mesh->WithMaterial = true;
    /*Mesh->Material.Ambient = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Diffuse = V4(0.1f, 0.35f, 0.1f, 1.0f);
    Mesh->Material.Specular = V4(0.45f, 0.55f, 0.45f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.25f;*/

    Mesh->Material.WithTexture = true;
    Mesh->Material.TextureName = PushString(WorldArena, "pole.png");
    Mesh->Material.Texture = LoadTexture(&Mesh->Material.TextureName);

    return (Result);
}

internal loaded_model *CreateGrassModel(memory_arena *WorldArena)
{
    loaded_model *Result = PushStruct(WorldArena, loaded_model);

    Result->Name = PushString(WorldArena, "GrassModel");

    Result->MeshesCount = 1;

    Result->Meshes = PushArray(WorldArena, Result->MeshesCount, single_mesh);

    single_mesh *Mesh = &Result->Meshes[0];

    Mesh->Name = PushString(WorldArena, "GrassMesh");

    Mesh->VerticesCount = 8;

    Mesh->Positions = PushArray(WorldArena, Mesh->VerticesCount, v3);
    Mesh->Positions[0] = V3(-0.5, 0, 0);
    Mesh->Positions[1] = V3(0.5, 0, 0);
    Mesh->Positions[2] = V3(0.5, 0, 1);
    Mesh->Positions[3] = V3(-0.5, 0, 1);
    Mesh->Positions[4] = V3(0, -0.5, 0);
    Mesh->Positions[5] = V3(0, 0.5, 0);
    Mesh->Positions[6] = V3(0, 0.5, 1);
    Mesh->Positions[7] = V3(0, -0.5, 1);

    Mesh->TexCoords = PushArray(WorldArena, Mesh->VerticesCount, v2);
    Mesh->TexCoords[0] = V2(0, 1);
    Mesh->TexCoords[1] = V2(1, 1);
    Mesh->TexCoords[2] = V2(1, 0);
    Mesh->TexCoords[3] = V2(0, 0);
    Mesh->TexCoords[4] = V2(0, 1);
    Mesh->TexCoords[5] = V2(1, 1);
    Mesh->TexCoords[6] = V2(1, 0);
    Mesh->TexCoords[7] = V2(0, 0);

    Mesh->IndicesCount = 12;
    Mesh->Indices = PushArray(WorldArena, Mesh->IndicesCount, u32);
    Mesh->Indices[0] = 0;
    Mesh->Indices[1] = 1;
    Mesh->Indices[2] = 2;
    Mesh->Indices[3] = 2;
    Mesh->Indices[4] = 3;
    Mesh->Indices[5] = 0;
    Mesh->Indices[6] = 4;
    Mesh->Indices[7] = 5;
    Mesh->Indices[8] = 6;
    Mesh->Indices[9] = 6;
    Mesh->Indices[10] = 7;
    Mesh->Indices[11] = 4;

    Mesh->Normals = PushArray(WorldArena, Mesh->VerticesCount, v3);
    for(u32 i = 0; i < Mesh->VerticesCount; i++)
    {
        Mesh->Normals[i] = V3(0, 0, 1);
    }

    Mesh->WithMaterial = true;
    /*
    Mesh->Material.Ambient = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Diffuse = V4(0.1f, 0.35f, 0.1f, 1.0f);
    Mesh->Material.Specular = V4(0.45f, 0.55f, 0.45f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.25f;
     */
    Mesh->Material.Ambient = V4(0.2f, 0.2f, 0.2f, 1.0f);
    Mesh->Material.Diffuse = V4(0.8f, 0.8f, 0.8f, 1.0f);
    Mesh->Material.Specular = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.0f;

    Mesh->Material.WithTexture = true;
    Mesh->Material.TextureName = PushString(WorldArena, "trava.png");
    Mesh->Material.Texture = LoadTexture(&Mesh->Material.TextureName);

    return (Result);
}

/*
internal v3 *CreateInstancingTranslations(memory_arena *WorldArena, entity_envobject *Terrain, v3 StartPos, u32 Count)
{
    v3 *Result = PushArray(WorldArena, Count, v3);

    for(u32 i = 0; i < Count; i++)
    {
        // Result[i].x = (r32)(rand() % TMapW);
        // Result[i].y = (r32)(rand() % TMapH);
        Result[i].x = (r32)(rand()) / (r32)(RAND_MAX / (TMapW - 2));
        Result[i].y = (r32)(rand()) / (r32)(RAND_MAX / (TMapH - 2));
        Result[i].z = TerrainGetHeight(Terrain, Result[i].x, Result[i].y);
    }

    return (Result);
}
*/

internal r32 DebugGetRandomNumberR32(r32 Min, r32 Max, u32 Precision)
{
    r32 Result;

    // получить случайное число как целое число с порядком precision
    Result = (r32)(rand() % (int)pow(10, Precision));

    // получить вещественное число
    Result = (r32)(Min + (Result / pow(10, Precision)) * (Max - Min));

    return (Result);
}

internal m4x4 *CreateInstancingTransformMatrices(memory_arena *WorldArena,  //
                                                 entity_envobject *Terrain, //
                                                 u32 Count,                 //
                                                 v3 SMM,                    // Scale rand() Min, Max, Precision
                                                 v3 Rotate,                 // Rotate X, Y, Z
                                                 v3 RXMM,                   // Rotate X rand() Min, Max, Precision
                                                 v3 RYMM,                   // Rotate Y rand() Min, Max, Precision
                                                 v3 RZMM)                   // Rotate Z rand() Min, Max, Precision
{
    m4x4 *Result = PushArray(WorldArena, Count, m4x4);

    for(u32 i = 0; i < Count; i++)
    {
        v3 TranslationVec = V3(0, 0, 0);
        TranslationVec.x = (r32)(rand()) / (r32)(RAND_MAX / (TMapW - 2));
        TranslationVec.y = (r32)(rand()) / (r32)(RAND_MAX / (TMapH - 2));
        TranslationVec.z = TerrainGetHeight(Terrain, TranslationVec.x, TranslationVec.y);
        m4x4 TranslationM = Translation(TranslationVec);

        r32 RotX = DebugGetRandomNumberR32(RXMM.x, RXMM.y, (u32)RXMM.z);
        r32 RotY = DebugGetRandomNumberR32(RYMM.x, RYMM.y, (u32)RYMM.z);
        r32 RotZ = DebugGetRandomNumberR32(RZMM.x, RZMM.y, (u32)RZMM.z);
        m4x4 RotationM = XRotation(Rotate.x) * YRotation(Rotate.y) * ZRotation(Rotate.z) //
                         * XRotation(RotX) * YRotation(RotY) * ZRotation(RotZ);

        r32 Scale = DebugGetRandomNumberR32(SMM.x, SMM.y, (u32)SMM.z);
        // v3 ScaleVec = V3(Scale, Scale, Scale);
        // m4x4 ScalingM = Scaling(ScaleVec);
        m4x4 ScalingM = Scaling(Scale);

        Result[i] = TranslationM * RotationM * ScalingM;
        Result[i] = Transpose(Result[i]); // opengl to glsl format
    }

    return (Result);
}

internal loaded_model *CreateTexturedSquareModel(memory_arena *WorldArena, char *TextureName)
{
    loaded_model *Result = PushStruct(WorldArena, loaded_model);

    Result->Name = PushString(WorldArena, "TexturedSquareModel");

    Result->MeshesCount = 1;

    Result->Meshes = PushArray(WorldArena, Result->MeshesCount, single_mesh);

    single_mesh *Mesh = &Result->Meshes[0];

    Mesh->Name = PushString(WorldArena, "SquareMesh");

    Mesh->VerticesCount = 4;

    Mesh->Positions = PushArray(WorldArena, Mesh->VerticesCount, v3);
    /*
    Mesh->Positions[0] = V3(-0.5, -0.5, 0);
    Mesh->Positions[1] = V3(0.5, -0.5, 0);
    Mesh->Positions[2] = V3(0.5, 0.5, 0);
    Mesh->Positions[3] = V3(-0.5, 0.5, 0);
    */
    Mesh->Positions[0] = V3(0, 0, 0);
    Mesh->Positions[1] = V3(1, 0, 0);
    Mesh->Positions[2] = V3(1, 1, 0);
    Mesh->Positions[3] = V3(0, 1, 0);

    Mesh->TexCoords = PushArray(WorldArena, Mesh->VerticesCount, v2);
    Mesh->TexCoords[0] = V2(0, 1);
    Mesh->TexCoords[1] = V2(1, 1);
    Mesh->TexCoords[2] = V2(1, 0);
    Mesh->TexCoords[3] = V2(0, 0);

    Mesh->IndicesCount = 6;
    Mesh->Indices = PushArray(WorldArena, Mesh->IndicesCount, u32);
    Mesh->Indices[0] = 0;
    Mesh->Indices[1] = 1;
    Mesh->Indices[2] = 2;
    Mesh->Indices[3] = 2;
    Mesh->Indices[4] = 3;
    Mesh->Indices[5] = 0;

    Mesh->Normals = PushArray(WorldArena, Mesh->VerticesCount, v3);
    for(u32 i = 0; i < Mesh->VerticesCount; i++)
    {
        Mesh->Normals[i] = V3(0, 0, 1);
    }

    Mesh->WithMaterial = true;
    Mesh->Material.Ambient = V4(0.2f, 0.2f, 0.2f, 1.0f);
    Mesh->Material.Diffuse = V4(0.8f, 0.8f, 0.8f, 1.0f);
    Mesh->Material.Specular = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.0f;

    Mesh->Material.WithTexture = true;
    Mesh->Material.TextureName = PushString(WorldArena, TextureName);
    Mesh->Material.Texture = LoadTexture(&Mesh->Material.TextureName);

    return (Result);
}