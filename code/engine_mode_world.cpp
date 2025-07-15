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
FillGroundChunk(renderer_terrain *Terrain, ground_buffer *GroundBuffer, world_position *ChunkP)
{
    GroundBuffer->P = *ChunkP;
    if(IsValid(GroundBuffer->P))
    {
        s32 TileX = GroundBuffer->P.ChunkX * Terrain->TileCount;
        s32 TileY = GroundBuffer->P.ChunkY * Terrain->TileCount;
        s32 TileZ = GroundBuffer->P.ChunkZ * Terrain->TileCount;
        s32 TileMax = 1000000;
        if((TileX > TileMax) || (TileY > TileMax) || (TileZ > TileMax) ||
           (TileX < -TileMax) || (TileY < -TileMax) || (TileZ < -TileMax))
        {
            InvalidCodePath;
        }
        
        u32 TmpPosCount3 = 0;
        for(u32 Y = 0;
            Y <= Terrain->TileCount;
            ++Y)
        {
            for(u32 X = 0;
                X <= Terrain->TileCount;
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
            if((ChunkP->ChunkX == 4) && (ChunkP->ChunkY == 0) && (ChunkP->ChunkZ == 0))
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
                        s32 TmpIndex = Y * (Terrain->TileCount + 1) + X;
                        Assert(TmpIndex >= 0);
                        Assert(TmpIndex < (s32)Terrain->GroundBufferArray.VertexCount);
                        r32 T1 = (r32)(HillTileX - X);
                        r32 T2 = (r32)(HillTileY - Y);
                        r32 Length = SquareRoot(Square(T1) + Square(T2));
                        if(Length < HillRadius)
                        {
                            Length = Length / HillRadius * (r32)Pi32_2;
                            GroundBuffer->Vertices[TmpIndex].Position.z += Cos(Length) * HillZ;
                        }
                    }
                }
            }
            
            HillZ = -2;
            if((ChunkP->ChunkX == 5) && (ChunkP->ChunkY == 0) && (ChunkP->ChunkZ == 0))
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
                        s32 TmpIndex = Y * (Terrain->TileCount + 1) + X;
                        Assert(TmpIndex >= 0);
                        Assert(TmpIndex < (s32)Terrain->GroundBufferArray.VertexCount);
                        r32 T1 = (r32)(HillTileX - X);
                        r32 T2 = (r32)(HillTileY - Y);
                        r32 Length = SquareRoot(Square(T1) + Square(T2));
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
            Y <= Terrain->TileCount;
            ++Y)
        {
            for(u32 X = 0;
                X <= Terrain->TileCount;
                ++X)
            {
                v3 A = {};
                A.x = X * Terrain->TileWidth;
                A.y = Y * Terrain->TileWidth;
                u32 SeedValueA = 139 * (TileX + X) + 593 * (TileY + Y) + 329 * TileZ;
                random_series SeriesA = RandomSeed(SeedValueA);
                r32 AZ = RandomUnilateral(&SeriesA);
                AZ *= Terrain->MaxHeight;
                A.z = AZ;
                GroundBuffer->Vertices[TmpPosCount].Position.z += AZ;
                
                // NOTE(ezexff): Normal
                v3 B = {};
                B.x = (X + 1) * Terrain->TileWidth;
                B.y = A.y;
                u32 SeedValueB = 139 * (TileX + (X + 1)) + 593 * (TileY + Y) + 329 * TileZ;
                random_series SeriesB = RandomSeed(SeedValueB);
                r32 BZ = RandomUnilateral(&SeriesB);
                BZ *= Terrain->MaxHeight;
                B.z = BZ;
                
                v3 C = {};
                C.x = A.x;;
                C.y = (Y + 1) * Terrain->TileHeight;
                u32 SeedValueC = 139 * (TileX + X) + 593 * (TileY + (Y + 1)) + 329 * TileZ;
                random_series SeriesC = RandomSeed(SeedValueC);
                r32 CZ = RandomUnilateral(&SeriesC);
                CZ *= Terrain->MaxHeight;
                
                //Frame->TerrainVertices[CurrentVertex].Normal = CalcNormal(A, B, C);
                GroundBuffer->Vertices[TmpPosCount].Normal = CalcNormal(A, B, C);
                
                TmpPosCount++;
            }
        }
        
        GroundBuffer->IsFilled = true;
    }
}

