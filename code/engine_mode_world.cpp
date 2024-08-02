internal v3
CalcNormal(v3 P1, v3 P2, v3 P3)
{
#if 0
    v3 Result = {};
    // Vec1 = B - A;
    v3 Vec1;
    Vec1.x = A.x - B.x;
    Vec1.y = A.y - B.y;
    Vec1.z = A.z - B.z;
    
    // Vec2 = C - A
    v3 Vec2;
    Vec2.x = B.x - C.x;
    Vec2.y = B.y - C.y;
    Vec2.z = B.z - C.z;
    
    // N = (B - A) * (C - A)
    Result.x = (Vec1.y * Vec2.z - Vec1.z * Vec2.y);
    Result.y = (Vec1.z * Vec2.x - Vec1.x * Vec2.z);
    Result.z = (Vec1.x * Vec2.y - Vec1.y * Vec2.x);
#else
    // Vec1 = B - A;
    /*v3 Vec1;
    Vec1.x = B.x - A.x;
    Vec1.y = B.y - A.y;
    Vec1.z = B.z - A.z;
    
    // Vec2 = C - A
    v3 Vec2;
    Vec2.x = C.x - A.x;
    Vec2.y = C.y - A.y;
    Vec2.z = C.z - A.z;
    
    // N = (B - A) * (C - A)
    Result.x = (Vec1.y * Vec2.z - Vec1.z * Vec2.y);
    Result.y = (Vec1.z * Vec2.x - Vec1.x * Vec2.z);
    Result.z = (Vec1.x * Vec2.y - Vec1.y * Vec2.x);*/
#endif
    
    /*r32 Magnitude = SquareRoot(Square(Result.x) + Square(Result.y) + Square(Result.z));
    Result.x /= Magnitude;
    Result.y /= Magnitude;
    Result.z /= Magnitude;*/
    /*
    So for a triangle p1, p2, p3, if the vector U = p2 - p1 and the vector
V = p3 - p1 then the normal N = U X V and can be calculated by:
    Nx = UyVz - UzVy
        Ny = UzVx - UxVz
        Nz = UxVy - UyVx
*/
    
    v3 U = P2 - P1;
    v3 V = P3 - P1;
    v3 Result;
    Result.x = U.y * V.z - U.z * V.y;
    Result.y = U.z * V.x - U.x * V.z;
    Result.z = U.x * V.y - U.y * V.x;
    Result = Normalize(Result);
    
    return(Result);
}

