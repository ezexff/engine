internal string ReadStringFromFile(memory_arena *WorldArena, FILE *In)
{
    string Result;

    // число символов в строке
    fread(&Result.Count, sizeof(u64), 1, In);

    // TODO(me): убрать? считываем строку во временный буффер
    char *TmpString = (char *)malloc(sizeof(char) * Result.Count + 1);
    fread(TmpString, sizeof(char), Result.Count, In);
    TmpString[Result.Count] = '\0';

    Result = PushString(WorldArena, TmpString);
    free(TmpString);

    return (Result);
}

internal u32 LoadTexture(string *FileName)
{
    u32 Result;

    // stbi_set_flip_vertically_on_load(true);

    // TODO(me): переделать?
    char *Dir = "assets/textures/";
    u64 FullPathLength = StringLength(Dir) + FileName->Count - 1;

    char *FullPath = (char *)malloc(FullPathLength);
    for(u32 i = 0; i < StringLength(Dir); i++)
    {
        FullPath[i] = Dir[i];
    }

    for(u32 i = 0; i < FileName->Count; i++)
    {
        FullPath[i + StringLength(Dir)] = FileName->Data[i];
    }
    glGenTextures(1, &Result);
    glBindTexture(GL_TEXTURE_2D, Result);
    // Texture wrapping or filtering options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int32 Width, Height, NrChannels;
    unsigned char *data = stbi_load(FullPath, &Width, &Height, &NrChannels, 0);
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, //
                     NrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        // Bitmap texture failed to load at path: Path
        char TempBuffer[256];
        _snprintf_s(TempBuffer, sizeof(TempBuffer), "[error] Texture loading: %s\n", FullPath);
        OutputDebugStringA(TempBuffer);
        Log.AddLog(TempBuffer);
    }
    stbi_image_free(data);

    return (Result);
}

//
// NOTE(me): 3d-model
//
internal node_anim *FindNodeAnim(animation *CurrentAnimation, string NodeName)
{
    for(u32 i = 0; i < CurrentAnimation->ChannelsCount; i++)
    {
        node_anim *NodeAnim = &CurrentAnimation->Channels[i];

        if(NodeAnim->Name == NodeName)
        {
            return NodeAnim;
        }
    }

    return NULL;
}