// NOTE(ezexff): Collision rules
internal void
AddCollisionRule(mode_world *ModeWorld, u32 StorageIndexA, u32 StorageIndexB, b32 CanCollide)
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
    u32 HashBucket = StorageIndexA & (ArrayCount(ModeWorld->CollisionRuleHash) - 1);
    for(pairwise_collision_rule *Rule = ModeWorld->CollisionRuleHash[HashBucket];
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
        Found = ModeWorld->FirstFreeCollisionRule;
        if(Found)
        {
            ModeWorld->FirstFreeCollisionRule = Found->NextInHash;
        }
        else
        {
            Found = PushStruct(ModeWorld->ConstArena, pairwise_collision_rule);
        }
        
        Found->NextInHash = ModeWorld->CollisionRuleHash[HashBucket];
        ModeWorld->CollisionRuleHash[HashBucket] = Found;
    }
    
    if(Found)
    {
        Found->StorageIndexA = StorageIndexA;
        Found->StorageIndexB = StorageIndexB;
        Found->CanCollide = CanCollide;
    }
}

internal void
ClearCollisionRulesFor(mode_world *ModeWorld, u32 StorageIndex)
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
        HashBucket < ArrayCount(ModeWorld->CollisionRuleHash);
        ++HashBucket)
    {
        for(pairwise_collision_rule **Rule = &ModeWorld->CollisionRuleHash[HashBucket];
            *Rule;)
        {
            if(((*Rule)->StorageIndexA == StorageIndex) || ((*Rule)->StorageIndexB == StorageIndex))
            {
                pairwise_collision_rule *RemovedRule = *Rule;
                *Rule = (*Rule)->NextInHash;
                
                RemovedRule->NextInHash = ModeWorld->FirstFreeCollisionRule;
                ModeWorld->FirstFreeCollisionRule = RemovedRule;
            }
            else
            {
                Rule = &(*Rule)->NextInHash;
            }
        }
    }
}

sim_entity_collision_volume_group *
MakeSimpleGroundedCollision(mode_world *ModeWorld, r32 DimX, r32 DimY, r32 DimZ)
{
    // TODO(casey): NOT WORLD ARENA!  Change to using the fundamental types arena, etc.
    sim_entity_collision_volume_group *Group = PushStruct(ModeWorld->ConstArena, sim_entity_collision_volume_group);
    Group->VolumeCount = 1;
    Group->Volumes = PushArray(ModeWorld->ConstArena, Group->VolumeCount, sim_entity_collision_volume);
    Group->TotalVolume.OffsetP = V3(0, 0, 0.5f * DimZ);
    //Group->TotalVolume.OffsetP = V3(0, 0, 0);
    Group->TotalVolume.Dim = V3(DimX, DimY, DimZ);
    Group->Volumes[0] = Group->TotalVolume;
    
    return (Group);
}

sim_entity_collision_volume_group *
MakeNullCollision(mode_world *ModeWorld)
{
    // TODO(casey): NOT WORLD ARENA!  Change to using the fundamental types arena, etc.
    sim_entity_collision_volume_group *Group = PushStruct(ModeWorld->ConstArena, sim_entity_collision_volume_group);
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
AddLowEntity(mode_world *ModeWorld, entity_type Type, world_position P)
{
    Assert(ModeWorld->LowEntityCount < ArrayCount(ModeWorld->LowEntities));
    u32 EntityIndex = ModeWorld->LowEntityCount++;
    
    low_entity *EntityLow = ModeWorld->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Sim.Type = Type;
    EntityLow->Sim.Collision = ModeWorld->NullCollision;
    EntityLow->P = NullPosition();
    
    ChangeEntityLocation(ModeWorld, EntityIndex, EntityLow, P);
    
    add_low_entity_result Result;
    Result.Low = EntityLow;
    Result.LowIndex = EntityIndex;
    
    // TODO(casey): Do we need to have a begin/end paradigm for adding
    // entities so that they can be brought into the high set when they
    // are added and are in the camera region?
    
    return (Result);
}

internal add_low_entity_result 
AddGroundedEntity(mode_world *ModeWorld, entity_type Type, world_position P,
                  sim_entity_collision_volume_group *Collision)
{
    add_low_entity_result Entity = AddLowEntity(ModeWorld, Type, P);
    Entity.Low->Sim.Collision = Collision;
    return (Entity);
}

internal add_low_entity_result 
AddStandardRoom(mode_world *ModeWorld, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(ModeWorld->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Space, P, ModeWorld->StandardRoomCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Traversable);
    
    return (Entity);
}

internal add_low_entity_result 
AddWater(mode_world *ModeWorld, s32 ChunkX, s32 ChunkY, s32 ChunkZ)
{
    world_position P = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Water, P, ModeWorld->WaterCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Traversable);
    
    return (Entity);
}

