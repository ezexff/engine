string ReadStringFromFile(memory_arena *WorldArena, FILE *In)
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

node_anim *FindNodeAnim(animation *CurrentAnimation, string NodeName)
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

u32 FindScaling(r32 AnimationTimeTicks, node_anim *NodeAnim)
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

v3 CalcInterpolatedScaling(r32 AnimationTimeTicks, node_anim *NodeAnim)
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

u32 FindRotation(r32 AnimationTimeTicks, node_anim *NodeAnim)
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

v4 CalcInterpolatedRotation(r32 AnimationTimeTicks, node_anim *NodeAnim)
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

u32 FindPosition(r32 AnimationTimeTicks, node_anim *NodeAnim)
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

v3 CalcInterpolatedPosition(r32 AnimationTimeTicks, node_anim *NodeAnim)
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

void ReadNodeHierarchy(single_mesh *Mesh, u32 AnimIndex, r32 AnimationTimeTicks, node *Node, m4x4 ParentTransform)
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

void GetBoneTransforms(single_mesh *Mesh, u32 AnimIndex, r32 TimeInSeconds)
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

internal u32 LoadTexture(string *FileName)
{
    u32 Result;

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

    int32 Width, Height, NrChannels;
    unsigned char *data = stbi_load(FullPath, &Width, &Height, &NrChannels, 0);
    if(data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Width, Height, 0, //
                     NrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    }
    else
    {
        // Bitmap texture failed to load at path: Path
        stbi_image_free(data);
        //InvalidCodePath;

        char TempBuffer[256];
        _snprintf_s(TempBuffer, sizeof(TempBuffer), "[error] Texture loading: %s\n", FullPath);
        OutputDebugStringA(TempBuffer);
        Log.AddLog(TempBuffer);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return (Result);
}

void ProcessBoneTransformsHierarchy(memory_arena *WorldArena, node *Node, FILE *In)
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

loaded_model *LoadModel(memory_arena *WorldArena, char *FileName)
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