internal u32 FindScaling(r32 AnimationTimeTicks, node_anim *NodeAnim)
{
    Assert(NodeAnim->ScalingKeysCount > 0);

    for(u32 i = 0; i < NodeAnim->ScalingKeysCount - 1; i++)
    {
        r32 t = (r32)NodeAnim->ScalingKeys[i + 1].Time;
        if(AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

internal v3 CalcInterpolatedScaling(r32 AnimationTimeTicks, node_anim *NodeAnim)
{
    v3 Result = V3(0, 0, 0);

    // we need at least two values to interpolate...
    if(NodeAnim->ScalingKeysCount == 1)
    {
        Result = NodeAnim->ScalingKeys[0].Value;
        return (Result);
    }

    u32 ScalingIndex = FindScaling(AnimationTimeTicks, NodeAnim);
    u32 NextScalingIndex = ScalingIndex + 1;
    Assert(NextScalingIndex < NodeAnim->ScalingKeysCount);
    r32 t1 = (r32)NodeAnim->ScalingKeys[ScalingIndex].Time;
    r32 t2 = (r32)NodeAnim->ScalingKeys[NextScalingIndex].Time;
    r32 DeltaTime = t2 - t1;
    r32 Factor = (AnimationTimeTicks - (r32)t1) / DeltaTime;
    Assert(Factor >= 0.0f && Factor <= 1.0f);
    v3 Start = NodeAnim->ScalingKeys[ScalingIndex].Value;
    v3 End = NodeAnim->ScalingKeys[NextScalingIndex].Value;
    v3 Delta = End - Start;
    Result = Start + Factor * Delta;

    return (Result);
}

internal u32 FindRotation(r32 AnimationTimeTicks, node_anim *NodeAnim)
{
    Assert(NodeAnim->RotationKeysCount > 0);

    for(u32 i = 0; i < NodeAnim->RotationKeysCount - 1; i++)
    {
        r32 t = (r32)NodeAnim->RotationKeys[i + 1].Time;
        if(AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

internal v4 CalcInterpolatedRotation(r32 AnimationTimeTicks, node_anim *NodeAnim)
{
    v4 Result = V4(0, 0, 0, 0);

    // we need at least two values to interpolate...
    if(NodeAnim->RotationKeysCount == 1)
    {
        Result = NodeAnim->RotationKeys[0].Value;
        return (Result);
    }

    u32 RotationIndex = FindRotation(AnimationTimeTicks, NodeAnim);
    u32 NextRotationIndex = RotationIndex + 1;
    Assert(NextRotationIndex < NodeAnim->RotationKeysCount);
    r32 t1 = (r32)NodeAnim->RotationKeys[RotationIndex].Time;
    r32 t2 = (r32)NodeAnim->RotationKeys[NextRotationIndex].Time;
    r32 DeltaTime = t2 - t1;
    r32 Factor = (AnimationTimeTicks - t1) / DeltaTime;
    Assert(Factor >= 0.0f && Factor <= 1.0f);
    v4 Start = NodeAnim->RotationKeys[RotationIndex].Value;
    v4 End = NodeAnim->RotationKeys[NextRotationIndex].Value;
    // Result = Lerp(StartRotation, Factor, EndRotation);
    Result = AiLerp(Start, Factor, End);
    // v4 Delta = End - Start;
    // Result = Start + Factor * Delta;
    // Result = Normalize(Result);
    Result = AiNormalize(Result);

    return (Result);
}

internal u32 FindPosition(r32 AnimationTimeTicks, node_anim *NodeAnim)
{
    Assert(NodeAnim->PositionKeysCount > 0);

    for(u32 i = 0; i < NodeAnim->PositionKeysCount - 1; i++)
    {
        r32 t = (r32)NodeAnim->PositionKeys[i + 1].Time;
        if(AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

internal v3 CalcInterpolatedPosition(r32 AnimationTimeTicks, node_anim *NodeAnim)
{
    v3 Result = V3(0, 0, 0);

    // we need at least two values to interpolate...
    if(NodeAnim->PositionKeysCount == 1)
    {
        Result = NodeAnim->PositionKeys[0].Value;
        return (Result);
    }

    u32 PositionIndex = FindPosition(AnimationTimeTicks, NodeAnim);
    u32 NextPositionIndex = PositionIndex + 1;
    Assert(NextPositionIndex < NodeAnim->PositionKeysCount);
    r32 t1 = (r32)NodeAnim->PositionKeys[PositionIndex].Time;
    r32 t2 = (r32)NodeAnim->PositionKeys[NextPositionIndex].Time;
    r32 DeltaTime = t2 - t1;
    r32 Factor = (AnimationTimeTicks - t1) / DeltaTime;
    Assert(Factor >= 0.0f && Factor <= 1.0f);
    v3 Start = NodeAnim->PositionKeys[PositionIndex].Value;
    v3 End = NodeAnim->PositionKeys[NextPositionIndex].Value;
    v3 Delta = End - Start;
    Result = Start + Factor * Delta;

    return (Result);
}

internal void ReadNodeHierarchy(single_mesh *Mesh, u32 AnimIndex, r32 AnimationTimeTicks, node *Node,
                                m4x4 ParentTransform)
{
    animation *CurrentAnimation = &Mesh->Animations[AnimIndex];

    node_anim *NodeAnim = FindNodeAnim(CurrentAnimation, Node->Name);

    m4x4 NodeTransformation = Node->Transformation;

    if(Node->Name == "Scene")
    {
        // Mesh->SceneTransform = Inversion(NodeTransformation);
        Mesh->SceneTransform = Transpose(NodeTransformation);
        Mesh->WithSceneTransform = true;
    }

    if(NodeAnim)
    {
        // Interpolate scaling and generate scaling transformation matrix
        v3 ScaleVec = CalcInterpolatedScaling(AnimationTimeTicks, NodeAnim);
        m4x4 ScalingM = Scaling(ScaleVec);

        // Interpolate rotation and generate rotation transformation matrix
        v4 RotationVec = CalcInterpolatedRotation(AnimationTimeTicks, NodeAnim);
        // TODO(me): rotate by vector
        // m4x4 RotationM = XRotation(RotationVec.x) * YRotation(RotationVec.y) * ZRotation(RotationVec.z);
        m4x4 RotationM = Rotation(RotationVec);

        // Interpolate translation and generate translation transformation matrix
        v3 TranslationVec = CalcInterpolatedPosition(AnimationTimeTicks, NodeAnim);
        m4x4 TranslationM = Translation(TranslationVec);

        // Combine the above transformations
        // NodeTransformation = TranslationM * RotationM * ScalingM;
        NodeTransformation = TranslationM * RotationM * ScalingM;
    }

    m4x4 GlobalTransformation = ParentTransform * NodeTransformation;

    for(u32 i = 0; i < Mesh->BonesCount; i++)
    {
        if(Mesh->BoneNames[i] == Node->Name)
        {
            // TODO(me): перемещение всей сцены?
            // Mesh->FinalTransforms[i] = Mesh->SceneTransformMatrix * GlobalTransformation * Mesh->BoneOffsets[i];
            if(Mesh->WithSceneTransform)
            {
                Mesh->FinalTransforms[i] = Mesh->SceneTransform * GlobalTransformation * Mesh->BoneOffsets[i];
            }
            else
            {
                Mesh->FinalTransforms[i] = GlobalTransformation * Mesh->BoneOffsets[i];
            }
            // Mesh->FinalTransforms[i] = Mesh->SceneTransform * GlobalTransformation * Mesh->BoneOffsets[i];
            int Ending = 0;
            break;
        }
    }

    for(u32 i = 0; i < Node->ChildrenCount; i++)
    {
        ReadNodeHierarchy(Mesh, AnimIndex, AnimationTimeTicks, Node->Children[i], GlobalTransformation);
    }
}

internal void ProcessBoneTransformsHierarchy(memory_arena *WorldArena, node *Node, FILE *In)
{
    Node->Name = ReadStringFromFile(WorldArena, In);
    fread(&Node->Transformation, sizeof(m4x4), 1, In);
    fread(&Node->ChildrenCount, sizeof(u32), 1, In);

    Node->Children = (node **)PushStruct(WorldArena, node);
    for(u32 i = 0; i < Node->ChildrenCount; i++)
    {
        Node->Children[i] = PushStruct(WorldArena, node);
        Node->Children[i]->Parent = Node;
        ProcessBoneTransformsHierarchy(WorldArena, Node->Children[i], In);
    }
}

internal void GetBoneTransforms(single_mesh *Mesh, u32 AnimIndex, r32 TimeInSeconds)
{
    r32 TicksPerSecond =
        (r32)(Mesh->Animations[AnimIndex].TicksPerSeconds != 0 ? Mesh->Animations[AnimIndex].TicksPerSeconds : 25.0f);
    r32 TimeInTicks = TimeInSeconds * TicksPerSecond;
    r32 AnimationTimeTicks = (r32)fmod(TimeInTicks, (r32)Mesh->Animations[AnimIndex].Duration);

    // ReadNodeHierarchy(Mesh, AnimIndex, AnimationTimeTicks, Mesh->BoneTransformsHierarchy, Identity());

    // TODO(me): передать Identity матрицу?
    // ReadNodeHierarchy(AnimationTimeTicks, Mesh->Hierarchy, Identity(), Mesh);
    // ReadNodeHierarchy(AnimationTimeTicks, Mesh->Hierarchy->Children[0]->Children[1]->Children[0],
    //                  Mesh->Hierarchy->Children[0]->Children[1]->Transformation, Mesh);

    // Mesh->SceneTransformMatrix = Inversion(Mesh->Hierarchy->Children[0]->Transformation);

    // ReadNodeHierarchy(AnimationTimeTicks, Mesh->Hierarchy->Children[0]->Children[1], Identity(), Mesh);

    ReadNodeHierarchy(Mesh, AnimIndex, AnimationTimeTicks, Mesh->BoneTransformsHierarchy, Identity());

    // ReadNodeHierarchy(Mesh, AnimIndex, AnimationTimeTicks, Mesh->BoneTransformsHierarchy->Children[1],
    //                  Identity());
}

internal loaded_model *LoadModel(memory_arena *WorldArena, char *FileName)
{
    loaded_model *Result = PushStruct(WorldArena, loaded_model);

    FILE *In;
    if((In = fopen(FileName, "rb")) == NULL)
    {
        InvalidCodePath;
    }

    // модель
    Result->Name = ReadStringFromFile(WorldArena, In); // имя модели
    fread(&Result->MeshesCount, sizeof(u32), 1, In);   // число мешей

    // меши
    Result->Meshes = PushArray(WorldArena, Result->MeshesCount, single_mesh);
    for(u32 i = 0; i < Result->MeshesCount; i++)
    {
        single_mesh *Mesh = &Result->Meshes[i];

        Mesh->Name = ReadStringFromFile(WorldArena, In); // имя меша

        fread(&Mesh->VerticesCount, sizeof(u32), 1, In); // число вершин меша

        fread(&Mesh->IndicesCount, sizeof(u32), 1, In); // число индексов меша

        // позиции вершин
        Mesh->Positions = PushArray(WorldArena, Mesh->VerticesCount, v3);
        fread(Mesh->Positions, sizeof(v3) * Mesh->VerticesCount, 1, In);

        // нормали
        Mesh->Normals = PushArray(WorldArena, Mesh->VerticesCount, v3);
        fread(Mesh->Normals, sizeof(v3) * Mesh->VerticesCount, 1, In);

        // текстурные координаты
        Mesh->TexCoords = PushArray(WorldArena, Mesh->VerticesCount, v2);
        fread(Mesh->TexCoords, sizeof(v2) * Mesh->VerticesCount, 1, In);

        // касательные и бикасательные для маппинга нормалей
        Mesh->Tangents = PushArray(WorldArena, Mesh->VerticesCount, v3);
        fread(Mesh->Tangents, sizeof(v3) * Mesh->VerticesCount, 1, In);
        // Mesh->Bitangents = PushArray(WorldArena, Mesh->VerticesCount * 3, r32);
        // fread(Mesh->Bitangents, sizeof(r32) * Mesh->VerticesCount * 3, 1, In);

        // индексы
        Mesh->Indices = PushArray(WorldArena, Mesh->VerticesCount, u32);
        fread(Mesh->Indices, sizeof(u32) * Mesh->IndicesCount, 1, In);

        // материал
        fread(&Mesh->WithMaterial, sizeof(b32), 1, In);
        if(Mesh->WithMaterial)
        {
            Mesh->Material.Name = ReadStringFromFile(WorldArena, In);
            fread(&Mesh->Material.Ambient, sizeof(v4), 1, In);
            fread(&Mesh->Material.Diffuse, sizeof(v4), 1, In);
            fread(&Mesh->Material.Specular, sizeof(v4), 1, In);
            fread(&Mesh->Material.Emission, sizeof(v4), 1, In);
            fread(&Mesh->Material.Shininess, sizeof(r32), 1, In);

            fread(&Mesh->Material.WithTexture, sizeof(b32), 1, In);
            if(Mesh->Material.WithTexture)
            {
                Mesh->Material.TextureName = ReadStringFromFile(WorldArena, In);
                Mesh->Material.Texture = LoadTexture(&Mesh->Material.TextureName);
            }
        }

        // анимации
        fread(&Mesh->WithAnimations, sizeof(b32), 1, In);
        if(Mesh->WithAnimations)
        {
            // смещения костей относительно друг друга
            fread(&Mesh->BonesCount, sizeof(r32), 1, In);

            Mesh->BoneNames = PushArray(WorldArena, Mesh->BonesCount, string);
            Mesh->BoneOffsets = PushArray(WorldArena, Mesh->BonesCount, m4x4);
            for(u32 j = 0; j < Mesh->BonesCount; j++)
            {
                Mesh->BoneNames[j] = ReadStringFromFile(WorldArena, In);
                fread(&Mesh->BoneOffsets[j], sizeof(m4x4), 1, In);
            }

            Mesh->BoneIDs = PushArray(WorldArena, Mesh->VerticesCount * 4, u32);
            Mesh->Weights = PushArray(WorldArena, Mesh->VerticesCount * 4, r32);

            fread(Mesh->BoneIDs, sizeof(u32) * Mesh->VerticesCount * 4, 1, In);
            fread(Mesh->Weights, sizeof(r32) * Mesh->VerticesCount * 4, 1, In);

            // иерархия с преобразованиями костей
            Mesh->BoneTransformsHierarchy = PushStruct(WorldArena, node);
            ProcessBoneTransformsHierarchy(WorldArena, Mesh->BoneTransformsHierarchy, In);

            // маркеры времени у анимаций
            fread(&Mesh->AnimationsCount, sizeof(u32), 1, In);
            Mesh->Animations = PushArray(WorldArena, Mesh->AnimationsCount, animation);
            for(u32 j = 0; j < Mesh->AnimationsCount; j++)
            {
                animation *Animation = &Mesh->Animations[j];

                Animation->Name = ReadStringFromFile(WorldArena, In);

                fread(&Animation->Duration, sizeof(r64), 1, In);
                fread(&Animation->TicksPerSeconds, sizeof(r64), 1, In);

                fread(&Animation->ChannelsCount, sizeof(u32), 1, In);
                Animation->Channels = PushArray(WorldArena, Animation->ChannelsCount, node_anim);
                for(u32 k = 0; k < Animation->ChannelsCount; k++)
                {
                    node_anim *AnimChannel = &Animation->Channels[k];

                    AnimChannel->Name = ReadStringFromFile(WorldArena, In);

                    fread(&AnimChannel->PositionKeysCount, sizeof(u32), 1, In);
                    AnimChannel->PositionKeys = PushArray(WorldArena, AnimChannel->PositionKeysCount, position_key);
                    for(u32 t = 0; t < AnimChannel->PositionKeysCount; t++)
                    {
                        fread(&AnimChannel->PositionKeys[t].Time, sizeof(r64), 1, In);
                        fread(&AnimChannel->PositionKeys[t].Value, sizeof(v3), 1, In);
                    }

                    fread(&AnimChannel->RotationKeysCount, sizeof(u32), 1, In);
                    AnimChannel->RotationKeys = PushArray(WorldArena, AnimChannel->RotationKeysCount, rotation_key);
                    for(u32 t = 0; t < AnimChannel->RotationKeysCount; t++)
                    {
                        fread(&AnimChannel->RotationKeys[t].Time, sizeof(r64), 1, In);
                        fread(&AnimChannel->RotationKeys[t].Value, sizeof(v4), 1, In);
                    }

                    fread(&AnimChannel->ScalingKeysCount, sizeof(u32), 1, In);
                    AnimChannel->ScalingKeys = PushArray(WorldArena, AnimChannel->ScalingKeysCount, scaling_key);
                    for(u32 t = 0; t < AnimChannel->ScalingKeysCount; t++)
                    {
                        fread(&AnimChannel->ScalingKeys[t].Time, sizeof(r64), 1, In);
                        fread(&AnimChannel->ScalingKeys[t].Value, sizeof(v3), 1, In);
                    }
                }
            }

            Mesh->WithSceneTransform = false;
            Mesh->FinalTransforms = PushArray(WorldArena, Mesh->BonesCount, m4x4);
        }
    }

    fclose(In);

    return (Result);
}

//
// NOTE(me): Terrain
//
internal void CalcNormal(v3 A, v3 B, v3 C, v3 *Result)
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

internal b32 IsPosOnTerrain(r32 x, r32 y)
{
    b32 Result;

    Result = ((x >= 0) &&    //
              (x < TMapW) && //
              (y >= 0) &&    //
              (y < TMapH));

    return (Result);
}

internal void CreateHill(v3 *Positions, s32 PosX, s32 PosY, s32 PosZ, s32 Radius)
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

//
// NOTE(me): Grass
//
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

//
// NOTE(me): Other
//
internal loaded_model *CreateTexturedSquareModel(memory_arena *WorldArena, char *TextureName)
{
    loaded_model *Result = PushStruct(WorldArena, loaded_model);

    Result->Name = PushString(WorldArena, "TexturedSquareModel");

    Result->MeshesCount = 1;

    Result->Meshes = PushArray(WorldArena, Result->MeshesCount, single_mesh);

    single_mesh *Mesh = &Result->Meshes[0];

    Mesh->Name = PushString(WorldArena, TextureName);

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