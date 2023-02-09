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

/*b32 IsPosOnMap(r32 x, r32 y)
{
    b32 Result;

    Result = ((x >= 0) && (x < TMapW) && (y >= 0) && (y < TMapH));

    return (Result);
}

void TMapCreateHill(v3 *Positions, s32 x, s32 y, s32 rad, s32 z)
{
    for(s32 i = x - rad; i <= x + rad; i++)
    {
        for(s32 j = y - rad; j <= y + rad; j++)
        {
            if(IsPosOnMap((r32)i, (r32)j))
            {
                // TODO(me): избавиться от math.z
                //r32 len = (r32)sqrt(pow(x - i, 2) + pow(y - j, 2));
                r32 t1 = (r32)(x - i);
                r32 t2 = (r32)(y - j);
                r32 len = SquareRoot(Square(t1) + Square(t2));
                if(len < rad)
                {
                    len = len / rad * (r32)Pi32_2;

                    u32 TmpIndex = i * TMapH + j;
                    Positions[TmpIndex].z += Cos(len) * z;
                    // TMap[i][j].z += cos(len) * z;
                }
            }
        }
    }
}*/

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
        // CreateHill(World, HillX, HillY, HillZ, HillRadius);
        CreateHill(Mesh->Positions, HillX, HillY, HillZ, HillRadius);
    }

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

    /*Mesh->Material.Ambient = V4(0.2f, 0.2f, 0.2f, 1.0f);
    Mesh->Material.Diffuse = V4(0.8f, 0.8f, 0.8f, 1.0f);
    Mesh->Material.Specular = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.0f;*/

    Mesh->WithMaterial = true;
    Mesh->Material.Ambient = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Diffuse = V4(0.1f, 0.35f, 0.1f, 1.0f);
    Mesh->Material.Specular = V4(0.45f, 0.55f, 0.45f, 1.0f);
    Mesh->Material.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
    Mesh->Material.Shininess = 0.25f;

    Mesh->Material.WithTexture = true;
    Mesh->Material.TextureName = PushString(WorldArena, "pole.png");
    Mesh->Material.Texture = LoadTexture(&Mesh->Material.TextureName);

    return (Result);
}