internal add_low_entity_result 
AddWall(mode_world *ModeWorld, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(ModeWorld->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Wall, P, ModeWorld->WallCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    
    return (Entity);
}

internal add_low_entity_result 
AddStair(mode_world *ModeWorld, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(ModeWorld->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Stairwell, P, ModeWorld->StairCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    Entity.Low->Sim.WalkableDim = Entity.Low->Sim.Collision->TotalVolume.Dim.xy;
    Entity.Low->Sim.WalkableHeight = ModeWorld->TypicalFloorHeight;
    
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
AddSword(mode_world *ModeWorld)
{
    add_low_entity_result Entity = AddLowEntity(ModeWorld, EntityType_Sword, NullPosition());
    Entity.Low->Sim.Collision = ModeWorld->SwordCollision;
    
    AddFlags(&Entity.Low->Sim, EntityFlag_Moveable);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    
    return (Entity);
}

internal add_low_entity_result 
AddPlayer(mode_world *ModeWorld)
{
    world_position P = ModeWorld->Camera.P;
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Hero, P, ModeWorld->PlayerCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);
    
    InitHitPoints(Entity.Low, 3);
    
    add_low_entity_result Sword = AddSword(ModeWorld);
    Entity.Low->Sim.Sword.Index = Sword.LowIndex;
    
    if(ModeWorld->CameraFollowingEntityIndex == 0)
    {
        ModeWorld->CameraFollowingEntityIndex = Entity.LowIndex;
    }
    
    return (Entity);
}

internal add_low_entity_result
AddMonstar(mode_world *ModeWorld, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(ModeWorld->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Monstar, P, ModeWorld->MonstarCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);
    
    InitHitPoints(Entity.Low, 3);
    
    return (Entity);
}

internal add_low_entity_result 
AddFamiliar(mode_world *ModeWorld, u32 AbsTileX, u32 AbsTileY, u32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(ModeWorld->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(ModeWorld, EntityType_Familiar, P, ModeWorld->FamiliarCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);
    
    return (Entity);
}

internal void
UpdateAndRenderWorld(game_memory *Memory, game_input *Input)
{
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
    imgui *ImGuiHandle = &Memory->ImGuiHandle;
#endif
#endif
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    game_assets *Assets = TranState->Assets;
    mode_world *ModeWorld = &GameState->ModeWorld;
    
    renderer_frame *Frame = &Memory->Frame;
    
    if(!ModeWorld->IsInitialized)
    {
        //~ NOTE(ezexff): 
        ModeWorld->ConstArena = &GameState->ConstArena;
        ModeWorld->TranArena = &TranState->TranArena;
        
        //~ NOTE(ezexff): World creation
        {
            AddLowEntity(ModeWorld, EntityType_Null, NullPosition()); // Reserve entity slot 0 for the null entity
            
            ModeWorld->GroundBufferWidth = GameState->GroundBufferWidth;
            ModeWorld->GroundBufferHeight = GameState->GroundBufferHeight;
            ModeWorld->TypicalFloorHeight = GameState->TypicalFloorHeight;
            
            v3 WorldChunkDimInMeters = V3((r32)ModeWorld->GroundBufferWidth,
                                          (r32)ModeWorld->GroundBufferHeight,
                                          ModeWorld->TypicalFloorHeight);
            
            ModeWorld->World = PushStruct(ModeWorld->ConstArena, world);
            InitializeWorld(ModeWorld->World, WorldChunkDimInMeters);
            
            
            u32 TilesPerWidth = 17;
            u32 TilesPerHeight = 9;
            r32 TileSideInMeters = 1.4f;
            r32 TileDepthInMeters = ModeWorld->TypicalFloorHeight;
            
            ModeWorld->NullCollision = MakeNullCollision(ModeWorld);
            //GameState->SwordCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.1f);
            ModeWorld->SwordCollision = MakeSimpleGroundedCollision(ModeWorld, 1.0f, 0.5f, 1.2f);
            ModeWorld->StairCollision = MakeSimpleGroundedCollision(ModeWorld,
                                                                    TileSideInMeters,
                                                                    2.0f * TileSideInMeters,
                                                                    1.1f * TileDepthInMeters);
            ModeWorld->PlayerCollision = MakeSimpleGroundedCollision(ModeWorld, 1.0f, 0.5f, 1.2f);
            ModeWorld->MonstarCollision = MakeSimpleGroundedCollision(ModeWorld, 1.0f, 0.5f, 0.5f);
            ModeWorld->FamiliarCollision = MakeSimpleGroundedCollision(ModeWorld, 1.0f, 0.5f, 0.5f);
            ModeWorld->WallCollision = MakeSimpleGroundedCollision(ModeWorld,
                                                                   TileSideInMeters,
                                                                   TileSideInMeters,
                                                                   TileDepthInMeters);
            /*GameState->StandardRoomCollision = MakeSimpleGroundedCollision(GameState,
                                                                           TilesPerWidth * TileSideInMeters,
                                                                           TilesPerHeight * TileSideInMeters,
                                                                           0.9f * TileDepthInMeters);*/
            
            ModeWorld->StandardRoomCollision = MakeSimpleGroundedCollision(ModeWorld,
                                                                           2 * TilesPerWidth * TileSideInMeters,
                                                                           2 * TilesPerHeight * TileSideInMeters,
                                                                           0.9f * TileDepthInMeters);
            
            ModeWorld->WaterCollision = MakeSimpleGroundedCollision(ModeWorld,
                                                                    (r32)ModeWorld->GroundBufferWidth,
                                                                    (r32)ModeWorld->GroundBufferHeight,
                                                                    0.9f * TileDepthInMeters);
            
            u32 ScreenBaseX = 0;
            u32 ScreenBaseY = 0;
            u32 ScreenBaseZ = 0;
            u32 ScreenX = ScreenBaseX;
            u32 ScreenY = ScreenBaseY;
            //u32 AbsTileX = ScreenX * TilesPerWidth + 1;  // TileX=1
            //u32 AbsTileY = ScreenY * TilesPerHeight + 1; // TileY=2
            u32 AbsTileZ = ScreenBaseZ;
            
            AddStandardRoom(ModeWorld,
                            ScreenX * TilesPerWidth + TilesPerWidth / 2,
                            ScreenY * TilesPerHeight + TilesPerHeight / 2,
                            AbsTileZ);
            // left
            for(u32 Index = 0;
                Index < TilesPerHeight;
                Index++)
            {
                AddWall(ModeWorld, 0, Index, AbsTileZ);
            }
            // top and bot
            for(u32 Index = 1;
                Index < TilesPerWidth;
                Index++)
            {
                AddWall(ModeWorld, Index, TilesPerHeight - 1, AbsTileZ);
                AddWall(ModeWorld, Index, 0, AbsTileZ);
            }
            
            for(u32 RoomIndex = 0;
                RoomIndex < 100;
                RoomIndex++)
            {
                ScreenX++;
                
                AddStandardRoom(ModeWorld,
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
            NewCameraP = ChunkPositionFromTilePosition(ModeWorld->World, CameraTileX, CameraTileY, CameraTileZ);
            ModeWorld->Camera.P = NewCameraP;
            ModeWorld->Camera.Angle.pitch = 0.0f;
            ModeWorld->Camera.Angle.yaw = 0.0f;
            ModeWorld->Camera.Angle.roll = 0.0f;
            
            AddMonstar(ModeWorld, CameraTileX - 3, CameraTileY + 2, CameraTileZ);
            AddMonstar(ModeWorld, CameraTileX, CameraTileY + 2, CameraTileZ);
            AddMonstar(ModeWorld, CameraTileX + 3, CameraTileY + 2, CameraTileZ);
            
            AddWater(ModeWorld, 5, 0, 0);
        }
        
        //~ NOTE(ezexff): Rendering parameters
        {
            // NOTE(ezexff): Camera
            ModeWorld->Camera.Angle.pitch = 90.0f;
            ModeWorld->Camera.Angle.yaw = 0.0f;
        }
        
        ModeWorld->IsInitialized = true;
    }
    
    //~ NOTE(ezexff): Pointers
    renderer *Renderer = (renderer *)Frame->Renderer;
    world *World = ModeWorld->World;
    
    //~ NOTE(ezexff): Keyboard and gamepad inputs
    BEGIN_BLOCK("GameInputProcessing");
    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_hero *ConHero = ModeWorld->ControlledHeroes + ControllerIndex;
        if(ConHero->EntityIndex == 0)
        {
            //if(Controller->Start.EndedDown)
            if(WasPressed(Controller->Start))
            {
                *ConHero = {};
                ConHero->EntityIndex = AddPlayer(ModeWorld).LowIndex;
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                Log->Add("[engineworld] 1st player added\n");
#endif
#endif
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
                // TODO(ezexff): Mb implement jump
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
        if(Input->dMouseP.z != 0 && Input->CenteringMouseCursor)
        {
            Renderer->Camera.P.z -= Input->dMouseP.z;
        }
        
        // TODO(ezexff): Tmp
        local b32 TmpIsCenteringMouseCursor = false;
        
        // NOTE(ezexff): Keys
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
        {
            //Log->Add("[input]: VK_LBUTTON was pressed\n");
        }
        if(WasPressed(Input->MouseButtons[PlatformMouseButton_Middle]))
        {
            TmpIsCenteringMouseCursor = !TmpIsCenteringMouseCursor;
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
        Input->CenteringMouseCursor = TmpIsCenteringMouseCursor;
        if(TmpIsCenteringMouseCursor)
        {
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
            if(!ImGuiHandle->ShowImGuiWindows)
#endif
#endif
            {
                r32 Sensitivity = 0.03;
                
                // Horizontal (camera yaw)
                ModeWorld->Camera.Angle.yaw -= (r32)Input->dMouseP.x * Sensitivity;
                if(ModeWorld->Camera.Angle.yaw < -180)
                {
                    ModeWorld->Camera.Angle.yaw += 360;
                }
                if(ModeWorld->Camera.Angle.yaw > 180)
                {
                    ModeWorld->Camera.Angle.yaw -= 360;
                }
                
                // Vertical (camera pitch)
                ModeWorld->Camera.Angle.pitch += (r32)Input->dMouseP.y * Sensitivity;
                if(ModeWorld->Camera.Angle.pitch < 0)
                {
                    ModeWorld->Camera.Angle.pitch = 0;
                }
                if(ModeWorld->Camera.Angle.pitch > 180)
                {
                    ModeWorld->Camera.Angle.pitch = 180;
                }
                
                // Camera pitch (inverted)
                /* 
                            Frame->CameraPitchInverted -= (r32)Input->MouseDelta.y * Sensitivity;
                            if(Frame->CameraPitchInverted < 0)
                            {
                                Frame->CameraPitchInverted = 0;
                            }
                            if(Frame->CameraPitchInverted > 180)
                            {
                                Frame->CameraPitchInverted = 180;
                            }
                 */
            }
        }
    }
    END_BLOCK();
    
    BEGIN_BLOCK("BeginSim");
    u32 SimChunksInCamera = 5;
    r32 CameraWidthInMeters = SimChunksInCamera * World->ChunkDimInMeters.x;
    r32 CameraHeightInMeters = SimChunksInCamera * World->ChunkDimInMeters.y;
    rectangle3 CameraBoundsInMeters = RectCenterDim(V3(0, 0, 0), V3(CameraWidthInMeters, CameraHeightInMeters, 0.0f));
    
    v3 SimBoundsExpansion = V3(15.0f, 15.0f, 15.0f);
    rectangle3 SimBounds = AddRadiusTo(CameraBoundsInMeters, SimBoundsExpansion);
    
    temporary_memory SimMemory = BeginTemporaryMemory(&TranState->TranArena);
    sim_region *SimRegion = BeginSim(ModeWorld, SimBounds, Input->dtForFrame);
    END_BLOCK();
    
    BEGIN_BLOCK("InitRenderer");
    Renderer->Camera.Angle = ModeWorld->Camera.Angle;
    AddFlags(Renderer,
             RendererFlag_Skybox | 
             RendererFlag_Lighting | 
             RendererFlag_Shadows |
             RendererFlag_Water |
             RendererFlag_Terrain);
    
    //~ TODO(ezexff): Mb rework?
    if(IsSet(Renderer, RendererFlag_Skybox))
    {
        for(u32 Index = 0;
            Index < 6;
            Index++)
        {
            bitmap_id ID = {GetFirstAssetFrom(Assets, Asset_Skybox) + Index};
            Renderer->Skybox->Bitmaps[Index] = *GetBitmap(Assets, ID, true);
        }
    }
    if(IsSet(Renderer, RendererFlag_Lighting))
    {
        Renderer->Lighting->TestSunP = V3(-17.0f, 40.0f, 35.0f);
    }
    if(IsSet(Renderer, RendererFlag_Water))
    {
        renderer_water *Water = Renderer->Water;
        Water->MoveFactor += Water->WaveSpeed * Input->dtForFrame;
        if(Water->MoveFactor >= 1)
        {
            Water->MoveFactor = 0;
        }
        Water->Reflection.DuDv = GetBitmap(Assets, GetFirstBitmapFrom(Assets, Asset_DuDvMap), true);
        Water->Reflection.NormalMap = GetBitmap(Assets, GetFirstBitmapFrom(Assets, Asset_NormalMap), true);
        
        // TODO(ezexff): Test
        /* 
                world_position WaterChunkCenterP = CenteredChunkPoint(Frame->TestWaterP.ChunkX, Frame->TestWaterP.ChunkY, Frame->TestWaterP.ChunkZ);
                Frame->TestWaterRelP = Subtract(World, &WaterChunkCenterP, &ModeWorld->Camera.P);
                 */
        
        world_position SunChunkCenterP = CenteredChunkPoint(5, 0, 0);
        Frame->TestSunRelP = Subtract(World, &SunChunkCenterP, &ModeWorld->Camera.P);
        Frame->TestSunRelP.z = 10.0f;
    }
    
    if(IsSet(Renderer, RendererFlag_Terrain))
    {
        Renderer->Terrain->Bitmap = GetBitmap(Assets, GetFirstBitmapFrom(Assets, Asset_Terrain), true);
    }
    
    //fsdfdsf;
    //Frame->WaterDUDVTextureName = PushString(WorldArena, "NewWaterDUDV.png");
    //Frame->WaterDUDVTexture = LoadTexture(&Render->WaterDUDVTextureName);
    //Frame->WaterNormalMapName = PushString(WorldArena, "NewWaterNormalMap.png");
    //Frame->WaterNormalMap = LoadTexture(&Render->WaterNormalMapName);
    
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
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
#endif
    END_BLOCK();
    
    BEGIN_BLOCK("SimUpdateEntities");
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
                        ControlIndex < ArrayCount(ModeWorld->ControlledHeroes);
                        ++ControlIndex)
                    {
                        controlled_hero *ConHero = ModeWorld->ControlledHeroes + ControlIndex;
                        
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
                            r32 Yaw = DegToRad(ModeWorld->Camera.Angle.yaw);
                            r32 Tmp1 = 90 - ModeWorld->Camera.Angle.pitch;
                            r32 Pitch = DegToRad(Tmp1);
                            //r32 Pitch = DegToRad(GameState->CameraPitch);
                            //Log->Add("test %f\n", Pitch);
                            r32 Roll = DegToRad(ModeWorld->Camera.Angle.roll);
                            
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
                                    AddCollisionRule(ModeWorld, Sword->StorageIndex, Entity->StorageIndex, false);
                                    
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
                        ClearCollisionRulesFor(ModeWorld, Entity->StorageIndex);
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
                        ClearCollisionRulesFor(ModeWorld, Entity->StorageIndex);
                        MakeEntityNonSpatial(Entity);
                    }
                } break;
            }
            
            if(!IsSet(Entity, EntityFlag_Nonspatial) &&
               IsSet(Entity, EntityFlag_Moveable))
            {
                MoveEntity(ModeWorld, SimRegion, Entity, Input->dtForFrame, &MoveSpec, ddP,
                           WishDir, WishSpeed, sv_accelerate);
            }
            
            //RenderGroup->Transform.OffsetP = GetEntityGroundPoint(Entity);
            //Frame->OffsetP = GetEntityGroundPoint(Entity);
            v3 InCameraSpaceEntityP = GetEntityGroundPoint(Entity);
            
            // NOTE(ezexff): Calc entity z based on terrain height
            if(Entity->Type == EntityType_Wall || 
               Entity->Type == EntityType_Monstar ||
               Entity->Type == EntityType_Hero)
            {
                low_entity *LowEntity = GetLowEntity(ModeWorld, Entity->StorageIndex);
                world_position WorldEntityP = LowEntity->P;
                
                renderer_terrain *Terrain = Renderer->Terrain;
                r32 RelTileX = Terrain->TileCount * WorldEntityP.ChunkX + 
                (ModeWorld->GroundBufferWidth) + WorldEntityP.Offset_.x;
                r32 RelTileY = Terrain->TileCount * WorldEntityP.ChunkY +
                (ModeWorld->GroundBufferHeight) + WorldEntityP.Offset_.y;
                s32 TileX = FloorR32ToS32(RelTileX);
                s32 TileY = FloorR32ToS32(RelTileY);
                s32 TileZ = 0; // TODO(ezexff): Need rework when start using z chunks
                
                random_series Series = RandomSeed(139 * (TileX) + 593 * (TileY) + 329 * TileZ);
                r32 RandomZ00 = RandomUnilateral(&Series) * Terrain->MaxHeight;
                Series = RandomSeed(139 * (TileX + 1) + 593 * (TileY) + 329 * TileZ);
                r32 RandomZ10 = RandomUnilateral(&Series) * Terrain->MaxHeight;
                Series = RandomSeed(139 * (TileX) + 593 * (TileY + 1) + 329 * TileZ);
                r32 RandomZ01 = RandomUnilateral(&Series) * Terrain->MaxHeight;
                Series = RandomSeed(139 * (TileX + 1) + 593 * (TileY + 1) + 329 * TileZ);
                r32 RandomZ11 = RandomUnilateral(&Series) * Terrain->MaxHeight;
                
                r32 BaseOffsetX = RelTileX - TileX;
                r32 BaseOffsetY = RelTileY - TileY;
                r32 H1 = Lerp(RandomZ00, BaseOffsetX, RandomZ10); // x-axis
                r32 H2 = Lerp(RandomZ01, BaseOffsetX, RandomZ11); // y-axis
                r32 OnTerrainZ = (H1, BaseOffsetY, H2);
                InCameraSpaceEntityP.z = OnTerrainZ;
                
                r32 HillHeight = 0.0f;
                {
                    s32 HillTileX = 8;
                    s32 HillTileY = 8;
                    s32 HillZ = 2;
                    r32 HillRadius = 8.0f;
                    if((WorldEntityP.ChunkX == 4) && (WorldEntityP.ChunkY == 0) && (WorldEntityP.ChunkZ == 0))
                    {
                        s32 ChunkTileX = TileX - WorldEntityP.ChunkX * Terrain->TileCount;
                        s32 ChunkTileY = TileY - WorldEntityP.ChunkY * Terrain->TileCount;
                        r32 T1 = (r32)(HillTileX - ChunkTileX);
                        r32 T2 = (r32)(HillTileY - ChunkTileY);
                        r32 Length = SquareRoot(Square(T1) + Square(T2));
                        if(Length < HillRadius)
                        {
                            Length = Length / HillRadius * (r32)Pi32_2;
                            HillHeight = Cos(Length) * HillZ;
                        }
                    }
                    HillZ = -2;
                    if((WorldEntityP.ChunkX == 5) && (WorldEntityP.ChunkY == 0) && (WorldEntityP.ChunkZ == 0))
                    {
                        s32 ChunkTileX = TileX - WorldEntityP.ChunkX * Terrain->TileCount;
                        s32 ChunkTileY = TileY - WorldEntityP.ChunkY * Terrain->TileCount;
                        r32 T1 = (r32)(HillTileX - ChunkTileX);
                        r32 T2 = (r32)(HillTileY - ChunkTileY);
                        r32 Length = SquareRoot(Square(T1) + Square(T2));
                        if(Length < HillRadius)
                        {
                            Length = Length / HillRadius * (r32)Pi32_2;
                            HillHeight = Cos(Length) * HillZ;
                        }
                    }
                }
                
                // NOTE(ezexff): Terrain z for player camera
                if(Entity->Type == EntityType_Hero && Frame->FixCameraOnTerrain)
                {
#define PLAYER_EYE_HEIGHT_FROM_GROUND 1.75f // TODO(ezexff): Replace
                    Renderer->Camera.P.z = OnTerrainZ + HillHeight + PLAYER_EYE_HEIGHT_FROM_GROUND;
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
                        PushRectOutlineOnGround(Frame, InCameraSpaceEntityP.xy, Volume->Dim.xy, V4(0, 1, 0, 1));
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
                        InCameraSpaceEntityP.z += Volume->OffsetP.z;
                        PushCube(Frame, InCameraSpaceEntityP, Volume->Dim, V4(1, 0.5f, 0, 1));
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
                        PushRectOutlineOnGround(Frame, InCameraSpaceEntityP.xy, Volume->Dim.xy, V4(0, 0, 1, 1));
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
                        InCameraSpaceEntityP.z += Volume->OffsetP.z;
                        PushCubeOutline(Frame, InCameraSpaceEntityP, Volume->Dim, V4(1, 0, 0, 1));
                    }
                    
                    /*ImGui::Begin("Monstar");
                    ImGui::Text("HitPoints = %d", Entity->HitPointMax);
                    ImGui::End();*/
                } break;
                
                case EntityType_Space:
                {
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                    if(ImGuiHandle->DrawSpaceBounds)
                    {
                        for(u32 VolumeIndex = 0;
                            VolumeIndex < Entity->Collision->VolumeCount;
                            ++VolumeIndex)
                        {
                            sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                            //PushRectOnGround(Frame, Frame->OffsetP.xy, Volume->Dim.xy, V4(0.5f, 0.5f, 0.5f, 1));
                            PushRectOutlineOnGround(Frame, InCameraSpaceEntityP.xy, Volume->Dim.xy, V4(1, 1, 1, 1));
                        }
                    }
#endif
#endif
                } break;
                
                case EntityType_Water:
                {
                    for(u32 VolumeIndex = 0;
                        VolumeIndex < Entity->Collision->VolumeCount;
                        ++VolumeIndex)
                    {
                        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                        //PushRectOutlineOnGround(Frame, Frame->OffsetP.xy, Volume->Dim.xy, V4(0, 1, 0, 1));
                        PushWater(Frame, InCameraSpaceEntityP, Volume->Dim.xy);
                    }
                } break;
                
                InvalidDefaultCase;
            }
        }
    }
    END_BLOCK();
    
    // NOTE(ezexff): Fill ground buffer
#if 1
    BEGIN_BLOCK("FillGroundBuffer");
    world_position MinChunkP = MapIntoChunkSpace(World, ModeWorld->Camera.P, GetMinCorner(CameraBoundsInMeters));
    world_position MaxChunkP = MapIntoChunkSpace(World, ModeWorld->Camera.P, GetMaxCorner(CameraBoundsInMeters));
    
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
                v3 RelP = Subtract(World, &ChunkCenterP, &ModeWorld->Camera.P);
                
                u32 TmpGroundBufferIndex = 0;
                // TODO(casey): This is super inefficient fix it!
                r32 FurthestBufferLengthSq = 0.0f;
                ground_buffer *FurthestBuffer = 0;
                for(u32 GroundBufferIndex = 0;
                    GroundBufferIndex < Renderer->Terrain->GroundBufferArray.Count;
                    ++GroundBufferIndex)
                {
                    ground_buffer *GroundBuffer = Renderer->Terrain->GroundBufferArray.Buffers + GroundBufferIndex;
                    if(AreInSameChunk(World, &GroundBuffer->P, &ChunkCenterP))
                    {
                        TmpGroundBufferIndex = 0;
                        FurthestBuffer = 0;
                        break;
                    }
                    else if(IsValid(GroundBuffer->P))
                    {
                        v3 RelP = Subtract(World, &GroundBuffer->P, &ModeWorld->Camera.P);
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
                    FillGroundChunk(Renderer->Terrain, FurthestBuffer, &ChunkCenterP);
                }
            }
        }
    }
    END_BLOCK();
    
    // NOTE(ezexff): Render Ground Buffers
    BEGIN_BLOCK("RenderGroundBuffers");
    {
        for(u32 GroundBufferIndex = 0;
            GroundBufferIndex < Renderer->Terrain->GroundBufferArray.Count;
            ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = Renderer->Terrain->GroundBufferArray.Buffers + GroundBufferIndex;
            if(IsValid(GroundBuffer->P))
            {
                GroundBuffer->OffsetP = Subtract(ModeWorld->World, &GroundBuffer->P, &ModeWorld->Camera.P);
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
                if(ImGuiHandle->DrawGroundBufferBounds)
                {
                    PushRectOutlineOnGround(Frame, GroundBuffer->OffsetP.xy, V2(8, 8), V4(0, 0, 1, 1));
                }
#endif
#endif
                // NOTE(ezexff): From center pos to left bottom pos for proper rendering
                GroundBuffer->OffsetP.x -= 0.5f * ModeWorld->GroundBufferWidth;
                GroundBuffer->OffsetP.y -= 0.5f * ModeWorld->GroundBufferHeight;
#if 0                
                // NOTE(ezexff): Draw terrain chunk normals
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
#endif
            }
        }
    }