internal void
CreateTerrainModel(memory_arena *WorldArena, loaded_model Model)
{
    Model.MeshesCount = 1;
    Model.Meshes = PushArray(WorldArena, Model.MeshesCount, mesh);
    
    mesh *Mesh = &Model.Meshes[0];
    
    u32 TMapW = 1;
    u32 TMapH = 1;
    Mesh->VertCount = TMapW * TMapH;
    Mesh->Positions = PushArray(WorldArena, Mesh->VertCount, v3);
    Mesh->TexCoords = PushArray(WorldArena, Mesh->VertCount, v2);
    
    u32 VertCountTmp = 0;
    for(u32 i = 0; i < TMapW; i++)
    {
        for(u32 j = 0; j < TMapH; j++)
        {
            Mesh->Positions[VertCountTmp] = V3((r32)i, (r32)j, (rand() % 10) * 0.02f);
            Mesh->TexCoords[VertCountTmp] = V2((r32)i, (r32)j);
            VertCountTmp++;
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
}

internal void
FillGroundChunk(tran_state *TranState, game_state *GameState, renderer_frame *Frame,
                ground_buffer *GroundBuffer,
                world_position *ChunkP)
{
#if 0
    GroundBuffer->P = *ChunkP;
    
    r32 Width = GameState->World->ChunkDimInMeters.x;
    r32 Height = GameState->World->ChunkDimInMeters.y;
    
    single_mesh *Mesh = &GroundBuffer->TerrainModel->Meshes[0];
    
    for(int32 ChunkOffsetY = -1; //
        ChunkOffsetY <= 1;       //
        ++ChunkOffsetY)
    {
        for(int32 ChunkOffsetX = -1; //
            ChunkOffsetX <= 1;       //
            ++ChunkOffsetX)
        {
            int32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
            int32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
            int32 ChunkZ = ChunkP->ChunkZ;
            
            // TODO(casey): Make random number generation more systemic
            // TODO(casey): Look into wang hashing or some other spatial seed generation "thing"!
            random_series Series = RandomSeed(139 * ChunkX + 593 * ChunkY + 329 * ChunkZ);
            
            // v2 Center = V2(ChunkOffsetX * Width + HalfDim.x, ChunkOffsetY * Height + HalfDim.y);
            v2 Center = V2(ChunkOffsetX * Width, ChunkOffsetY * Height);
            
            r32 MaxTerrainHeight = 0.2f;
            
            int VertexIndex = 0;
            for(u32 Y = 0;      //
                Y < Height + 1; //
                Y++)
            {
                for(u32 X = 0;     //
                    X < Width + 1; //
                    X++, VertexIndex++)
                {
                    // r32 RandomZ2 = (r32)(rand() % 10) * 0.02f;
                    
                    //
                    {
                        if((X == 0) || (Y == 0) || (X == Width) || (Y == Height))
                        {
                            // if((ChunkOffsetX == 0) && (ChunkOffsetY == 0))
                            {
                                // r32 RandomZ1 = RandomUnilateral(&Series) * MaxTerrainHeight;
                                // Mesh->Positions[VertexIndex] = V3((r32)X, (r32)Y, RandomZ1);
                            }
                        }
                        else
                        {
                            r32 RandomZ1 = RandomUnilateral(&Series) * MaxTerrainHeight;
                            Mesh->Positions[VertexIndex] = V3((r32)X, (r32)Y, RandomZ1);
                        }
                    }
                }
            }
        }
    }
#else
    //temporary_memory GroundMemory = BeginTemporaryMemory(&TranState->TranArena);
    GroundBuffer->P = *ChunkP;
    GroundBuffer->IsFilled = true;
    if(IsValid(GroundBuffer->P))
    {
        s32 TileX = GroundBuffer->P.ChunkX * Frame->TileCount;
        s32 TileY = GroundBuffer->P.ChunkY * Frame->TileCount;
        s32 TileZ = GroundBuffer->P.ChunkZ * Frame->TileCount;
        
        u32 TmpPosCount3 = 0;
        for(u32 Y = 0;
            Y <= Frame->TileCount;
            ++Y)
        {
            for(u32 X = 0;
                X <= Frame->TileCount;
                ++X)
            {
                GroundBuffer->Vertices[TmpPosCount3].Position.z = 0;
                TmpPosCount3++;
            }
        }
        
        // NOTE(ezexff): Calc pit and hill
        {
            s32 HillTileX = 8;
            s32 HillTileY = 8;
            s32 HillZ = 2;
            s32 HillRadius = 8;
            if((ChunkP->ChunkX == 0) && (ChunkP->ChunkY == 0) & (ChunkP->ChunkZ == 0))
            {
                // TODO(ezexff): Add hill or pit
                for(s32 Y = HillTileY - HillRadius;
                    Y <= HillTileY + HillRadius;
                    ++Y)
                {
                    for(s32 X = HillTileX - HillRadius;
                        X <= HillTileX + HillRadius;
                        ++X)
                    {
                        s32 TmpIndex = Y * (Frame->TileCount + 1) + X;
                        Assert(TmpIndex >= 0);
                        Assert(TmpIndex < (s32)Frame->ChunkVertexCount);
                        r32 t1 = (r32)(HillTileX - X);
                        r32 t2 = (r32)(HillTileY - Y);
                        r32 Length = SquareRoot(Square(t1) + Square(t2));
                        if(Length < HillRadius)
                        {
                            Length = Length / HillRadius * (r32)Pi32_2;
                            GroundBuffer->Vertices[TmpIndex].Position.z += Cos(Length) * HillZ;
                        }
                    }
                }
            }
            
            HillZ = -2;
            if((ChunkP->ChunkX == 1) && (ChunkP->ChunkY == 0) & (ChunkP->ChunkZ == 0))
            {
                // TODO(ezexff): Add hill or pit
                for(s32 Y = HillTileY - HillRadius;
                    Y <= HillTileY + HillRadius;
                    ++Y)
                {
                    for(s32 X = HillTileX - HillRadius;
                        X <= HillTileX + HillRadius;
                        ++X)
                    {
                        s32 TmpIndex = Y * (Frame->TileCount + 1) + X;
                        Assert(TmpIndex >= 0);
                        Assert(TmpIndex < (s32)Frame->ChunkVertexCount);
                        r32 t1 = (r32)(HillTileX - X);
                        r32 t2 = (r32)(HillTileY - Y);
                        r32 Length = SquareRoot(Square(t1) + Square(t2));
                        if(Length < HillRadius)
                        {
                            Length = Length / HillRadius * (r32)Pi32_2;
                            GroundBuffer->Vertices[TmpIndex].Position.z += Cos(Length) * HillZ;
                        }
                    }
                }
            }
        }
        
        // NOTE(ezexff): Calc z
        u32 TmpPosCount = 0;
        for(u32 Y = 0;
            Y <= Frame->TileCount;
            ++Y)
        {
            for(u32 X = 0;
                X <= Frame->TileCount;
                ++X)
            {
                /* 
                                u32 SeedValueA = 139 * (TileX + X) + 593 * (TileY + Y) + 329 * TileZ;
                                random_series SeriesA = RandomSeed(SeedValueA);
                                r32 Z = RandomUnilateral(&SeriesA);
                                Z *= Frame->MaxTerrainHeight;
                                GroundBuffer->Vertices[TmpPosCount].Position.z = Z;
                 */
                
                v3 A = {};
                A.x = X * Frame->TileWidth;
                A.y = Y * Frame->TileWidth;
                u32 SeedValueA = 139 * (TileX + X) + 593 * (TileY + Y) + 329 * TileZ;
                random_series SeriesA = RandomSeed(SeedValueA);
                r32 AZ = RandomUnilateral(&SeriesA);
                AZ *= Frame->MaxTerrainHeight;
                A.z = AZ;
                GroundBuffer->Vertices[TmpPosCount].Position.z += AZ;
                
                
                // NOTE(ezexff): Normal
                v3 B = {};
                B.x = (X + 1) * Frame->TileWidth;
                B.y = A.y;
                u32 SeedValueB = 139 * (TileX + (X + 1)) + 593 * (TileY + Y) + 329 * TileZ;
                random_series SeriesB = RandomSeed(SeedValueB);
                r32 BZ = RandomUnilateral(&SeriesB);
                BZ *= Frame->MaxTerrainHeight;
                B.z = BZ;
                
                v3 C = {};
                C.x = A.x;;
                C.y = (Y + 1) * Frame->TileHeight;
                u32 SeedValueC = 139 * (TileX + X) + 593 * (TileY + (Y + 1)) + 329 * TileZ;
                random_series SeriesC = RandomSeed(SeedValueC);
                r32 CZ = RandomUnilateral(&SeriesC);
                CZ *= Frame->MaxTerrainHeight;
                
                //Frame->TerrainVertices[CurrentVertex].Normal = CalcNormal(A, B, C);
                GroundBuffer->Vertices[TmpPosCount].Normal = CalcNormal(A, B, C);
                
                TmpPosCount++;
            }
        }
    }
    
    //Log->Add("ChunkP = (%d,%d)\n", ChunkP->ChunkX, ChunkP->ChunkY);
    
    /*s32 ChunkX = ChunkP->ChunkX;
    s32 ChunkY = ChunkP->ChunkY;
    s32 ChunkZ = ChunkP->ChunkZ;*/
    
    // TODO(casey): Make random number generation more systemic
    // TODO(casey): Look into wang hashing or some other spatial seed generation "thing"!
    
    /*r32 MaxTerrainHeight = 0.2f;
    random_series Series = RandomSeed(139 * ChunkX + 593 * ChunkY + 329 * ChunkZ);
    GroundBuffer->RandomZ = RandomUnilateral(&Series) * MaxTerrainHeight;*/
    
    /*if(ChunkX == 1 && ChunkY == 1)
    {
        for(u32 Index = 0;
            Index < 6;
            ++Index)
        {
            random_series Series = RandomSeed(139 * ChunkX + 593 * ChunkY + 329 * ChunkZ + 222 * Index);
            r32 RandomZ = RandomUnilateral(&Series) * MaxTerrainHeight;
            Log->Add("Random[%d] = (%.3f)\n", Index, RandomZ);
        }
    }*/
    
    /*loaded_texture *Buffer = &GroundBuffer->DrawBuffer;
    r32 Width = (r32)Buffer->Width;
    r32 Height = (r32)Buffer->Height;
    v2 HalfDim = 0.5f * V2(Width, Height);
    
    render_group *RenderGroup = AllocateRenderGroup(&TranState->TranArena, Megabytes(4), //
                                                    0, 0, 0, true);
    
    glBindFramebuffer(GL_FRAMEBUFFER, Buffer->FBO);
    
    // Create Render Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Buffer->ID);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Buffer->Width, Buffer->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Buffer->Width, Buffer->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Buffer->ID, 0);
    
    //  Create Depth Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Buffer->DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Buffer->Width, Buffer->Height, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Buffer->DepthTexture, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    for(int32 ChunkOffsetY = -1; //
        ChunkOffsetY <= 1;       //
        ++ChunkOffsetY)
    {
        for(int32 ChunkOffsetX = -1; //
            ChunkOffsetX <= 1;       //
            ++ChunkOffsetX)
        {
            int32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
            int32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
            int32 ChunkZ = ChunkP->ChunkZ;
            
            // TODO(casey): Make random number generation more systemic
            // TODO(casey): Look into wang hashing or some other spatial seed generation "thing"!
            random_series Series = RandomSeed(139 * ChunkX + 593 * ChunkY + 329 * ChunkZ);
            
            // v2 Center = V2(ChunkOffsetX * Width + HalfDim.x, ChunkOffsetY * Height + HalfDim.y);
            v2 Center = V2(ChunkOffsetX * Width, ChunkOffsetY * Height);
            
            PushTexture(RenderGroup, V3(Center, 0), V2(Width, Height), GameState->TestTexture1.ID, //
                        true, 2.0f);
            
            for(uint32 GrassIndex = 0; //
                GrassIndex < 10;       //
                ++GrassIndex)
            {
                // loaded_bitmap *Stamp;
                v4 Color;
                v2 Dim;
                if(RandomChoice(&Series, 2))
                {
                    // Stamp = GameState->Grass + RandomChoice(&Series, ArrayCount(GameState->Grass));
                    Color = V4(1, 0, 0, 1);
                    Dim = V2(64, 64);
                }
                else
                {
                    // Stamp = GameState->Stone + RandomChoice(&Series, ArrayCount(GameState->Stone));
                    Color = V4(1, 1, 0, 1);
                    Dim = V2(32, 32);
                }
                
                v2 RandomValue = V2(RandomBilateral(&Series), RandomBilateral(&Series));
                v2 P = Center + Hadamard(HalfDim, RandomValue);
                
                PushRect(RenderGroup, V3(P, 0), Dim, Color);
            }
        }
    }*/
    
    /*for(int32 ChunkOffsetY = -1; //
        ChunkOffsetY <= 1;       //
        ++ChunkOffsetY)
    {
        for(int32 ChunkOffsetX = -1; //
            ChunkOffsetX <= 1        //
            ;
            ++ChunkOffsetX)
        {
            int32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
            int32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
            int32 ChunkZ = ChunkP->ChunkZ;
    
            // TODO(casey): Make random number generation more systemic
            // TODO(casey): Look into wang hashing or some other spatial seed generation "thing"!
            //random_series Series = RandomSeed(139 * ChunkX + 593 * ChunkY + 329 * ChunkZ);
    
            v2 Center = V2(ChunkOffsetX * Width, ChunkOffsetY * Height);
    
            for(uint32 GrassIndex = 0; GrassIndex < 50; ++GrassIndex)
            {
                loaded_bitmap *Stamp = GameState->Tuft + RandomChoice(&Series, ArrayCount(GameState->Tuft));
    
                v2 BitmapCenter = 0.5f * V2i(Stamp->Width, Stamp->Height);
                v2 Offset = {Width * RandomUnilateral(&Series), Height * RandomUnilateral(&Series)};
                v2 P = Center + Offset - BitmapCenter;
    
                PushBitmap(RenderGroup, Stamp, V3(P, 0.0f));
            }
        }
    }*/
    
    //RenderGroupToOutput(RenderGroup, Buffer);
    //EndTemporaryMemory(GroundMemory);
#endif
}

// NOTE(ezexff): Collision rules
internal void
AddCollisionRule(game_state *GameState, u32 StorageIndexA, u32 StorageIndexB, b32 CanCollide)
{
    // TODO(casey): Collapse this with ShouldCollide
    if(StorageIndexA > StorageIndexB)
    {
        u32 Temp = StorageIndexA;
        StorageIndexA = StorageIndexB;
        StorageIndexB = Temp;
    }
    
    // TODO(casey): BETTER HASH FUNCTION
    pairwise_collision_rule *Found = 0;
    u32 HashBucket = StorageIndexA & (ArrayCount(GameState->CollisionRuleHash) - 1);
    for(pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket];
        Rule;
        Rule = Rule->NextInHash)
    {
        if((Rule->StorageIndexA == StorageIndexA) && (Rule->StorageIndexB == StorageIndexB))
        {
            Found = Rule;
            break;
        }
    }
    
    if(!Found)
    {
        Found = GameState->FirstFreeCollisionRule;
        if(Found)
        {
            GameState->FirstFreeCollisionRule = Found->NextInHash;
        }
        else
        {
            Found = PushStruct(&GameState->ConstArena, pairwise_collision_rule);
        }
        
        Found->NextInHash = GameState->CollisionRuleHash[HashBucket];
        GameState->CollisionRuleHash[HashBucket] = Found;
    }
    
    if(Found)
    {
        Found->StorageIndexA = StorageIndexA;
        Found->StorageIndexB = StorageIndexB;
        Found->CanCollide = CanCollide;
    }
}

internal void
ClearCollisionRulesFor(game_state *GameState, u32 StorageIndex)
{
    // TODO(casey): Need to make a better data structure that allows
    // removal of collision rules without searching the entire table
    // NOTE(casey): One way to make removal easy would be to always
    // add _both_ orders of the pairs of storage indices to the
    // hash table, so no matter which position the entity is in,
    // you can always find it.  Then, when you do your first pass
    // through for removal, you just remember the original top
    // of the free list, and when you're done, do a pass through all
    // the new things on the free list, and remove the reverse of
    // those pairs.
    for(u32 HashBucket = 0;
        HashBucket < ArrayCount(GameState->CollisionRuleHash);
        ++HashBucket)
    {
        for(pairwise_collision_rule **Rule = &GameState->CollisionRuleHash[HashBucket];
            *Rule;)
        {
            if(((*Rule)->StorageIndexA == StorageIndex) || ((*Rule)->StorageIndexB == StorageIndex))
            {
                pairwise_collision_rule *RemovedRule = *Rule;
                *Rule = (*Rule)->NextInHash;
                
                RemovedRule->NextInHash = GameState->FirstFreeCollisionRule;
                GameState->FirstFreeCollisionRule = RemovedRule;
            }
            else
            {
                Rule = &(*Rule)->NextInHash;
            }
        }
    }
}

sim_entity_collision_volume_group *
MakeSimpleGroundedCollision(game_state *GameState, r32 DimX, r32 DimY, r32 DimZ)
{
    // TODO(casey): NOT WORLD ARENA!  Change to using the fundamental types arena, etc.
    sim_entity_collision_volume_group *Group = PushStruct(&GameState->ConstArena, sim_entity_collision_volume_group);
    Group->VolumeCount = 1;
    Group->Volumes = PushArray(&GameState->ConstArena, Group->VolumeCount, sim_entity_collision_volume);
    Group->TotalVolume.OffsetP = V3(0, 0, 0.5f * DimZ);
    //Group->TotalVolume.OffsetP = V3(0, 0, 0);
    Group->TotalVolume.Dim = V3(DimX, DimY, DimZ);
    Group->Volumes[0] = Group->TotalVolume;
    
    return (Group);
}

sim_entity_collision_volume_group *
MakeNullCollision(game_state *GameState)
{
    // TODO(casey): NOT WORLD ARENA!  Change to using the fundamental types arena, etc.
    sim_entity_collision_volume_group *Group = PushStruct(&GameState->ConstArena, sim_entity_collision_volume_group);
    Group->VolumeCount = 0;
    Group->Volumes = 0;
    Group->TotalVolume.OffsetP = V3(0, 0, 0);
    // TODO(casey): Should this be negative?
    Group->TotalVolume.Dim = V3(0, 0, 0);
    
    return (Group);
}

//~ NOTE(ezexff): Add objects into World
inline world_position
ChunkPositionFromTilePosition(world *World, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ,
                              v3 AdditionalOffset = V3(0, 0, 0))
{
    world_position BasePos = {};
    
    r32 TileSideInMeters = 1.4f;
    r32 TileDepthInMeters = 3.0f;
    
    v3 TileDim = V3(TileSideInMeters, TileSideInMeters, TileDepthInMeters);
    v3 Offset = Hadamard(TileDim, V3((r32)AbsTileX, (r32)AbsTileY, (r32)AbsTileZ));
    world_position Result = MapIntoChunkSpace(World, BasePos, AdditionalOffset + Offset);
    
    Assert(IsCanonical(World, Result.Offset_));
    
    return (Result);
}

struct add_low_entity_result
{
    low_entity *Low;
    u32 LowIndex;
};
internal add_low_entity_result
AddLowEntity(game_state *GameState, entity_type Type, world_position P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
    u32 EntityIndex = GameState->LowEntityCount++;
    
    low_entity *EntityLow = GameState->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Sim.Type = Type;
    EntityLow->Sim.Collision = GameState->NullCollision;
    EntityLow->P = NullPosition();
    
    ChangeEntityLocation(&GameState->ConstArena, GameState->World, EntityIndex, EntityLow, P);
    
    add_low_entity_result Result;
    Result.Low = EntityLow;
    Result.LowIndex = EntityIndex;
    
    // TODO(casey): Do we need to have a begin/end paradigm for adding
    // entities so that they can be brought into the high set when they
    // are added and are in the camera region?
    
    return (Result);
}

internal add_low_entity_result 
AddGroundedEntity(game_state *GameState, entity_type Type, world_position P,
                  sim_entity_collision_volume_group *Collision)
{
    add_low_entity_result Entity = AddLowEntity(GameState, Type, P);
    Entity.Low->Sim.Collision = Collision;
    return (Entity);
}

internal add_low_entity_result 
AddStandardRoom(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Space, P, GameState->StandardRoomCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Traversable);
    
    return (Entity);
}

internal add_low_entity_result 
AddWall(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Wall, P, GameState->WallCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    
    return (Entity);
}

internal add_low_entity_result 
AddStair(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Stairwell, P, GameState->StairCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    Entity.Low->Sim.WalkableDim = Entity.Low->Sim.Collision->TotalVolume.Dim.xy;
    Entity.Low->Sim.WalkableHeight = GameState->TypicalFloorHeight;
    
    return (Entity);
}

internal void 
InitHitPoints(low_entity *EntityLow, u32 HitPointCount)
{
    Assert(HitPointCount <= ArrayCount(EntityLow->Sim.HitPoint));
    EntityLow->Sim.HitPointMax = HitPointCount;
    for(u32 HitPointIndex = 0;
        HitPointIndex < EntityLow->Sim.HitPointMax;
        ++HitPointIndex)
    {
        hit_point *HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
        HitPoint->Flags = 0;
        HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
    }
}

internal add_low_entity_result 
AddSword(game_state *GameState)
{
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());
    Entity.Low->Sim.Collision = GameState->SwordCollision;
    
    AddFlags(&Entity.Low->Sim, EntityFlag_Moveable);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    
    return (Entity);
}

internal add_low_entity_result 
AddPlayer(game_state *GameState)
{
    world_position P = GameState->CameraP;
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Hero, P, GameState->PlayerCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);
    
    InitHitPoints(Entity.Low, 3);
    
    add_low_entity_result Sword = AddSword(GameState);
    Entity.Low->Sim.Sword.Index = Sword.LowIndex;
    
    if(GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = Entity.LowIndex;
    }
    
    return (Entity);
}

internal add_low_entity_result
AddMonstar(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Monstar, P, GameState->MonstarCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);
    
    InitHitPoints(Entity.Low, 3);
    
    return (Entity);
}

internal add_low_entity_result 
AddFamiliar(game_state *GameState, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Familiar, P, GameState->FamiliarCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);
    
    return (Entity);
}