#endif
    END_BLOCK();
    
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
    
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
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
            
            if(ImGui::CollapsingHeader("EntityList"))
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
                u32 EntityTypesCount = ArrayCount(EntityTypes);
                Assert(EntityTypesCount == EntityType_Count);
                
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
#endif
    
    // TODO(ezexff): Mb rework text output and add kerning
    /* 
        if(Frame->DrawDebugTextLine)
        {
            char *TestString = "The quick brown fox jumps over a lazy dog.";
            DEBUGTextLine(Frame, TranState->Assets, TestString);
        }
     */
    /*PushRectOnGround(Frame, V3(0, 0, 0), V2(5, 5), V4(0, 1, 0, 1));
    PushRectOutlineOnGround(Frame, V3(0, 7, 0), V2(5, 5), V4(0, 0, 1, 1), 0.5f);
    PushBitmapOnGround(Frame, Assets, GetFirstBitmapFrom(Assets, Asset_Ground), V3(0, -7, 0), V2(5, 5), 4.0f);*/
    
    BEGIN_BLOCK("EndSim")
        EndSim(SimRegion, ModeWorld);
    EndTemporaryMemory(SimMemory);
    END_BLOCK();
    
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
    {
        // NOTE(ezexff): Player pos, angle, velocity
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Appearing);
        //ImGui::SetNextWindowSize(ImVec2(200, 100));
        
        static ImGuiWindowFlags Flags = ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
        ImGui::Begin("Player", NULL, Flags);
        /* 
                renderer_camera *Camera = (renderer_camera *)Frame->Camera;
                
                world_position *CameraP = &GameState->Camera.P;
                ImGui::Text("WorldP: %d %d %d", CameraP->ChunkX, CameraP->ChunkY, CameraP->ChunkZ);
                ImGui::Text("ChunkP: %.3f %.3f %.3f", CameraP->Offset_.x, CameraP->Offset_.y, CameraP->Offset_.z);
                ImGui::Text("RenderWorldOrigin: %.3f %.3f %.3f", Renderer->CameraP.x, Renderer->CameraP.y, Renderer->CameraP.z);
                ImGui::Text("Ang: %.3f %.3f %.3f", Camera->Angle.x, Camera->Angle.y, Camera->Angle.z);
                ImGui::Text("Fps: %.1f", 1.0f / Input->dtForFrame);
                ImGui::Text("dt: %.6f", Input->dtForFrame);
                 */
        
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
#endif
}