// NOTE(ezexff): Main cycle
internal void
UpdateAndRenderWorld(game_memory *Memory, game_input *Input)
{
#if ENGINE_INTERNAL
    imgui *ImGuiHandle = &Memory->Frame.ImGuiHandle;
#endif
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    game_assets *Assets = TranState->Assets;
    mode_world *ModeWorld = &GameState->ModeWorld;
    //camera *Camera = &ModeWorld->Camera;
    renderer_frame *Frame = &Memory->Frame;
    world *World = GameState->World;
    
    if(!ModeWorld->IsInitialized)
    {
        //~ NOTE(ezexff): World creation
        {
            // NOTE(casey): Reserve entity slot 0 for the null entity
            AddLowEntity(GameState, EntityType_Null, NullPosition());
            
            v3 WorldChunkDimInMeters = V3((r32)GameState->GroundBufferWidth,
                                          (r32)GameState->GroundBufferHeight,
                                          GameState->TypicalFloorHeight);
            
            GameState->World = PushStruct(&GameState->ConstArena, world);
            World = GameState->World;
            InitializeWorld(World, WorldChunkDimInMeters);
            
            
            u32 TilesPerWidth = 17;
            u32 TilesPerHeight = 9;
            r32 TileSideInMeters = 1.4f;
            r32 TileDepthInMeters = GameState->TypicalFloorHeight;
            
            GameState->NullCollision = MakeNullCollision(GameState);
            //GameState->SwordCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.1f);
            GameState->SwordCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 1.2f);
            GameState->StairCollision = MakeSimpleGroundedCollision(GameState,
                                                                    TileSideInMeters,
                                                                    2.0f * TileSideInMeters,
                                                                    1.1f * TileDepthInMeters);
            GameState->PlayerCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 1.2f);
            GameState->MonstarCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
            GameState->FamiliarCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
            GameState->WallCollision = MakeSimpleGroundedCollision(GameState,
                                                                   TileSideInMeters,
                                                                   TileSideInMeters,
                                                                   TileDepthInMeters);
            /*GameState->StandardRoomCollision = MakeSimpleGroundedCollision(GameState,
                                                                           TilesPerWidth * TileSideInMeters,
                                                                           TilesPerHeight * TileSideInMeters,
                                                                           0.9f * TileDepthInMeters);*/
            
            GameState->StandardRoomCollision = MakeSimpleGroundedCollision(GameState,
                                                                           2 * TilesPerWidth * TileSideInMeters,
                                                                           2 * TilesPerHeight * TileSideInMeters,
                                                                           0.9f * TileDepthInMeters);
            
            u32 ScreenBaseX = 0;
            u32 ScreenBaseY = 0;
            u32 ScreenBaseZ = 0;
            u32 ScreenX = ScreenBaseX;
            u32 ScreenY = ScreenBaseY;
            //u32 AbsTileX = ScreenX * TilesPerWidth + 1;  // TileX=1
            //u32 AbsTileY = ScreenY * TilesPerHeight + 1; // TileY=2
            u32 AbsTileZ = ScreenBaseZ;
            
            AddStandardRoom(GameState,
                            ScreenX * TilesPerWidth + TilesPerWidth / 2,
                            ScreenY * TilesPerHeight + TilesPerHeight / 2,
                            AbsTileZ);
            // left
            for(u32 Index = 0;
                Index < TilesPerHeight;
                Index++)
            {
                AddWall(GameState, 0, Index, AbsTileZ);
            }
            // top and bot
            for(u32 Index = 1;
                Index < TilesPerWidth;
                Index++)
            {
                AddWall(GameState, Index, TilesPerHeight - 1, AbsTileZ);
                AddWall(GameState, Index, 0, AbsTileZ);
            }
            
            for(u32 RoomIndex = 0;
                RoomIndex < 100;
                RoomIndex++)
            {
                ScreenX++;
                
                AddStandardRoom(GameState,
                                ScreenX * TilesPerWidth + TilesPerWidth / 2,
                                ScreenY * TilesPerHeight + TilesPerHeight / 2,
                                AbsTileZ);
            }
            
            /*ScreenY = 1;
            ScreenX = 1;
            
            AddStandardRoom(GameState,
                            ScreenX * TilesPerWidth + TilesPerWidth / 2,
                            ScreenY * TilesPerHeight + TilesPerHeight / 2,
                            AbsTileZ);*/
            
            world_position NewCameraP = {};
            u32 CameraTileX = ScreenBaseX * TilesPerWidth + 17 / 2;
            u32 CameraTileY = ScreenBaseY * TilesPerHeight + 9 / 2;
            u32 CameraTileZ = ScreenBaseZ;
            NewCameraP = ChunkPositionFromTilePosition(World, CameraTileX, CameraTileY, CameraTileZ);
            GameState->CameraP = NewCameraP;
            GameState->CameraPitch = 0.0f;
            GameState->CameraYaw = 0.0f;
            GameState->CameraRoll = 0.0f;
            
            AddMonstar(GameState, CameraTileX - 3, CameraTileY + 2, CameraTileZ);
            AddMonstar(GameState, CameraTileX, CameraTileY + 2, CameraTileZ);
            AddMonstar(GameState, CameraTileX + 3, CameraTileY + 2, CameraTileZ);
        }
        
        // NOTE(ezexff): Rendering parameters
        {
            // NOTE(ezexff): Camera
            GameState->CameraPitch = 45.0f;
            GameState->CameraYaw = 315.0f;
            Frame->CameraZ = 1.75f;
            
            // NOTE(ezexff): Skybox
            Frame->DrawSkybox = true;
            
            // NOTE(ezexff): Terrain
            GameState->MaxTerrainHeight = 0.2f;
            GameState->TilesPerChunkRow = 16;
            Frame->DrawTerrain = true;
            /*Frame->TerrainMaterial.Ambient = V4(0.2f, 0.2f, 0.2f, 1.0f);
            Frame->TerrainMaterial.Diffuse = V4(0.8f, 0.8f, 0.8f, 1.0f);
            Frame->TerrainMaterial.Specular = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Frame->TerrainMaterial.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Frame->TerrainMaterial.Shininess = 0.0f;*/
            Frame->TerrainMaterial.Ambient = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Frame->TerrainMaterial.Diffuse = V4(0.1f, 0.35f, 0.1f, 1.0f);
            Frame->TerrainMaterial.Specular = V4(0.45f, 0.55f, 0.45f, 1.0f);
            Frame->TerrainMaterial.Emission = V4(0.0f, 0.0f, 0.0f, 1.0f);
            Frame->TerrainMaterial.Shininess = 0.25f;
            
            // NOTE(ezexff): Light
            //Frame->DirLight.Base.Color = V3(0.5f, 0.5f, 0.5f);
            Frame->DirLight.Base.Color = V3(1.0f, 1.0f, 1.0f);
            Frame->DirLight.Base.AmbientIntensity = 0.1f;
            Frame->DirLight.Base.DiffuseIntensity = 1.0f;
            //Frame->DirLight.WorldDirection = V3(1.0f, 1.0f, -1.0f);
            //Frame->DirLight.WorldDirection = V3(1.5f, 0.0f, 3.0f);
            //Frame->DirLight.WorldDirection = V3(3.0f, 6.0f, -73.5f);
            //Frame->DirLight.WorldDirection = V3(1.0f, 0.0f, 1.0f);
            Frame->DirLight.WorldDirection = V3(0.0f, 0.0f, -1.0f);
            
            // NOTE(ezexff): Shadows
            //Frame->ShadowMapDim.x = 1920 * 4;
            //Frame->ShadowMapDim.y = 1080 * 4;
            Frame->ShadowMapSize = 60.0f;
            Frame->ShadowMapNearPlane = -50.0f;
            Frame->ShadowMapFarPlane = 100.0f;
            Frame->ShadowMapCameraPitch = 60.0f;
            Frame->ShadowMapCameraYaw = -120.0f;
            Frame->ShadowMapCameraPos = V3(0.0f, 0.0f, 0.0f);
            Frame->ShadowMapBias = 0.005f;
            
#if ENGINE_INTERNAL
            ImGuiHandle->DrawSpaceBounds = true;
#endif
        }
        
        ModeWorld->IsInitialized = true;
    }
    
    //~ NOTE(ezexff): Keyboard and gamepad inputs
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_hero *ConHero = GameState->ControlledHeroes + ControllerIndex;
        if(ConHero->EntityIndex == 0)
        {
            //if(Controller->Start.EndedDown)
            if(WasPressed(Controller->Start))
            {
#if ENGINE_INTERNAL
                Log->Add("[world input]: 1st time start was pressed\n");
#endif
                *ConHero = {};
                ConHero->EntityIndex = AddPlayer(GameState).LowIndex;
            }
        }
        else
        {
            ConHero->dZ = 0.0f;
            ConHero->ddP = {};
            ConHero->dSword = {};
            
            if(IsDown(Controller->MoveUp))
            {
                ConHero->ddP.y += 1.0f;
            }
            if(IsDown(Controller->MoveDown))
            {
                ConHero->ddP.y += -1.0f;
            }
            if(IsDown(Controller->MoveLeft))
            {
                ConHero->ddP.x += -1.0f;
            }
            if(IsDown(Controller->MoveRight))
            {
                ConHero->ddP.x += 1.0f;
            }
            
            if(WasPressed(Controller->Start))
            {
#if ENGINE_INTERNAL
                Log->Add("[world input]: Start was pressed\n");
#endif
                ConHero->dZ = 3.0f;
            }
            
            //if(Controller->ActionUp.EndedDown)
            if(IsDown(Controller->ActionUp))
            {
                ConHero->dSword = V2(0.0f, 1.0f);
            }
            //if(Controller->ActionDown.EndedDown)
            if(IsDown(Controller->ActionDown))
            {
                ConHero->dSword = V2(0.0f, -1.0f);
            }
            //if(Controller->ActionLeft.EndedDown)
            if(IsDown(Controller->ActionLeft))
            {
                ConHero->dSword = V2(-1.0f, 0.0f);
            }
            //if(Controller->ActionRight.EndedDown)
            if(IsDown(Controller->ActionRight))
            {
                ConHero->dSword = V2(1.0f, 0.0f);
            }
        }
    }
    
    // NOTE(ezexff): Mouse
    {
        // NOTE(ezexff): Camera z
        if(Input->MouseDelta.z != 0)
        {
            Frame->CameraZ -= Input->MouseDelta.z;
        }
        
        // NOTE(ezexff): Keys
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
        {
            //Log->Add("[input]: VK_LBUTTON was pressed\n");
        }
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Middle]))
        {
            //Log->Add("[input]: VK_MBUTTON was pressed\n");
        }
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
        {
            //Log->Add("[input]: VK_RBUTTON was pressed\n");
        }
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Extended0]))
        {
            //Log->Add("[input]: VK_XBUTTON1 was pressed\n");
        }
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Extended1]))
        {
            //Log->Add("[input]: VK_XBUTTON2 was pressed\n");
        }
        
        // NOTE(ezexff): Mouse cursor input delta
        Input->CenteringMouseCursor = true;
#if ENGINE_INTERNAL
        if(!ImGuiHandle->ShowImGuiWindows)
#endif
        {
            r32 Sensitivity = 0.05;
            
            // Horizontal (camera yaw)
            GameState->CameraYaw -= (r32)Input->MouseDelta.x * Sensitivity;
            if(GameState->CameraYaw < -180)
            {
                GameState->CameraYaw += 360;
            }
            if(GameState->CameraYaw > 180)
            {
                GameState->CameraYaw -= 360;
            }
            
            // Vertical (camera pitch)
            GameState->CameraPitch += (r32)Input->MouseDelta.y * Sensitivity;
            if(GameState->CameraPitch < 0)
            {
                GameState->CameraPitch = 0;
            }
            if(GameState->CameraPitch > 180)
            {
                GameState->CameraPitch = 180;
            }
        }
    }
    
    //~ NOTE(ezexff): Sim region
    u32 SimChunksInCamera = 5;
    r32 CameraWidthInMeters = SimChunksInCamera * World->ChunkDimInMeters.x;
    r32 CameraHeightInMeters = SimChunksInCamera * World->ChunkDimInMeters.y;
    rectangle3 CameraBoundsInMeters = RectCenterDim(V3(0, 0, 0), V3(CameraWidthInMeters, CameraHeightInMeters, 0.0f));
    
    v3 SimBoundsExpansion = V3(15.0f, 15.0f, 15.0f);
    rectangle3 SimBounds = AddRadiusTo(CameraBoundsInMeters, SimBoundsExpansion);
    
    temporary_memory SimMemory = BeginTemporaryMemory(&TranState->TranArena);
    sim_region *SimRegion = BeginSim(&TranState->TranArena, GameState, GameState->World, //
                                     GameState->CameraP, SimBounds, Input->dtForFrame);
    
    //~ NOTE(ezexff): Render
    Frame->Camera.P = V3(0, 0, 0);
    Frame->Camera.Angle.x = GameState->CameraPitch;
    Frame->Camera.Angle.y = GameState->CameraYaw;
    Frame->Camera.Angle.z = GameState->CameraRoll;
    
    //~ NOTE(ezexff): Load skybox assets
    asset_type *Type = TranState->Assets->AssetTypes + Asset_Skybox;
    u32 SkyboxBitmapCount = Type->OnePastLastAssetIndex - Type->FirstAssetIndex;
    if(SkyboxBitmapCount >= 6)
    {
        u32 Index = 0;
        for(u32 AssetIndex = Type->FirstAssetIndex;
            AssetIndex < Type->FirstAssetIndex + 6;
            AssetIndex++)
        {
            asset *Asset = TranState->Assets->Assets + AssetIndex;
            eab_bitmap EABBitmap = Asset->EAB.Bitmap;
            
            bitmap_id ID = {};
            ID.Value = AssetIndex;
            if(Asset->State == AssetState_Loaded)
            {
                loaded_bitmap *Bitmap = GetBitmap(TranState->Assets, ID, true);
                Frame->Skybox[Index] = *Bitmap;
            }
            else if(Asset->State == AssetState_Unloaded)
            {
                LoadBitmap(TranState->Assets, ID, true);
                Frame->MissingResourceCount++;
            }
            Index++;
        }
    }
    
#if ENGINE_INTERNAL
    // NOTE(ezexff): Sim region outlines
    {
        if(ImGuiHandle->DrawCameraBounds)
        {
            PushRectOutlineOnGround(Frame, V2(0, 0), V2(CameraWidthInMeters, CameraHeightInMeters), V4(1.0f, 1.0f, 0.0f, 1));
        }
        if(ImGuiHandle->DrawSimBounds)
        {
            PushRectOutlineOnGround(Frame, V2(0, 0), GetDim(SimBounds).xy, V4(0.0f, 1.0f, 1.0f, 1));
        }
        if(ImGuiHandle->DrawSimRegionBounds)
        {
            PushRectOutlineOnGround(Frame, V2(0, 0), GetDim(SimRegion->Bounds).xy, V4(1, 0.5, 0, 1));
        }
        if(ImGuiHandle->DrawSimRegionUpdatableBounds)
        {
            PushRectOutlineOnGround(Frame, V2(0, 0), GetDim(SimRegion->UpdatableBounds).xy, V4(1.0f, 0.0f, 1.0f, 1));
        }
    }
#endif
    
    // NOTE(ezexff): Entities
    for(u32 EntityIndex = 0;
        EntityIndex < SimRegion->EntityCount;
        ++EntityIndex)
    {
        sim_entity *Entity = SimRegion->Entities + EntityIndex;
        if(Entity->Updatable)
        {
            r32 dt = Input->dtForFrame;
            
            // TODO(casey): This is incorrect, should be computed after update!!!!
            move_spec MoveSpec = DefaultMoveSpec();
            v3 ddP = {};
            
            v3 WishDir = {};
            r32 WishSpeed = {};
            r32 sv_accelerate = 7.0f;
            r32 ForceForMove = 400.0f;
            ForceForMove *= 0.0254f;
            
            //
            // NOTE(me): Pre-physics entity work
            //
            switch(Entity->Type)
            {
                case EntityType_Hero:
                {
                    // TODO(casey): Now that we have some real usage examples, let's solidify
                    // the positioning system!
                    for(u32 ControlIndex = 0;
                        ControlIndex < ArrayCount(GameState->ControlledHeroes);
                        ++ControlIndex)
                    {
                        controlled_hero *ConHero = GameState->ControlledHeroes + ControlIndex;
                        
                        if(Entity->StorageIndex == ConHero->EntityIndex)
                        {
                            if(ConHero->dZ != 0.0f)
                            {
                                Entity->dP.z = ConHero->dZ;
                            }
                            
                            MoveSpec.UnitMaxAccelVector = true;
                            MoveSpec.Speed = 50.0f;
                            //MoveSpec.Drag = 8.0f;
                            MoveSpec.Drag = 1.0f;
                            // MoveSpec.Speed = 25.0f;
                            // MoveSpec.Drag = 1.0f;
#if 0
                            //r32 YawInRad = GameState->CameraYaw * Pi32 / 180;
                            r32 YawInRad = DegToRad(GameState->CameraYaw);
                            ddP.x = ConHero->ddP.x * Cos(YawInRad) - ConHero->ddP.y * Sin(YawInRad);
                            ddP.y = ConHero->ddP.x * Sin(YawInRad) + ConHero->ddP.y * Cos(YawInRad);
                            ddP.z = 0;
                            //ddP = V3(ConHero->ddP, 0);
#else
                            r32 Yaw = DegToRad(GameState->CameraYaw);
                            r32 Tmp1 = 90 - GameState->CameraPitch;
                            r32 Pitch = DegToRad(Tmp1);
                            //r32 Pitch = DegToRad(GameState->CameraPitch);
                            //Log->Add("test %f\n", Pitch);
                            r32 Roll = DegToRad(GameState->CameraRoll);
                            
                            r32 SinYaw = Sin(Yaw);
                            r32 CosYaw = Cos(Yaw);
                            
                            r32 SinPitch = Sin(Pitch);
                            r32 CosPitch = Cos(Pitch);
                            
                            r32 SinRoll = Sin(Roll);
                            r32 CosRoll = Cos(Roll);
                            
                            v3 Forward;
                            v3 Right;
                            
                            Forward.x = CosPitch * CosYaw;
                            Forward.y = CosPitch * SinYaw;
                            Forward.z = -SinPitch;
                            
                            Right.x = (-1 * SinRoll * SinPitch * CosYaw + -1 * CosRoll * -SinYaw);
                            Right.y = (-1 * SinRoll * SinPitch * SinYaw + -1 * CosRoll * CosYaw);
                            Right.z = (-1 * SinRoll * CosPitch);
                            
                            //Log->Add("f = %f %f %f\n", Forward.x, Forward.y, Forward.z);
                            //Log->Add("r = %f %f %f\n", Right.x, Right.y, Right.z);
                            
                            Forward.z = 0.0f;
                            Right.z = 0.0f;
                            v3 NForward = Normalize(Forward);
                            v3 NRight = Normalize(Right);
                            
                            if(ConHero->ddP.x == 1.0f)
                            {
                                int f4324 = 0;
                            }
                            
                            r32 FMove = ForceForMove * ConHero->ddP.x;
                            r32 SMove = -ForceForMove * ConHero->ddP.y;
                            
                            v3 WishVel;
                            WishVel.x = NForward.x * FMove + NRight.x * SMove;
                            WishVel.y = NForward.y * FMove + NRight.y * SMove;
                            WishVel.z = 0.0f;
                            
                            if((WishVel.x != 0) || (WishVel.y != 0) | (WishVel.z != 0))
                            {
                                WishDir = Normalize(WishVel);
                            }
                            WishSpeed = Length(WishVel);
#endif
                            
                            if((ConHero->dSword.x != 0.0f) || (ConHero->dSword.y != 0.0f))
                            {
                                sim_entity *Sword = Entity->Sword.Ptr;
                                if(Sword && IsSet(Sword, EntityFlag_Nonspatial))
                                {
                                    Sword->DistanceLimit = 1.0f;
                                    //MakeEntitySpatial(Sword, Entity->P, Entity->dP + 5.0f * V3(ConHero->dSword, 0));
                                    r32 dSwordMultiplier = 10.0f;
                                    MakeEntitySpatial(Sword, Entity->P, V3(ConHero->dSword * dSwordMultiplier, 0));
                                    AddCollisionRule(GameState, Sword->StorageIndex, Entity->StorageIndex, false);
                                    
                                    PlaySound(&GameState->AudioState, GetFirstSoundFrom(TranState->Assets, Asset_Bloop));
                                }
                            }
                        }
                    }
                } break;
                
                case EntityType_Sword:
                {
                    MoveSpec.UnitMaxAccelVector = false;
                    MoveSpec.Speed = 0.0f;
                    MoveSpec.Drag = 0.0f;
                    
                    if(Entity->DistanceLimit == 0.0f)
                    {
                        ClearCollisionRulesFor(GameState, Entity->StorageIndex);
                        MakeEntityNonSpatial(Entity);
                    }
                } break;
                
                case EntityType_Familiar:
                {
                    /*sim_entity *ClosestHero = 0;
                    real32 ClosestHeroDSq = Square(10.0f); // NOTE(casey): Ten meter maximum search!
    
                    if(ClosestHero && (ClosestHeroDSq > Square(3.0f)))
                    {
                        real32 Acceleration = 0.5f;
                        real32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
                        ddP = OneOverLength * (ClosestHero->P - Entity->P);
                    }
    
                    MoveSpec.UnitMaxAccelVector = true;
                    MoveSpec.Speed = 50.0f;
                    MoveSpec.Drag = 8.0f;
    
                    Entity->tBob += dt;
                    if(Entity->tBob > (2.0f * Pi32))
                    {
                        Entity->tBob -= (2.0f * Pi32);
                    }*/
                } break;
                
                case EntityType_Monstar:
                {
                    if(!IsSet(Entity, EntityFlag_Nonspatial) && Entity->HitPointMax == 0.0f)
                    {
                        ClearCollisionRulesFor(GameState, Entity->StorageIndex);
                        MakeEntityNonSpatial(Entity);
                    }
                } break;
            }
            
            if(!IsSet(Entity, EntityFlag_Nonspatial) &&
               IsSet(Entity, EntityFlag_Moveable))
            {
                MoveEntity(GameState, SimRegion, Entity, Input->dtForFrame, &MoveSpec, ddP,
                           WishDir, WishSpeed, sv_accelerate);
            }
            
            //RenderGroup->Transform.OffsetP = GetEntityGroundPoint(Entity);
            Frame->OffsetP = GetEntityGroundPoint(Entity);
            
            // NOTE(ezexff): Calc entity z based on terrain height
            if(Entity->Type == EntityType_Wall || 
               Entity->Type == EntityType_Monstar ||
               Entity->Type == EntityType_Hero)
            {
                low_entity *LowEntity = GetLowEntity(GameState, Entity->StorageIndex);
                world_position WorldEntityP = LowEntity->P;
                
                r32 RelTileX = GameState->TilesPerChunkRow * WorldEntityP.ChunkX + 
                (GameState->GroundBufferWidth / 2.0f) + WorldEntityP.Offset_.x;
                r32 RelTileY = GameState->TilesPerChunkRow * WorldEntityP.ChunkY +
                (GameState->GroundBufferHeight / 2.0f) + WorldEntityP.Offset_.y;
                s32 TileX = FloorReal32ToInt32(RelTileX);
                s32 TileY = FloorReal32ToInt32(RelTileY);
                s32 TileZ = 0; // TODO(ezexff): Need rework when start using z chunks
                
                random_series Series = RandomSeed(139 * (TileX) + 593 * (TileY) + 329 * TileZ);
                r32 RandomZ00 = RandomUnilateral(&Series) * GameState->MaxTerrainHeight;
                Series = RandomSeed(139 * (TileX + 1) + 593 * (TileY) + 329 * TileZ);
                r32 RandomZ10 = RandomUnilateral(&Series) * GameState->MaxTerrainHeight;
                Series = RandomSeed(139 * (TileX) + 593 * (TileY + 1) + 329 * TileZ);
                r32 RandomZ01 = RandomUnilateral(&Series) * GameState->MaxTerrainHeight;
                Series = RandomSeed(139 * (TileX + 1) + 593 * (TileY + 1) + 329 * TileZ);
                r32 RandomZ11 = RandomUnilateral(&Series) * GameState->MaxTerrainHeight;
                
                r32 BaseOffsetX = RelTileX - TileX;
                r32 BaseOffsetY = RelTileY - TileY;
                r32 H1 = Lerp(RandomZ00, BaseOffsetX, RandomZ10); // x-axis
                r32 H2 = Lerp(RandomZ01, BaseOffsetX, RandomZ11); // y-axis
                r32 OnTerrainZ = (H1, BaseOffsetY, H2);
                Frame->OffsetP.z = OnTerrainZ;
                
                // NOTE(ezexff): Terrain z for player camera
                if(Entity->Type == EntityType_Hero && Frame->FixCameraOnTerrain)
                {
#define PLAYER_HEIGHT 1.75f
                    Frame->CameraZ = OnTerrainZ + PLAYER_HEIGHT;
                }
            }
            
            //
            // NOTE(ezexff): Post-physics entity work
            //
            switch(Entity->Type)
            {
                case EntityType_Hero:
                {
                    /*if(Debug->DrawPlayerHitbox)
                    {
                        PushRectCollisionVolumes(RenderGroup, Entity, V4(0, 1, 0, 1));
                    }*/
                    for(u32 VolumeIndex = 0;
                        VolumeIndex < Entity->Collision->VolumeCount;
                        ++VolumeIndex)
                    {
                        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                        PushRectOutlineOnGround(Frame, Frame->OffsetP.xy, Volume->Dim.xy, V4(0, 1, 0, 1));
                    }
                } break;
                
                case EntityType_Wall:
                {
                    //PushRectCollisionVolumes(RenderGroup, Entity, V4(1, 0, 0, 1));
                    for(u32 VolumeIndex = 0;
                        VolumeIndex < Entity->Collision->VolumeCount;
                        ++VolumeIndex)
                    {
                        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                        Frame->OffsetP.z += Volume->OffsetP.z;
                        PushCube(Frame, Frame->OffsetP, Volume->Dim, V4(1, 0.5f, 0, 1));
                    }
                } break;
                
                case EntityType_Stairwell:
                {
                    // PushRect(&PieceGroup, V2(0, 0), 0, Entity->WalkableDim, V4(1, 0.5f, 0, 1), 0.0f);
                    // PushRect(&PieceGroup, V2(0, 0), Entity->WalkableHeight, Entity->WalkableDim, V4(1, 1, 0, 1),
                    // 0.0f);
                } break;
                
                case EntityType_Sword:
                {
                    //PushRectCollisionVolumes(RenderGroup, Entity, V4(1, 1, 0, 1));
                    for(u32 VolumeIndex = 0;
                        VolumeIndex < Entity->Collision->VolumeCount;
                        ++VolumeIndex)
                    {
                        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                        PushRectOutlineOnGround(Frame, Frame->OffsetP.xy, Volume->Dim.xy, V4(0, 0, 1, 1));
                    }
                } break;
                
                case EntityType_Familiar:
                {
                    /*
                    real32 BobSin = Sin(2.0f * Entity->tBob);
                    PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align,
                               (0.5f * ShadowAlpha) + 0.2f * BobSin, 0.0f);
                    PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0, 0), 0.25f * BobSin, HeroBitmaps->Align);
                    */
                } break;
                
                case EntityType_Monstar:
                {
                    //PushRectCollisionVolumes(RenderGroup, Entity, V4(0.5f, 0, 0.5f, 1));
                    for(u32 VolumeIndex = 0;
                        VolumeIndex < Entity->Collision->VolumeCount;
                        ++VolumeIndex)
                    {
                        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                        //PushRectOutlineOnGround(Frame, Frame->OffsetP, Volume->Dim.xy, V4(1, 0, 0, 1));
                        Frame->OffsetP.z += Volume->OffsetP.z;
                        PushCubeOutline(Frame, Frame->OffsetP, Volume->Dim, V4(1, 0, 0, 1));
                    }
                    
                    /*ImGui::Begin("Monstar");
                    ImGui::Text("HitPoints = %d", Entity->HitPointMax);
                    ImGui::End();*/
                } break;
                
                case EntityType_Space:
                {
#if ENGINE_INTERNAL
                    if(ImGuiHandle->DrawSpaceBounds)
                    {
                        for(u32 VolumeIndex = 0;
                            VolumeIndex < Entity->Collision->VolumeCount;
                            ++VolumeIndex)
                        {
                            sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                            //PushRectOnGround(Frame, Frame->OffsetP.xy, Volume->Dim.xy, V4(0.5f, 0.5f, 0.5f, 1));
                            PushRectOutlineOnGround(Frame, Frame->OffsetP.xy, Volume->Dim.xy, V4(1, 1, 1, 1));
                        }
                    }
#endif
                } break;
                
                InvalidDefaultCase;
            }
        }
    }
    
    // NOTE(ezexff): Fill ground buffer
#if 1
    world_position MinChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMinCorner(CameraBoundsInMeters));
    world_position MaxChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMaxCorner(CameraBoundsInMeters));
    
    for(s32 ChunkZ = MinChunkP.ChunkZ;
        ChunkZ <= MaxChunkP.ChunkZ;
        ++ChunkZ)
    {
        for(s32 ChunkY = MinChunkP.ChunkY;
            ChunkY <= MaxChunkP.ChunkY;
            ++ChunkY)
        {
            for(s32 ChunkX = MinChunkP.ChunkX;
                ChunkX <= MaxChunkP.ChunkX;
                ++ChunkX)
            {
                world_position ChunkCenterP = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
                v3 RelP = Subtract(World, &ChunkCenterP, &GameState->CameraP);
                
                u32 TmpGroundBufferIndex = 0;
                // TODO(casey): This is super inefficient fix it!
                r32 FurthestBufferLengthSq = 0.0f;
                ground_buffer *FurthestBuffer = 0;
                for(u32 GroundBufferIndex = 0;
                    GroundBufferIndex < Frame->GroundBufferCount;
                    ++GroundBufferIndex)
                {
                    ground_buffer *GroundBuffer = Frame->GroundBuffers + GroundBufferIndex;
                    if(AreInSameChunk(World, &GroundBuffer->P, &ChunkCenterP))
                    {
                        TmpGroundBufferIndex = 0;
                        FurthestBuffer = 0;
                        break;
                    }
                    else if(IsValid(GroundBuffer->P))
                    {
                        v3 RelP = Subtract(World, &GroundBuffer->P, &GameState->CameraP);
                        r32 BufferLengthSq = LengthSq(RelP.xy);
                        if(FurthestBufferLengthSq < BufferLengthSq)
                        {
                            TmpGroundBufferIndex = GroundBufferIndex;
                            FurthestBufferLengthSq = BufferLengthSq;
                            FurthestBuffer = GroundBuffer;
                        }
                    }
                    else
                    {
                        TmpGroundBufferIndex = GroundBufferIndex;
                        FurthestBufferLengthSq = F32Max;
                        FurthestBuffer = GroundBuffer;
                    }
                }
                
                if(FurthestBuffer)
                {
                    //Log->Add("[fillgroundchunk] TmpGroundBufferIndex=%d (%d,%d)\n", TmpGroundBufferIndex, ChunkCenterP.ChunkX, ChunkCenterP.ChunkY);
                    FillGroundChunk(TranState, GameState, Frame,
                                    FurthestBuffer,
                                    &ChunkCenterP);
                }
            }
        }
    }
    
    // NOTE(ezexff): Render Ground Buffers
    {
        for(u32 GroundBufferIndex = 0;
            GroundBufferIndex < Frame->GroundBufferCount;
            ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = Frame->GroundBuffers + GroundBufferIndex;
            if(IsValid(GroundBuffer->P))
            {
                GroundBuffer->OffsetP = Subtract(GameState->World, &GroundBuffer->P, &GameState->CameraP);
                
                // TODO(ezexff): Test
                if((GroundBuffer->P.ChunkX == 0) && (GroundBuffer->P.ChunkY == 0) && (GroundBuffer->P.ChunkZ == 0))
                {
                    u32 CurrentVertex = 0;
                    for(u32 Y = 0;
                        Y <= Frame->TileCount;
                        ++Y)
                    {
                        for(u32 X = 0;
                            X <= Frame->TileCount;
                            ++X)
                        {
                            v3 P1 = GroundBuffer->OffsetP + GroundBuffer->Vertices[CurrentVertex].Position;
                            v3 P2 = GroundBuffer->OffsetP + GroundBuffer->Vertices[CurrentVertex].Position + GroundBuffer->Vertices[CurrentVertex].Normal;
                            PushLine(Frame, P1,  P2);
                            CurrentVertex++;
                        }
                    }
                }
            }
        }
    }
#endif
    
#if 0
    // NOTE(ezexff): Render ground buffers
    Frame->TerrainVerticesCount = TranState->GroundBufferCount * ((GameState->TilesPerChunkRow + 1) * (GameState->TilesPerChunkRow + 1));
    Frame->TerrainIndicesCount = TranState->GroundBufferCount * (6 * GameState->TilesPerChunkRow * GameState->TilesPerChunkRow);
    Frame->TerrainVertices = PushArray(&TranState->TranArena, Frame->TerrainVerticesCount, vbo_vertex);
    Frame->TerrainIndices = PushArray(&TranState->TranArena, Frame->TerrainIndicesCount, u32);
    
    u32 CurrentVertex = 0;
    u32 CurrentIndex = 0;
    
    for(u32 GroundBufferIndex = 0;
        GroundBufferIndex < TranState->GroundBufferCount;
        ++GroundBufferIndex)
    {
        ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
        if(IsValid(GroundBuffer->P))
        {
            v3 Delta = Subtract(GameState->World, &GroundBuffer->P, &GameState->CameraP);
            
            if((Delta.z >= -1.0f) && (Delta.z < 1.0f))
            {
                r32 MinDeltaX = Delta.x - GameState->GroundBufferWidth / 2;
                r32 MinDeltaY = Delta.y - GameState->GroundBufferHeight / 2;
                r32 MaxDeltaX = MinDeltaX + GameState->GroundBufferWidth;
                r32 MaxDeltaY = MinDeltaY + GameState->GroundBufferHeight;
                
                s32 ChunkX = GroundBuffer->P.ChunkX;
                s32 ChunkY = GroundBuffer->P.ChunkY;
                s32 ChunkZ = GroundBuffer->P.ChunkZ;
                
                u32 TilesPerChunkRow = GameState->TilesPerChunkRow;
                s32 TileX = ChunkX * TilesPerChunkRow;
                s32 TileY = ChunkY * TilesPerChunkRow;
                s32 TileZ = ChunkZ * TilesPerChunkRow;
                if((TileX > 1000000) || (TileY > 1000000) || (TileZ > 1000000))
                {
                    InvalidCodePath;
                }
                
                r32 TileWidth = (r32)GameState->GroundBufferWidth / TilesPerChunkRow;
                r32 TileHeight = (r32)GameState->GroundBufferHeight / TilesPerChunkRow; 
                
                // Positions and indices
                for(u32 Y = 0;
                    Y <= TilesPerChunkRow;
                    ++Y)
                {
                    for(u32 X = 0;
                        X <= TilesPerChunkRow;
                        ++X)
                    {
                        // NOTE(ezexff): Position
                        v3 A = {};
                        A.x = MinDeltaX + X * TileWidth;
                        A.y = MinDeltaY + Y * TileHeight;
                        u32 SeedValueA = 139 * (TileX + X) + 593 * (TileY + Y) + 329 * TileZ;
                        random_series SeriesA = RandomSeed(SeedValueA);
                        r32 AZ = RandomUnilateral(&SeriesA);
                        AZ *= GameState->MaxTerrainHeight;
                        A.z = AZ;
                        Frame->TerrainVertices[CurrentVertex].Position = A;
                        
                        // NOTE(ezexff): Normal
                        v3 B = {};
                        B.x = MinDeltaX + (X + 1) * TileWidth;
                        B.y = A.y;
                        u32 SeedValueB = 139 * (TileX + (X + 1)) + 593 * (TileY + Y) + 329 * TileZ;
                        random_series SeriesB = RandomSeed(SeedValueB);
                        r32 BZ = RandomUnilateral(&SeriesB);
                        BZ *= GameState->MaxTerrainHeight;
                        B.z = BZ;
                        
                        v3 C = {};
                        C.x = A.x;;
                        C.y = MinDeltaY + (Y + 1) * TileHeight;
                        u32 SeedValueC = 139 * (TileX + X) + 593 * (TileY + (Y + 1)) + 329 * TileZ;
                        random_series SeriesC = RandomSeed(SeedValueC);
                        r32 CZ = RandomUnilateral(&SeriesC);
                        CZ *= GameState->MaxTerrainHeight;
                        
                        Frame->TerrainVertices[CurrentVertex].Normal = CalcNormal(A, B, C);
                        
                        
                        /* 
                                                if((ChunkX == 0) && (ChunkY == 0) && (ChunkZ == 0))
                                                {
                                                    v3 P1 = Frame->TerrainVertices[CurrentVertex].Position;
                                                    v3 P2 = Frame->TerrainVertices[CurrentVertex].Position + Frame->TerrainVertices[CurrentVertex].Normal;
                                                    PushLine(Frame, P1,  P2);
                                                }
                         */
                        
                        // NOTE(ezexff): Indices
                        if((X != TilesPerChunkRow) && (Y != TilesPerChunkRow))
                        {
                            Frame->TerrainIndices[CurrentIndex + 0] = CurrentVertex;
                            Frame->TerrainIndices[CurrentIndex + 1] = CurrentVertex + 1;
                            Frame->TerrainIndices[CurrentIndex + 2] = CurrentVertex + TilesPerChunkRow + 1;
                            
                            Frame->TerrainIndices[CurrentIndex + 3] = CurrentVertex + 1;
                            Frame->TerrainIndices[CurrentIndex + 4] = CurrentVertex + TilesPerChunkRow + 1;
                            Frame->TerrainIndices[CurrentIndex + 5] = CurrentVertex + TilesPerChunkRow + 2;
                            
                            CurrentIndex += 6;
                        }
                        CurrentVertex++;
                    }
                }
            }
        }
    }
    Frame->TerrainVerticesCount = CurrentVertex;
    Frame->TerrainIndicesCount = CurrentIndex;
#endif
    
#if 0
    // NOTE(ezexff): Ground chunks
    world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
    world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));
    
    u32 TestIndex = 0;
    for(s32 ChunkZ = MinChunkP.ChunkZ;
        ChunkZ <= MaxChunkP.ChunkZ;
        ++ChunkZ)
    {
        for(s32 ChunkY = MinChunkP.ChunkY;
            ChunkY <= MaxChunkP.ChunkY;
            ++ChunkY)
        {
            for(s32 ChunkX = MinChunkP.ChunkX;
                ChunkX <= MaxChunkP.ChunkX;
                ++ChunkX)
            {
                
                world_position ChunkCenterP = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
                v3 RelP = Subtract(World, &ChunkCenterP, &GameState->CameraP);
                
                //if(Debug->DrawSimChunks)
                {
                    //PushRectOutlineOnGround(Frame, V3(RelP.xy, 0), World->ChunkDimInMeters.xy, V4(0, 1, 0, 1));
                    PushBitmapOnGround(Frame, Assets, GetFirstBitmapFrom(Assets, Asset_Ground), 
                                       RelP.xy, World->ChunkDimInMeters.xy, 4.0f);
                }
                TestIndex++;
            }
        }
    }
    Log->Add("TestIndex = %u\n", TestIndex);
#endif
    
#if ENGINE_INTERNAL
    if(ImGuiHandle->ShowImGuiWindows)
    {
        if(ImGuiHandle->ShowSimRegionWindow)
        {
            ImGui::Begin("Sim Region", &ImGuiHandle->ShowSimRegionWindow);
            ImGui::Text("Debug window for sim region...");
            
            ImGui::SeparatorText("Origin");
            ImGui::BulletText("Chunk = %d %d %d", 
                              SimRegion->Origin.ChunkX, SimRegion->Origin.ChunkY, SimRegion->Origin.ChunkZ);
            ImGui::BulletText("Offset = %.3f %.3f %.3f", 
                              SimRegion->Origin.Offset_.x, SimRegion->Origin.Offset_.y, SimRegion->Origin.Offset_.z);
            
            ImGui::SeparatorText("Bounds");
            ImGui::BulletText("Min = %.3f %.3f %.3f", 
                              SimRegion->Bounds.Min.x, SimRegion->Bounds.Min.y, SimRegion->Bounds.Min.z);
            ImGui::BulletText("Max = %.3f %.3f %.3f", 
                              SimRegion->Bounds.Max.x, SimRegion->Bounds.Max.y, SimRegion->Bounds.Max.z);
            
            ImGui::SeparatorText("UpdatableBounds");
            ImGui::BulletText("Min = %.3f %.3f %.3f", 
                              SimRegion->UpdatableBounds.Min.x, 
                              SimRegion->UpdatableBounds.Min.y, 
                              SimRegion->UpdatableBounds.Min.z);
            ImGui::BulletText("Max = %.3f %.3f %.3f", 
                              SimRegion->UpdatableBounds.Max.x, 
                              SimRegion->UpdatableBounds.Max.y, 
                              SimRegion->UpdatableBounds.Max.z);
            
            ImGui::SeparatorText("Entities");
            ImGui::BulletText("MaxEntityCount = %d", SimRegion->MaxEntityCount);
            ImGui::BulletText("EntityCount = %d", SimRegion->EntityCount);
            
            if(ImGui::CollapsingHeader("List"))
            {
                char *EntityTypes[] = 
                {
                    "Null",
                    "Space",
                    "Hero",
                    "Wall",
                    "Familiar",
                    "Monstar",
                    "Sword",
                    "Stairwell",
                };
                
                /*
                for(u32 EntityIndex = 0;
                    EntityIndex < SimRegion->EntityCount;
                    ++EntityIndex)
                {
                    sim_entity *Entity = SimRegion->Entities + EntityIndex;
                    if(Entity->Updatable)
                    {
                        ImGui::Text("#%d %s", Entity->StorageIndex, EntityTypes[Entity->Type]);
                        ImGui::Text("  P: %.3f %.3f %.3f", Entity->P.x, Entity->P.y, Entity->P.z);
                        ImGui::Text("  HP: %d", Entity->HitPointMax);
                    }
                }*/
                
                ImGui::SeparatorText("Updatable");
                for(s32 TypeID = 0;
                    TypeID < EntityType_Count;
                    TypeID++)
                {
                    if(ImGui::TreeNode((void *)(intptr_t)TypeID, "%s", EntityTypes[TypeID]))
                    {
                        for(u32 EntityIndex = 0;
                            EntityIndex < SimRegion->EntityCount;
                            ++EntityIndex)
                        {
                            sim_entity *Entity = SimRegion->Entities + EntityIndex;
                            if(Entity->Updatable && (Entity->Type == TypeID))
                            {
                                ImGui::Text("#%d %s", Entity->StorageIndex, EntityTypes[Entity->Type]);
                                ImGui::Text("  P: %.3f %.3f %.3f", Entity->P.x, Entity->P.y, Entity->P.z);
                                ImGui::Text("  HP: %d", Entity->HitPointMax);
                            }
                        }
                        
                        ImGui::TreePop();
                        ImGui::Spacing();
                    }
                }
            }
            ImGui::End();
        }
    }
#endif
    
    // TODO(ezexff): Mb rework text output and add kerning
    if(Frame->DrawDebugTextLine)
    {
        char *TestString = "The quick brown fox jumps over a lazy dog.";
        DEBUGTextLine(Frame, TranState->Assets, TestString);
    }
    /*PushRectOnGround(Frame, V3(0, 0, 0), V2(5, 5), V4(0, 1, 0, 1));
    PushRectOutlineOnGround(Frame, V3(0, 7, 0), V2(5, 5), V4(0, 0, 1, 1), 0.5f);
    PushBitmapOnGround(Frame, Assets, GetFirstBitmapFrom(Assets, Asset_Ground), V3(0, -7, 0), V2(5, 5), 4.0f);*/
    
    //~ NOTE(ezexff): End frame
    EndSim(SimRegion, GameState);
    EndTemporaryMemory(SimMemory);
    
    CheckArena(&GameState->ConstArena);
    CheckArena(&TranState->TranArena);
    
#if ENGINE_INTERNAL
    {
        // NOTE(ezexff): Player pos, angle, velocity
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
        //ImGui::SetNextWindowSize(ImVec2(200, 100));
        
        static ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Player", NULL, Flags);
        camera *Camera = &Frame->Camera;
        
        world_position *CameraP = &GameState->CameraP;
        ImGui::Text("WorldP: %d %d %d", CameraP->ChunkX, CameraP->ChunkY, CameraP->ChunkZ);
        ImGui::Text("ChunkP: %.3f %.3f %.3f", CameraP->Offset_.x, CameraP->Offset_.y, CameraP->Offset_.z);
        ImGui::Text("Ang: %.3f %.3f %.3f", Camera->Angle.x, Camera->Angle.y, Camera->Angle.z);
        ImGui::Text("CameraZ: %.3f", Frame->CameraZ);
        ImGui::Text("Fps: %.1f", 1.0f / Input->dtForFrame);
        ImGui::Text("dt: %.6f", Input->dtForFrame);
        
        for(u32 EntityIndex = 0;
            EntityIndex < SimRegion->EntityCount;
            ++EntityIndex)
        {
            sim_entity *Entity = SimRegion->Entities + EntityIndex;
            if(Entity->Updatable)
            {
                if(Entity->Type == EntityType_Hero)
                {
                    ImGui::Text("Vel: %.3f %.3f", Entity->dP.x, Entity->dP.y);
                }
            }
        }
        
        ImGui::End();
    }
#endif
}