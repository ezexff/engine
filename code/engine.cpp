#include "engine.h"
#include "engine_asset.cpp"
#include "engine_world.cpp"
#include "engine_sim_region.cpp"
#include "engine_entity.cpp"
#include "engine_render.cpp"
#include "engine_render_group.cpp"

#include "engine_random.h"

/* TODO(me): TEST
struct tile_render_work
{
    render_group *RenderGroup;
    loaded_bitmap *OutputTarget;
    rectangle2i ClipRect;
};

internal PLATFORM_WORK_QUEUE_CALLBACK(DoTestWork)
{
    tile_render_work *Work = (tile_render_work *)Data;

    RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, true);
    RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, false);
}*/

internal PLATFORM_WORK_QUEUE_CALLBACK(DoWorkerWork2)
{
    Log.AddLog("[thread] %u: %s\n", GetCurrentThreadId(), (char *)Data);
}

internal loaded_model * //
CreateTerrainChunkModel(memory_arena *WorldArena, u32 Width, u32 Height)
{
    loaded_model *Result = PushStruct(WorldArena, loaded_model);

    // Для включения в просчёты максимального значения
    Width = Width + 1;
    Height = Height + 1;

    Result->Name = PushString(WorldArena, "TerrainModel");

    Result->MeshesCount = 1;

    Result->Meshes = PushArray(WorldArena, Result->MeshesCount, single_mesh);

    single_mesh *Mesh = &Result->Meshes[0];

    Mesh->Name = PushString(WorldArena, "TerrainMesh");

    Mesh->VerticesCount = Width * Height;

    Mesh->Positions = PushArray(WorldArena, Mesh->VerticesCount, v3);
    Mesh->TexCoords = PushArray(WorldArena, Mesh->VerticesCount, v2);

    /*int VertexIndex = 0;
    for(u32 i = 0; i < Width; i++)
    {
        for(u32 j = 0; j < Height; j++)
        {
            Mesh->Positions[VertexIndex] = V3((r32)i, (r32)j, (rand() % 10) * 0.02f);
            Mesh->TexCoords[VertexIndex] = V2((r32)i, (r32)j);
            VertexIndex++;
        }
    }*/
    int VertexIndex = 0;
    for(u32 Y = 0;  //
        Y < Height; //
        Y++)
    {
        for(u32 X = 0; //
            X < Width; //
            X++, VertexIndex++)
        {
            Mesh->Positions[VertexIndex] = V3((r32)X, (r32)Y, 0.0f);
            // Mesh->Positions[VertexIndex] = V3((r32)X, (r32)Y, (rand() % 10) * 0.02f);
            Mesh->TexCoords[VertexIndex] = V2((r32)X, (r32)Y);
        }
    }
    /*for(u32 i = 0; i < Width; i++)
    {
        for(u32 j = 0; j < Height; j++)
        {
            Mesh->Positions[VertexIndex] = V3((r32)i, (r32)j, (rand() % 10) * 0.02f);
            Mesh->TexCoords[VertexIndex] = V2((r32)i, (r32)j);
            VertexIndex++;
        }
    }*/

    Mesh->IndicesCount = (Width - 1) * (Height - 1) * 6;
    Mesh->Indices = PushArray(WorldArena, Mesh->IndicesCount, u32);

    for(u32 i = 0; i < Width - 1; i++)
    {
        u32 Pos = i * Height; // номер ячейки массива использующий сквозную нумерацию
        for(u32 j = 0; j < Height - 1; j++)
        {
            // Flat[ x * Height * depth + y * depth + z ] = elements[x][y][z]
            u32 TmpIndex = i * (Height - 1) * 6 + j * 6;

            // первый треугольник на плоскости (левая верхняя часть квадрата)
            Mesh->Indices[TmpIndex + 0] = Pos;
            Mesh->Indices[TmpIndex + 1] = Pos + 1; // переход к следующей вершине (перемещение по оси y)
            Mesh->Indices[TmpIndex + 2] =
                Pos + 1 + Height; // переход к вершине во второй размерности (перемещение по оси x)

            // второй треугольник на плоскости (правая нижняя часть квадрата)
            Mesh->Indices[TmpIndex + 3] = Pos + 1 + Height;
            Mesh->Indices[TmpIndex + 4] = Pos + Height;
            Mesh->Indices[TmpIndex + 5] = Pos;
            Pos++;
        }
    }

    // касательные и бикасательные для маппинга нормалей
    // Mesh->Tangents = PushArray(WorldArena, Mesh->VerticesCount, v3);
    // fread(Mesh->Tangents, sizeof(v3) * Mesh->VerticesCount, 1, In);

    // создание холмов
    /*for(u32 i = 0; i < 10; i++)
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
    }*/

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
    Mesh->Material.Texture = glLoadTexture(WorldArena, "pole.png");

    return (Result);
}

internal void                                                      //
FillGroundChunk(transient_state *TranState, game_state *GameState, //
                ground_buffer *GroundBuffer,                       //
                world_position *ChunkP)
{
#if 1
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
    temporary_memory GroundMemory = BeginTemporaryMemory(&TranState->TranArena);
    GroundBuffer->P = *ChunkP;

    loaded_texture *Buffer = &GroundBuffer->DrawBuffer;
    r32 Width = (r32)Buffer->Width;
    r32 Height = (r32)Buffer->Height;
    v2 HalfDim = 0.5f * V2(Width, Height);

    render_group *RenderGroup = AllocateRenderGroup(&TranState->TranArena, Megabytes(4), //
                                                    0, 0, 0, true);

    glBindFramebuffer(GL_FRAMEBUFFER, Buffer->FBO);

    // Create Render Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Buffer->Texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Buffer->Width, Buffer->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Buffer->Width, Buffer->Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Buffer->Texture, 0);

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

            PushTexture(RenderGroup, V3(Center, 0), V2(Width, Height), GameState->TestTexture1.Texture, //
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
    }

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

    RenderGroupToOutput(RenderGroup, Buffer);
    EndTemporaryMemory(GroundMemory);
#endif
}

inline world_position //
ChunkPositionFromTilePosition(world *World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ,
                              v3 AdditionalOffset = V3(0, 0, 0))
{
    world_position BasePos = {};

    r32 TileSideInMeters = 1.4f;
    r32 TileDepthInMeters = 3.0f;

    v3 TileDim = V3(TileSideInMeters, TileSideInMeters, TileDepthInMeters);
    v3 Offset = Hadamard(TileDim, V3((real32)AbsTileX, (real32)AbsTileY, (real32)AbsTileZ));
    world_position Result = MapIntoChunkSpace(World, BasePos, AdditionalOffset + Offset);

    Assert(IsCanonical(World, Result.Offset_));

    return (Result);
}

struct add_low_entity_result
{
    low_entity *Low;
    uint32 LowIndex;
};
internal add_low_entity_result //
AddLowEntity(game_state *GameState, entity_type Type, world_position P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
    uint32 EntityIndex = GameState->LowEntityCount++;

    low_entity *EntityLow = GameState->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Sim.Type = Type;
    EntityLow->Sim.Collision = GameState->NullCollision;
    EntityLow->P = NullPosition();

    ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, P);

    add_low_entity_result Result;
    Result.Low = EntityLow;
    Result.LowIndex = EntityIndex;

    // TODO(casey): Do we need to have a begin/end paradigm for adding
    // entities so that they can be brought into the high set when they
    // are added and are in the camera region?

    return (Result);
}

internal add_low_entity_result AddGroundedEntity(game_state *GameState, entity_type Type, world_position P,
                                                 sim_entity_collision_volume_group *Collision)
{
    add_low_entity_result Entity = AddLowEntity(GameState, Type, P);
    Entity.Low->Sim.Collision = Collision;
    return (Entity);
}

internal add_low_entity_result AddStandardRoom(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Space, P, GameState->StandardRoomCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Traversable);

    return (Entity);
}

internal add_low_entity_result AddWall(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Wall, P, GameState->WallCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);

    return (Entity);
}

internal add_low_entity_result AddStair(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Stairwell, P, GameState->StairCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    Entity.Low->Sim.WalkableDim = Entity.Low->Sim.Collision->TotalVolume.Dim.xy;
    Entity.Low->Sim.WalkableHeight = GameState->TypicalFloorHeight;

    return (Entity);
}

internal void InitHitPoints(low_entity *EntityLow, uint32 HitPointCount)
{
    Assert(HitPointCount <= ArrayCount(EntityLow->Sim.HitPoint));
    EntityLow->Sim.HitPointMax = HitPointCount;
    for(uint32 HitPointIndex = 0;                   //
        HitPointIndex < EntityLow->Sim.HitPointMax; //
        ++HitPointIndex)
    {
        hit_point *HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
        HitPoint->Flags = 0;
        HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
    }
}

internal add_low_entity_result AddSword(game_state *GameState)
{
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());
    Entity.Low->Sim.Collision = GameState->SwordCollision;

    AddFlags(&Entity.Low->Sim, EntityFlag_Moveable);

    return (Entity);
}

internal add_low_entity_result AddPlayer(game_state *GameState)
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

internal add_low_entity_result //
AddMonstar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Monstar, P, GameState->MonstarCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);

    InitHitPoints(Entity.Low, 3);

    return (Entity);
}

internal add_low_entity_result AddFamiliar(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Familiar, P, GameState->FamiliarCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Moveable);

    return (Entity);
}

internal void //
ClearCollisionRulesFor(game_state *GameState, uint32 StorageIndex)
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
    for(uint32 HashBucket = 0;                                 //
        HashBucket < ArrayCount(GameState->CollisionRuleHash); //
        ++HashBucket)
    {
        for(pairwise_collision_rule **Rule = &GameState->CollisionRuleHash[HashBucket]; //
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

internal void //
AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 CanCollide)
{
    // TODO(casey): Collapse this with ShouldCollide
    if(StorageIndexA > StorageIndexB)
    {
        uint32 Temp = StorageIndexA;
        StorageIndexA = StorageIndexB;
        StorageIndexB = Temp;
    }

    // TODO(casey): BETTER HASH FUNCTION
    pairwise_collision_rule *Found = 0;
    uint32 HashBucket = StorageIndexA & (ArrayCount(GameState->CollisionRuleHash) - 1);
    for(pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket]; //
        Rule;                                                                     //
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
            Found = PushStruct(&GameState->WorldArena, pairwise_collision_rule);
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

sim_entity_collision_volume_group * //
MakeSimpleGroundedCollision(game_state *GameState, real32 DimX, real32 DimY, real32 DimZ)
{
    // TODO(casey): NOT WORLD ARENA!  Change to using the fundamental types arena, etc.
    sim_entity_collision_volume_group *Group = PushStruct(&GameState->WorldArena, sim_entity_collision_volume_group);
    Group->VolumeCount = 1;
    Group->Volumes = PushArray(&GameState->WorldArena, Group->VolumeCount, sim_entity_collision_volume);
    Group->TotalVolume.OffsetP = V3(0, 0, 0.5f * DimZ);
    Group->TotalVolume.Dim = V3(DimX, DimY, DimZ);
    Group->Volumes[0] = Group->TotalVolume;

    return (Group);
}

sim_entity_collision_volume_group * //
MakeNullCollision(game_state *GameState)
{
    // TODO(casey): NOT WORLD ARENA!  Change to using the fundamental types arena, etc.
    sim_entity_collision_volume_group *Group = PushStruct(&GameState->WorldArena, sim_entity_collision_volume_group);
    Group->VolumeCount = 0;
    Group->Volumes = 0;
    Group->TotalVolume.OffsetP = V3(0, 0, 0);
    // TODO(casey): Should this be negative?
    Group->TotalVolume.Dim = V3(0, 0, 0);

    return (Group);
}

/*internal void DrawCollisionRect(sim_entity *Entity, r32 Z, v3 Color)
{
    v3 EntityBaseP = GetEntityGroundPoint(Entity);
    for(uint32 VolumeIndex = 0;                       //
        VolumeIndex < Entity->Collision->VolumeCount; //
        ++VolumeIndex)
    {
        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
        v2 CollisionMinP = V2(EntityBaseP.x - 0.5f * Volume->Dim.x, EntityBaseP.y - 0.5f * Volume->Dim.y);
        v2 CollisionMaxP = V2(EntityBaseP.x + 0.5f * Volume->Dim.x, EntityBaseP.y + 0.5f * Volume->Dim.y);
        DrawRectangle({CollisionMinP, CollisionMaxP}, Z, Color);
    }
}*/

/*internal void DrawCollisionRectOutline(sim_entity *Entity, r32 Z, v3 Color)
{
    v3 EntityBaseP = GetEntityGroundPoint(Entity);
    for(uint32 VolumeIndex = 0;                       //
        VolumeIndex < Entity->Collision->VolumeCount; //
        ++VolumeIndex)
    {
        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
        v2 CollisionMinP = V2(EntityBaseP.x - 0.5f * Volume->Dim.x, EntityBaseP.y - 0.5f * Volume->Dim.y);
        v2 CollisionMaxP = V2(EntityBaseP.x + 0.5f * Volume->Dim.x, EntityBaseP.y + 0.5f * Volume->Dim.y);
        DrawRectangleOutline({CollisionMinP, CollisionMaxP}, Z, Color);
    }
}*/

/*internal void //
DrawHitpoints(sim_entity *Entity)
{
    v3 EntityBaseP = GetEntityGroundPoint(Entity);
    if(Entity->HitPointMax >= 1)
    {
#if 1
        v2 HealthDim = {0.2f, 0.2f};
        r32 SpacingX = 1.5f * HealthDim.x;

        v2 HitP = EntityBaseP.xy;
        HitP.x -= SpacingX * (Entity->HitPointMax - 1) - HealthDim.x;
        HitP.y -= 0.5f;
        v2 dHitP = V2(SpacingX, 0.0f);

        for(uint32 HealthIndex = 0;            //
            HealthIndex < Entity->HitPointMax; //
            ++HealthIndex)
        {
            hit_point *HitPoint = Entity->HitPoint + HealthIndex;
            v4 Color = {1.0f, 0.0f, 0.0f, 1.0f};
            if(HitPoint->FilledAmount == 0)
            {
                Color = V4(0.2f, 0.2f, 0.2f, 1.0f);
            }

            DrawRectangle({HitP, HitP + HealthDim}, 0.1f, V3(1, 0, 0));
            HitP += dHitP;
        }

#else
        v2 HealthDim = {0.2f, 0.2f};
        real32 SpacingX = 1.5f * HealthDim.x;
        v2 HitP = {-0.5f * (Entity->HitPointMax - 1) * SpacingX, -0.25f};
        v2 dHitP = {SpacingX, 0.0f};
        for(uint32 HealthIndex = 0;            //
            HealthIndex < Entity->HitPointMax; //
            ++HealthIndex)
        {
            hit_point *HitPoint = Entity->HitPoint + HealthIndex;
            v4 Color = {1.0f, 0.0f, 0.0f, 1.0f};
            if(HitPoint->FilledAmount == 0)
            {
                Color = V4(0.2f, 0.2f, 0.2f, 1.0f);
            }

            // PushRect(PieceGroup, HitP, 0, HealthDim, Color, 0.0f);
            DrawRectangle({EntityBaseP.xy + HitP, EntityBaseP.xy + HitP + HealthDim}, 0.1f, V3(1, 0, 0));
            HitP += dHitP;
        }
#endif
    }
}*/

#if ENGINE_INTERNAL
game_memory *DebugGlobalMemory;
#endif
internal void //
EngineUpdateAndRender(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
{
#if ENGINE_INTERNAL
    DebugGlobalMemory = Memory;
#endif
    BEGIN_TIMED_BLOCK(EngineUpdateAndRender);

    Platform = Memory->PlatformAPI;

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));

    // u32 GroundBufferWidth = 256;
    // u32 GroundBufferHeight = 256;
    u32 GroundBufferWidth = 8;
    u32 GroundBufferHeight = 8;

    //
    // NOTE(me): Permanent initialization
    //
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!GameState->IsInitialized)
    {
        // установка указателя выделенной памяти после структуры game_state
        InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *)Memory->PermanentStorage + sizeof(game_state));
        memory_arena *WorldArena = &GameState->WorldArena;

        // NOTE(casey): Reserve entity slot 0 for the null entity
        AddLowEntity(GameState, EntityType_Null, NullPosition());

        GameState->TypicalFloorHeight = 3.0f;
        // GameState->MetersToPixels = 42.0f;
        // GameState->PixelsToMeters = 1.0f / GameState->MetersToPixels;

        // v3 WorldChunkDimInMeters = V3(GameState->PixelsToMeters * (r32)GroundBufferWidth,  //
        //                               GameState->PixelsToMeters * (r32)GroundBufferHeight, //
        //                               GameState->TypicalFloorHeight);
        v3 WorldChunkDimInMeters = V3((r32)GroundBufferWidth,  //
                                      (r32)GroundBufferHeight, //
                                      GameState->TypicalFloorHeight);

        GameState->World = PushStruct(&GameState->WorldArena, world);
        world *World = GameState->World;
        InitializeWorld(World, WorldChunkDimInMeters);

        uint32 TilesPerWidth = 17;
        uint32 TilesPerHeight = 9;
        r32 TileSideInMeters = 1.4f;
        r32 TileDepthInMeters = GameState->TypicalFloorHeight;

        GameState->NullCollision = MakeNullCollision(GameState);
        GameState->SwordCollision = MakeSimpleGroundedCollision(GameState, //
                                                                1.0f, 0.5f, 0.1f);
        GameState->StairCollision = MakeSimpleGroundedCollision(GameState,               //
                                                                TileSideInMeters,        //
                                                                2.0f * TileSideInMeters, //
                                                                1.1f * TileDepthInMeters);
        GameState->PlayerCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 1.2f);
        GameState->MonstarCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
        GameState->FamiliarCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
        GameState->WallCollision = MakeSimpleGroundedCollision(GameState,        //
                                                               TileSideInMeters, //
                                                               TileSideInMeters, //
                                                               TileDepthInMeters);
        GameState->StandardRoomCollision = MakeSimpleGroundedCollision(GameState,                         //
                                                                       TilesPerWidth * TileSideInMeters,  //
                                                                       TilesPerHeight * TileSideInMeters, //
                                                                       0.9f * TileDepthInMeters);

        uint32 ScreenBaseX = 0;
        uint32 ScreenBaseY = 0;
        uint32 ScreenBaseZ = 0;
        uint32 ScreenX = ScreenBaseX;
        uint32 ScreenY = ScreenBaseY;
        uint32 AbsTileX = ScreenX * TilesPerWidth + 1;  // TileX=1
        uint32 AbsTileY = ScreenY * TilesPerHeight + 1; // TileY=2
        uint32 AbsTileZ = ScreenBaseZ;
        AddStandardRoom(GameState,                                     //
                        ScreenX * TilesPerWidth + TilesPerWidth / 2,   //
                        ScreenY * TilesPerHeight + TilesPerHeight / 2, //
                        AbsTileZ);
        ScreenX++;
        AddStandardRoom(GameState,                                     //
                        ScreenX * TilesPerWidth + TilesPerWidth / 2,   //
                        ScreenY * TilesPerHeight + TilesPerHeight / 2, //
                        AbsTileZ);
        ScreenY++;
        AddStandardRoom(GameState,                                     //
                        ScreenX * TilesPerWidth + TilesPerWidth / 2,   //
                        ScreenY * TilesPerHeight + TilesPerHeight / 2, //
                        AbsTileZ);
        ScreenY--;
        ScreenX++;
        AddStandardRoom(GameState,                                     //
                        ScreenX * TilesPerWidth + TilesPerWidth / 2,   //
                        ScreenY * TilesPerHeight + TilesPerHeight / 2, //
                        AbsTileZ);
        AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);

        world_position NewCameraP = {};
        uint32 CameraTileX = ScreenBaseX * TilesPerWidth + 17 / 2;
        uint32 CameraTileY = ScreenBaseY * TilesPerHeight + 9 / 2;
        uint32 CameraTileZ = ScreenBaseZ;
        NewCameraP = ChunkPositionFromTilePosition(World, CameraTileX, CameraTileY, CameraTileZ);
        GameState->CameraP = NewCameraP;
        // GameState->CameraPitch = 90.0f;
        // GameState->CameraYaw = -45.0f;
        // GameState->CameraRenderZ = 1.7f;
        GameState->CameraPitch = 0.0f;
        GameState->CameraYaw = 0.0f;
        GameState->CameraRenderZ = 50.0f;

        AddMonstar(GameState, CameraTileX - 3, CameraTileY + 2, CameraTileZ);

        GameState->Debug = PushStruct(WorldArena, game_debug);
        game_debug *Debug = GameState->Debug;
        Debug->ProcessAnimations = true;
        Debug->DrawSimRegionBounds = false;
        Debug->DrawSimRegionUpdatableBounds = false;
        Debug->DrawSimChunks = true;
        Debug->DrawChunkWhereCamera = false;
        Debug->DrawPlayerHitbox = true;
        Debug->LogCycleCounters = false;
        Debug->GroundBufferIndex = 0;

        GameState->Settings = PushStruct(WorldArena, app_settings);
        app_settings *Settings = GameState->Settings;
        Settings->RBFullscreenIsActive = false;
        Settings->RBWindowedIsActive = true;
        Settings->MouseSensitivity = 25.0;
        Settings->NewFrameRate = 60;
        Settings->RBCappedIsActive = true;
        Settings->RBUncappedIsActive = false;
        Settings->RBVSyncIsActive = false;

        GameState->Render = PushStruct(WorldArena, render);
        render *Render = GameState->Render;
        Render->FOV = 0.1f;
        Render->SStMeshesCount = 0;
        Render->SAnMeshesCount = 0;
        Render->MStMeshesCount = 0;
        Render->GrMeshesCount = 0;
        Render->Animator.Timer = 1.0f;

        // TODO(me): remove
        // GameState->TestTexture1 = LoadTexture(&GameState->TestTexture1Name);
        GameState->TestTexture1 = glLoadTexture(WorldArena, "pole.png");

        glGenFramebuffers(1, &Buffer->FBO);
        glGenTextures(1, &Buffer->DrawTexture);
        glGenTextures(1, &Buffer->DepthTexture);

        glGenFramebuffers(1, &Buffer->GroundFBO);

        /*GameState->Player = PushStruct(WorldArena, entity_player);
        entity_player *Player = GameState->Player;
        // Player->Position = V3(5, 5, 0);
        Player->P.ChunkX = -2;
        Player->P.ChunkY = -2;
        Player->P.Offset_ = V3(0, 0, 0);
        Player->TmpZ = 2.7f; // высота камеры
        // Player->dP = V3(0, 0, 0);
        Player->CameraPitch = 90.0f;
        Player->CameraYaw = -45.0f;
        Player->CameraZOffset = 2.7f;
        Player->Width = 0.5f;
        Player->Height = 0.5f;
        Player->CameraPitchInversed = Player->CameraPitch;

        GameState->Clip = PushStruct(WorldArena, entity_clip);
        entity_clip *PlayerClip = GameState->Clip;
        // PlayerClip->CenterPos = V2(-50.0f, 50.0f);
        PlayerClip->P.ChunkX = -5;
        PlayerClip->P.ChunkX = -5;
        PlayerClip->Side = 50.0f;*/

        //
        // NOTE(me): Источники света
        //
        // directional light
        /*directional_light *DirLight = &Render->DirLight;
        DirLight->Base.Color = V3(0.5f, 0.5f, 0.5f);
        DirLight->Base.AmbientIntensity = 0.1f;
        DirLight->Base.DiffuseIntensity = 1.0f;
        DirLight->WorldDirection = V3(1.0f, 1.0f, -1.0f);
        // DirLight->WorldDirection = V3(3.0f, 6.0f, -73.5f);

        // point lights
        Render->PointLightsCount = 1;
        Render->PointLights = PushArray(WorldArena, Render->PointLightsCount, point_light);
        point_light *PointLights = Render->PointLights;
        PointLights[0].Base.Color = V3(0.0f, 0.0f, 1.0f);
        PointLights[0].Base.AmbientIntensity = 0.0f;
        PointLights[0].Base.DiffuseIntensity = 1.0f;
        PointLights[0].WorldPosition = V3(4.5f, 9.0f, 3.0f);
        PointLights[0].Atten.Constant = 0.1f; // 1.0f
        PointLights[0].Atten.Linear = 0.1f;   // 0.1f, 0.0f
        PointLights[0].Atten.Exp = 0.0f;      // 0.0f
        // имена переменных point lights для оправки в шейдер
        Render->PLVarNames = PushArray(WorldArena, Render->PointLightsCount, point_light_var_names);
        for(u32 i = 0; i < Render->PointLightsCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.Color", i);
            Render->PLVarNames[i].VarNames[0] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.AmbientIntensity", i);
            Render->PLVarNames[i].VarNames[1] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Base.DiffuseIntensity", i);
            Render->PLVarNames[i].VarNames[2] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].WorldPos", i);
            Render->PLVarNames[i].VarNames[3] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Constant", i);
            Render->PLVarNames[i].VarNames[4] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Linear", i);
            Render->PLVarNames[i].VarNames[5] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gPointLights[%d].Atten.Exp", i);
            Render->PLVarNames[i].VarNames[6] = PushStringZ(WorldArena, TmpName);
        }

        // spot lights
        Render->SpotLightsCount = 1;
        Render->SpotLights = PushArray(WorldArena, Render->SpotLightsCount, spot_light);
        spot_light *SpotLights = Render->SpotLights;
        SpotLights[0].Base.Base.Color = V3(1.0f, 0.0f, 0.0f);
        SpotLights[0].Base.Base.AmbientIntensity = 1.0f;
        SpotLights[0].Base.Base.DiffuseIntensity = 1.0f;
        SpotLights[0].Base.WorldPosition = V3(5.5f, 5.0f, 12.0f);
        SpotLights[0].Base.Atten.Constant = 1.0f;
        SpotLights[0].Base.Atten.Linear = 0.01f;
        SpotLights[0].Base.Atten.Exp = 0.0f;
        SpotLights[0].WorldDirection = V3(0.0f, 0.0f, -1.0f);
        SpotLights[0].WorldDirection = Normalize(SpotLights[0].WorldDirection);
        SpotLights[0].Cutoff = 0.9f;
        // имена переменных spot lights для оправки в шейдер
        Render->SLVarNames = PushArray(WorldArena, Render->SpotLightsCount, spot_light_var_names);
        for(u32 i = 0; i < Render->SpotLightsCount; i++)
        {
            char TmpName[128];
            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.Color", i);
            Render->SLVarNames[i].VarNames[0] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.AmbientIntensity", i);
            Render->SLVarNames[i].VarNames[1] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Base.DiffuseIntensity", i);
            Render->SLVarNames[i].VarNames[2] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.WorldPos", i);
            Render->SLVarNames[i].VarNames[3] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Constant", i);
            Render->SLVarNames[i].VarNames[4] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Linear", i);
            Render->SLVarNames[i].VarNames[5] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Base.Atten.Exp", i);
            Render->SLVarNames[i].VarNames[6] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Direction", i);
            Render->SLVarNames[i].VarNames[7] = PushStringZ(WorldArena, TmpName);

            _snprintf_s(TmpName, sizeof(TmpName), "gSpotLights[%d].Cutoff", i);
            Render->SLVarNames[i].VarNames[8] = PushStringZ(WorldArena, TmpName);
        }

        //
        // NOTE(me): Объекты окружения (2d-текстуры и 3d-модели)
        //
        for(u32 i = 0; i < ENV_OBJECTS_MAX; i++)
        {
            GameState->EnvObjects[i] = PushStruct(WorldArena, entity_envobject);
        }

        // Единичные матрицы объектам окружения
        for(u32 i = 0; i < ENV_OBJECTS_MAX; i++)
        {
            GameState->EnvObjects[i]->TranslateMatrix = Identity();
            GameState->EnvObjects[i]->RotateMatrix = Identity();
            GameState->EnvObjects[i]->ScaleMatrix = Identity();
        }

        u32 EnvIndex = 0;
        entity_envobject **EnvObjects = GameState->EnvObjects;

        // Террейн
        EnvObjects[EnvIndex]->Model = CreateTerrainModel(WorldArena);
        EnvIndex++;

        // ваза
        EnvObjects[EnvIndex]->InstancingCount = 100;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(2, 4, 1),                           // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),  // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),  // Rotate Y rand() Min, Max, Precision
                                              V3(0, 0, 0)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_vase.spm");
        EnvIndex++;

        // бочка
        EnvObjects[EnvIndex]->InstancingCount = 50;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(1.0, 2.0, 1),                       // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),                           // Rotate X, Y, Z
                                              V3(0, 0, 0),    // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),    // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1)); // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_barrel.spm");
        EnvIndex++;

        // дерево
        EnvObjects[EnvIndex]->InstancingCount = 10;
        EnvObjects[EnvIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                            // Memory
                                              EnvObjects[0],                         // Terrain
                                              EnvObjects[EnvIndex]->InstancingCount, // Amount
                                              V3(0.7, 0.7, 1),                       // Scale rand() Min, Max, Precision
                                              V3(90, 0, 0),                          // Rotate X, Y, Z
                                              V3(0, 0, 0),   // Rotate X rand() Min, Max, Precision
                                              V3(0, 360, 1), // Rotate Y rand() Min, Max, Precision
                                              V3(0, 0, 0));  // Rotate Z rand() Min, Max, Precision
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_tree.spm");
        EnvIndex++;

        // ковбой (анимированный)
        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(2, 3, 0));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.4f);
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_cowboy.spm");
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(2, 3 + 3, 0));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.4f);
        EnvObjects[EnvIndex]->Model = EnvObjects[EnvIndex - 1]->Model;
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(2, 3 + 6, 0));
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.4f);
        EnvObjects[EnvIndex]->Model = EnvObjects[EnvIndex - 1]->Model;
        EnvIndex++;

        // страж (анимированный)
        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(5, 10 + 2, 0));
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90.0f);
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.1f);
        EnvObjects[EnvIndex]->Model = LoadModel(WorldArena, "assets/test_guard.spm");
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(5 + 3, 10 + 2, 0));
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90.0f);
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.1f);
        EnvObjects[EnvIndex]->Model = EnvObjects[EnvIndex - 1]->Model;
        EnvIndex++;

        EnvObjects[EnvIndex]->TranslateMatrix = Translation(V3(5 + 6, 10 + 2, 0));
        EnvObjects[EnvIndex]->RotateMatrix = XRotation(90.0f);
        EnvObjects[EnvIndex]->ScaleMatrix = Scaling(0.1f);
        EnvObjects[EnvIndex]->Model = EnvObjects[EnvIndex - 1]->Model;
        EnvIndex++;

        Assert(EnvIndex <= ENV_OBJECTS_MAX);
        Render->EnvObjectsCount = EnvIndex;

        // TODO(me): нужно ли это? убрать? высота объектов окружения на ландшафте
        for(u32 i = 1; i < Render->EnvObjectsCount; i++)
        {
            EnvObjects[i]->TranslateMatrix.E[2][3] +=
                EnvObjects[i]->TranslateMatrix.E[2][3] + TerrainGetHeight(EnvObjects[0],
                                                                          EnvObjects[i]->TranslateMatrix.E[0][3],
                                                                          EnvObjects[i]->TranslateMatrix.E[1][3]);
        }

        // Добавление объектов окружения в рендерер и создание VBO
        // AddEnvObjectsToRender(Render, EnvObjects);
        // InitEnvVBOs(WorldArena, Render);

        //
        // NOTE(me): Объекты травы
        //
        for(u32 i = 0; i < GRASS_OBJECTS_MAX; i++)
        {
            GameState->GrassObjects[i] = PushStruct(WorldArena, entity_grassobject);
        }

        u32 GrassIndex = 0;
        entity_grassobject **GrassObjects = GameState->GrassObjects;

        // трава 1
        GrassObjects[GrassIndex]->InstancingCount = 150;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass1.spm");
        GrassIndex++;

        // трава 2
        GrassObjects[GrassIndex]->InstancingCount = 150;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass2.spm");
        GrassIndex++;

        // трава 3
        GrassObjects[GrassIndex]->InstancingCount = 20000;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass3.spm");
        GrassIndex++;

        // трава 4
        GrassObjects[GrassIndex]->InstancingCount = 500;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass4.spm");
        GrassIndex++;

        // трава 5
        GrassObjects[GrassIndex]->InstancingCount = 50;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass5.spm");
        GrassIndex++;

        // трава 6
        GrassObjects[GrassIndex]->InstancingCount = 20;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass6.spm");
        GrassIndex++;

        // трава 7
        GrassObjects[GrassIndex]->InstancingCount = 50;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass7.spm");
        GrassIndex++;

        // трава 8
        GrassObjects[GrassIndex]->InstancingCount = 50;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass8.spm");
        GrassIndex++;

        // трава 9
        GrassObjects[GrassIndex]->InstancingCount = 20;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass9.spm");
        GrassIndex++;

        // трава 10
        GrassObjects[GrassIndex]->InstancingCount = 20;
        GrassObjects[GrassIndex]->InstancingTransformMatrices =
            CreateInstancingTransformMatrices(WorldArena,                                // Memory
                                              EnvObjects[0],                             // Terrain
                                              GrassObjects[GrassIndex]->InstancingCount, // Amount
                                              V3(0.05, 0.1, 1), // Scale rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate X, Y, Z
                                              V3(0, 0, 0),      // Rotate X rand() Min, Max, Precision
                                              V3(0, 0, 0),      // Rotate Y rand() Min, Max, Precision
                                              V3(0, 360, 1));   // Rotate Z rand() Min, Max, Precision
        GrassObjects[GrassIndex]->Model = LoadModel(WorldArena, "assets/test_grass10.spm");
        GrassIndex++;

        Assert(GrassIndex <= GRASS_OBJECTS_MAX);
        Render->GrassObjectsCount = GrassIndex;

        // Добавление объектов травы в рендерер и создание VBO
        // AddGrassObjectsToRender(Render, GrassObjects);
        // InitGrassVBO(WorldArena, Render);

        //
        // NOTE(me): Шейдеры и VBO
        //

        u32 Shader1Vert = LoadShader("../code/shaders/Default.vert", GL_VERTEX_SHADER);
        u32 Shader1Frag = LoadShader("../code/shaders/Default.frag", GL_FRAGMENT_SHADER);
        Render->DefaultShaderProgram = LinkShaderProgram(Shader1Vert, Shader1Frag);

        u32 Shader2Vert = LoadShader("../code/shaders/Water.vert", GL_VERTEX_SHADER);
        u32 Shader2Frag = LoadShader("../code/shaders/Water.frag", GL_FRAGMENT_SHADER);
        Render->WaterShaderProgram = LinkShaderProgram(Shader2Vert, Shader2Frag);

        u32 Shader3Vert = LoadShader("../code/shaders/DepthMap.vert", GL_VERTEX_SHADER);
        u32 Shader3Frag = LoadShader("../code/shaders/DepthMap.frag", GL_FRAGMENT_SHADER);
        Render->DepthMapShaderProgram = LinkShaderProgram(Shader3Vert, Shader3Frag);

        u32 Shader4Vert = LoadShader("../code/shaders/Grass.vert", GL_VERTEX_SHADER);
        u32 Shader4Frag = LoadShader("../code/shaders/Grass.frag", GL_FRAGMENT_SHADER);
        Render->GrassShaderProgram = LinkShaderProgram(Shader4Vert, Shader4Frag);

        //
        // NOTE(me): Water
        //
        Render->WaterMoveFactor = 0; // only for calc
        Render->ReflWidth = 1920 / 4;
        Render->ReflHeight = 1080 / 4;
        Render->RefrWidth = 1920 / 2;
        Render->RefrHeight = 1080 / 2;
        Render->WaterWaveSpeed = 0.4f;
        Render->WaterTiling = 0.5f;
        Render->WaterWaveStrength = 0.02f;
        Render->WaterShineDamper = 20.0f;
        Render->WaterReflectivity = 0.6f;
        Render->WaterDUDVTextureName = PushString(WorldArena, "NewWaterDUDV.png");
        Render->WaterDUDVTexture = LoadTexture(&Render->WaterDUDVTextureName);
        Render->WaterNormalMapName = PushString(WorldArena, "NewWaterNormalMap.png");
        Render->WaterNormalMap = LoadTexture(&Render->WaterNormalMapName);
        InitWaterFBOs(Render);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }

        //
        // NOTE(me): Shadows
        //
        Render->DepthMapWidth = 1920 * 4;
        Render->DepthMapHeight = 1080 * 4;
        Render->ShadowMapSize = 68.0f;
        Render->NearPlane = 50.0f;
        Render->FarPlane = 144.0f;
        Render->ShadowCameraPitch = 51.5f;
        Render->ShadowCameraYaw = 317.0f;
        // That SunPos vector from origin to sun pos (camera view)
        // Need be inverted if want from sun to surface direction
        Render->SunPos = V3(-3.0f, -6.0f, 73.5f);
        Render->Bias = 0.005f;
        // Render->ShadowMapSize = 10.0f;
        // Render->NearPlane = 0.0f;
        // Render->FarPlane = 100.0f;
        // Render->ShadowCameraPitch = 65.0f;
        // Render->ShadowCameraYaw = 315.0f;
        // Render->SunPos = V3(-10.0f, -10.0f, 10.0f);
        InitDepthMapFBO(Render);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            InvalidCodePath;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        //
        // NOTE(me): Textures for Debug Elements
        //
        Render->ClipTextureName = PushString(WorldArena, "clip.png");
        Render->ClipTexture = LoadTexture(&Render->ClipTextureName);
        Render->LightgTextureName = PushString(WorldArena, "lamp.png");
        Render->LightTexture = LoadTexture(&Render->LightgTextureName);*/

        // ImGui Demo Window
        GameState->ShowDemoWindow = false;
        GameState->ShowAnotherWindow = false;

        // TODO(me): удалить?
        Log.AddLog("[test] Hello %d world\n", 123);
        Log.AddLog("[test] 567657657\n");
        Log.AddLog("[test] 1\n");
        Log.AddLog("[test] 2\n");

        // String example
        // string TestStr = PushString(WorldArena, "fdsfsdfss");

        // Assert example
        // int32 xxx = 1;
        // Assert(xxx < 0);

        GameState->IsInitialized = true;
    }

    //
    // NOTE(casey): Transient initialization
    //
    Assert(sizeof(transient_state) <= Memory->TransientStorageSize);
    transient_state *TranState = (transient_state *)Memory->TransientStorage;
    if(!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena, Memory->TransientStorageSize - sizeof(transient_state),
                        (uint8 *)Memory->TransientStorage + sizeof(transient_state));
        memory_arena *TranArena = &TranState->TranArena;

        TranState->TestQueue = Platform.HighPriorityQueue;

        TranState->GroundBufferCount = 64;
        TranState->GroundBuffers = PushArray(TranArena, TranState->GroundBufferCount, ground_buffer);
        for(uint32 GroundBufferIndex = 0;                     //
            GroundBufferIndex < TranState->GroundBufferCount; //
            ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
            // GroundBuffer->Bitmap = MakeEmptyBitmap(TranArena, GroundBufferWidth, GroundBufferHeight, false);
            // GroundBuffer->Texture = U32Max;
            s32 GroundTextureSizeMultiplyer = 32;
            GroundBuffer->DrawBuffer.Width = (s32)GroundBufferWidth * GroundTextureSizeMultiplyer;
            GroundBuffer->DrawBuffer.Height = (s32)GroundBufferHeight * GroundTextureSizeMultiplyer;
            glGenTextures(1, &GroundBuffer->DrawBuffer.ID);
            glGenTextures(1, &GroundBuffer->DrawBuffer.DepthTexture);
            GroundBuffer->P = NullPosition();
            GroundBuffer->DrawBuffer.FBO = Buffer->GroundFBO;

            GroundBuffer->TerrainModel = CreateTerrainChunkModel(TranArena, GroundBufferWidth, GroundBufferHeight);
        }

        // TODO(me): Testing only
        Platform.AddEntry(TranState->TestQueue, DoWorkerWork2, "Testing work hm...");

        TranState->IsInitialized = true;
    }

    // Local pointers for game_state sctructs
    game_debug *Debug = GameState->Debug;
    app_settings *Settings = GameState->Settings;
    world *World = GameState->World;
    render *Render = GameState->Render;

    // real32 MetersToPixels = GameState->MetersToPixels;
    // real32 PixelsToMeters = GameState->PixelsToMeters;

    // entity_player *Player = GameState->Player;
    // entity_clip *PlayerClip = GameState->Clip;
    // entity_envobject **EnvObjects = GameState->EnvObjects; // TODO(me): избавиться от 16
    // entity_grassobject **GrassObjects = GameState->GrassObjects;
    //  world_position *CameraP = &GameState->CameraP;

    //
    // NOTE(me): Inputs
    //
    if(!Input->ShowMouseCursorMode)
    {
        if(Input->MouseOffsetX != 0)
        {
            int tmp = 0;
        }
        // по горизонтали
        r32 ZAngle = Input->MouseOffsetX;
        GameState->CameraYaw -= ZAngle * Settings->MouseSensitivity;
        if(GameState->CameraYaw < 0)
        {
            GameState->CameraYaw += 360;
        }
        if(GameState->CameraYaw > 360)
        {
            GameState->CameraYaw -= 360;
        }

        // по вертикали
        r32 XAngle = Input->MouseOffsetY;
        GameState->CameraPitch += XAngle * Settings->MouseSensitivity;
        if(GameState->CameraPitch < 0)
        {
            GameState->CameraPitch = 0;
        }
        if(GameState->CameraPitch > 180)
        {
            GameState->CameraPitch = 180;
        }

        // по вертикали (inversed)
        /*GameState->CameraPitchInversed -= XAngle * Settings->MouseSensitivity;
        if(GameState->CameraPitchInversed < 0)
        {
            GameState->CameraPitchInversed = 0;
        }
        if(GameState->CameraPitchInversed > 180)
        {
            GameState->CameraPitchInversed = 180;
        }*/
    }

    for(int ControllerIndex = 0;                          //
        ControllerIndex < ArrayCount(Input->Controllers); //
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_hero *ConHero = GameState->ControlledHeroes + ControllerIndex;
        if(ConHero->EntityIndex == 0)
        {
            if(Controller->Start.EndedDown)
            {
                *ConHero = {};
                ConHero->EntityIndex = AddPlayer(GameState).LowIndex;
            }
        }
        else
        {
            ConHero->dZ = 0.0f;
            ConHero->ddP = {};
            ConHero->dSword = {};

            if(Controller->IsAnalog)
            {
                // NOTE(casey): Use analog movement tuning
                // ConHero->ddP = V2(Controller->StickAverageX, Controller->StickAverageY);
            }
            else
            {
                // NOTE(casey): Use digital movement tuning
                if(Controller->MoveUp.EndedDown)
                {
                    ConHero->ddP.y = 1.0f;
                }
                if(Controller->MoveDown.EndedDown)
                {
                    ConHero->ddP.y = -1.0f;
                }
                if(Controller->MoveLeft.EndedDown)
                {
                    ConHero->ddP.x = -1.0f;
                }
                if(Controller->MoveRight.EndedDown)
                {
                    ConHero->ddP.x = 1.0f;
                }
            }

            if(Controller->Start.EndedDown)
            {
                ConHero->dZ = 3.0f;
            }

            if(Controller->ActionUp.EndedDown)
            {
                ConHero->dSword = V2(0.0f, 1.0f);
            }
            if(Controller->ActionDown.EndedDown)
            {
                ConHero->dSword = V2(0.0f, -1.0f);
            }
            if(Controller->ActionLeft.EndedDown)
            {
                ConHero->dSword = V2(-1.0f, 0.0f);
            }
            if(Controller->ActionRight.EndedDown)
            {
                ConHero->dSword = V2(1.0f, 0.0f);
            }
        }
    }

    //
    // NOTE(me): World Sim
    //
    Render->DisplayWidth = Buffer->Width;
    Render->DisplayHeight = Buffer->Height;
    Render->AspectRatio = (r32)Render->DisplayWidth / (r32)Render->DisplayHeight;
    // real32 ScreenWidthInMeters = Render->DisplayWidth * PixelsToMeters;
    // real32 ScreenHeightInMeters = Render->DisplayHeight * PixelsToMeters;
    // rectangle3 CameraBoundsInMeters = RectCenterDim(V3(0, 0, 0), //
    //                                                 V3(ScreenWidthInMeters, ScreenHeightInMeters, 0.0f));
    // rectangle3 CameraBounds =
    //     RectCenterDim(V3(0, 0, 0), V3((r32)Render->DisplayWidth, (r32)Render->DisplayHeight, 0.0f));
    u32 SimChunksInCamera = 5;
    real32 CameraWidthInMeters = SimChunksInCamera * World->ChunkDimInMeters.x;
    real32 CameraHeightInMeters = SimChunksInCamera * World->ChunkDimInMeters.y;
    rectangle3 CameraBoundsInMeters = RectCenterDim(V3(0, 0, 0), V3(CameraWidthInMeters, CameraHeightInMeters, 0.0f));

    // v3 SimBoundsExpansion = V3(15.0f, 15.0f, 15.0f);
    v3 SimBoundsExpansion = V3(15.0f, 15.0f, 15.0f);
    rectangle3 SimBounds = AddRadiusTo(CameraBoundsInMeters, SimBoundsExpansion);

    temporary_memory SimMemory = BeginTemporaryMemory(&TranState->TranArena);
    sim_region *SimRegion = BeginSim(&TranState->TranArena, GameState, GameState->World, //
                                     GameState->CameraP, SimBounds, Input->dtForFrame);

    // r32 ScreenCenterX = 0.5f * Render->DisplayWidth;
    // r32 ScreenCenterY = 0.5f * Render->DisplayHeight;

    // Высота камеры игрока на террейне (ландшафте)
    /*Player->Position.z =
        TerrainGetHeight(EnvObjects[0], Player->Position.x, Player->Position.y) + Player->CameraZOffset;*/
    // Player->Position.z = Player->CameraZOffset;
    //  Перемещение маркеров источников света
    // EnvObjects[1]->TranslateMatrix = Translation(Render->PointLights[0].WorldPosition);
    // EnvObjects[2]->TranslateMatrix = Translation(Render->SpotLights[0].Base.WorldPosition);
    // Перемещение клип текстуры игрока
    // v3 ClipTextureNewPos = Player->Position - V3(Player->Width * 0.05f, Player->Height * 0.05f, 0);
    // EnvObjects[13]->TranslateMatrix = Translation(ClipTextureNewPos);

    // Формируем матрицы преобразований у объектов окружения
    /*for(u32 i = 0; i < Render->EnvObjectsCount; i++)
    {
        if(EnvObjects[i]->InstancingCount == 0)
        {
            EnvObjects[i]->TransformMatrix =
                EnvObjects[i]->TranslateMatrix * EnvObjects[i]->RotateMatrix * EnvObjects[i]->ScaleMatrix;
            EnvObjects[i]->TransformMatrix = Transpose(EnvObjects[i]->TransformMatrix); // opengl to glsl format
        }
    }

    // Обработка анимаций
    if(Render->Animator.Timer > 0.0f)
    {
        if(Debug->ProcessAnimations)
        {
            for(u32 i = 0; i < Render->SAnMeshesCount; i++)
            {
                single_mesh *Mesh = Render->SAnMeshes[i];
                GetBoneTransforms(Mesh, //
                                  0,    // индекс анимации
                                  Render->Animator.Timer);
            }
        }

        Render->Animator.Timer -= Input->dtForFrame;
        if(Render->Animator.Timer <= 0.0f)
        {
            // Debug->Animator.Timer = 0.0f;
            Render->Animator.Timer = 5.0f; // начинаем проигрывать анимацию заново
        }
    }*/

    //
    // NOTE(me): Render
    //
    loaded_texture DrawBuffer_ = {};
    loaded_texture *DrawBuffer = &DrawBuffer_;
    DrawBuffer->Width = Buffer->Width;
    DrawBuffer->Height = Buffer->Height;
    DrawBuffer->ID = Buffer->DrawTexture;
    DrawBuffer->FBO = Buffer->FBO;

    glBindFramebuffer(GL_FRAMEBUFFER, DrawBuffer->FBO);

    // Create Render Texture Attachment
    glBindTexture(GL_TEXTURE_2D, DrawBuffer->ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Buffer->Width, Buffer->Height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, DrawBuffer->ID, 0);

    //  Create Depth Texture Attachment
    glBindTexture(GL_TEXTURE_2D, Buffer->DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, Buffer->Width, Buffer->Height, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, Buffer->DepthTexture, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    temporary_memory RenderMemory = BeginTemporaryMemory(&TranState->TranArena);
    // TODO(casey): Decide what our pushbuffer size is!

    render_group *RenderGroup =
        AllocateRenderGroup(&TranState->TranArena, Megabytes(4), //
                            GameState->CameraPitch, GameState->CameraYaw, GameState->CameraRenderZ);

    // Clear(RenderGroup, V4(0.25f, 0.25f, 0.25f, 0.0f));
    Clear(RenderGroup, V4(0.45f, 0.55f, 0.60f, 1.0f));
    // Clear(RenderGroup, V4(0.45f, 0.55f, 0.60f, 0.0f));
    //  glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    //  glEnable(GL_DEPTH_TEST);
    //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //   glTranslatef(-CameraP->ChunkX * World->ChunkSideInMeters, -CameraP->ChunkY * World->ChunkSideInMeters, -10.0f);

    // draw oxyz
    /*glPushMatrix();
    glScalef(5, 5, 5);
    OGLDrawLinesOXYZ(V3(0, 0, 1), 1);
    glPopMatrix();

    glLineWidth(1);
    glEnable(GL_LINE_SMOOTH);*/
    if(Debug->DrawSimRegionBounds)
    {
        PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(SimBounds).xy, V4(1, 1, 0, 1));
    }
    if(Debug->DrawSimRegionUpdatableBounds)
    {
        // PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(SimRegion->Bounds).xy, V4(0, 1, 0, 1));
        PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(SimRegion->UpdatableBounds).xy, V4(0, 1, 0, 1));
    }
    // glDisable(GL_LINE_SMOOTH);

    // TODO(casey): Move this out into handmade_entity.cpp!
    // entity_visible_piece_group PieceGroup;
    // PieceGroup.GameState = GameState;
    for(u32 EntityIndex = 0;                  //
        EntityIndex < SimRegion->EntityCount; //
        ++EntityIndex)
    {
        sim_entity *Entity = SimRegion->Entities + EntityIndex;
        if(Entity->Updatable)
        {
            r32 dt = Input->dtForFrame;

            // TODO(casey): This is incorrect, should be computed after update!!!!
            move_spec MoveSpec = DefaultMoveSpec();
            v3 ddP = {};

            //
            // NOTE(me): Pre-physics entity work
            //
            switch(Entity->Type)
            {
            case EntityType_Hero: //
            {
                // TODO(casey): Now that we have some real usage examples, let's solidify
                // the positioning system!
                for(u32 ControlIndex = 0;                                   //
                    ControlIndex < ArrayCount(GameState->ControlledHeroes); //
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
                        MoveSpec.Drag = 8.0f;
                        // MoveSpec.Speed = 25.0f;
                        // MoveSpec.Drag = 1.0f;
#if 1
                        r32 Angle = GameState->CameraYaw * Pi32 / 180;
                        ddP.x = ConHero->ddP.x * Cos(Angle) - ConHero->ddP.y * Sin(Angle);
                        ddP.y = ConHero->ddP.x * Sin(Angle) + ConHero->ddP.y * Cos(Angle);
                        ddP.z = 0;
#else
                        ddP = V3(ConHero->ddP, 0);
#endif

                        if((ConHero->dSword.x != 0.0f) || (ConHero->dSword.y != 0.0f))
                        {
                            sim_entity *Sword = Entity->Sword.Ptr;
                            if(Sword && IsSet(Sword, EntityFlag_Nonspatial))
                            {
                                Sword->DistanceLimit = 5.0f;
                                MakeEntitySpatial(Sword, Entity->P, Entity->dP + 5.0f * V3(ConHero->dSword, 0));
                                AddCollisionRule(GameState, Sword->StorageIndex, Entity->StorageIndex, false);
                            }
                        }
                    }
                }
            }
            break;

            case EntityType_Sword: //
            {
                MoveSpec.UnitMaxAccelVector = false;
                MoveSpec.Speed = 0.0f;
                MoveSpec.Drag = 0.0f;

                if(Entity->DistanceLimit == 0.0f)
                {
                    ClearCollisionRulesFor(GameState, Entity->StorageIndex);
                    MakeEntityNonSpatial(Entity);
                }
            }
            break;

            case EntityType_Familiar: //
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
            }
            break;
            }

            if(!IsSet(Entity, EntityFlag_Nonspatial) && //
               IsSet(Entity, EntityFlag_Moveable))
            {
                MoveEntity(GameState, SimRegion, Entity, Input->dtForFrame, &MoveSpec, ddP);
            }

            // Basis->P = GetEntityGroundPoint(Entity);
            RenderGroup->Transform.OffsetP = GetEntityGroundPoint(Entity);

            //
            // NOTE(me): Post-physics entity work
            //
            switch(Entity->Type)
            {
            case EntityType_Hero: //
            {
                if(Debug->DrawPlayerHitbox)
                {
                    PushRectCollisionVolumes(RenderGroup, Entity, V4(0, 1, 0, 1));
                }
            }
            break;

            case EntityType_Wall: //
            {
                PushRectCollisionVolumes(RenderGroup, Entity, V4(1, 0, 0, 1));
            }
            break;

            case EntityType_Stairwell: //
            {
                // PushRect(&PieceGroup, V2(0, 0), 0, Entity->WalkableDim, V4(1, 0.5f, 0, 1), 0.0f);
                // PushRect(&PieceGroup, V2(0, 0), Entity->WalkableHeight, Entity->WalkableDim, V4(1, 1, 0, 1),
                // 0.0f);
            }
            break;

            case EntityType_Sword: //
            {
                PushRectCollisionVolumes(RenderGroup, Entity, V4(1, 1, 0, 1));
            }
            break;

            case EntityType_Familiar: //
            {
                /*
                real32 BobSin = Sin(2.0f * Entity->tBob);
                PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align,
                           (0.5f * ShadowAlpha) + 0.2f * BobSin, 0.0f);
                PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0, 0), 0.25f * BobSin, HeroBitmaps->Align);
                */
            }
            break;

            case EntityType_Monstar: //
            {
                PushRectCollisionVolumes(RenderGroup, Entity, V4(0.5f, 0, 0.5f, 1));
            }
            break;

            case EntityType_Space: //
            {
                PushRectOutlineCollisionVolumes(RenderGroup, Entity, V4(0, 0, 1, 1), 2);
                // DrawCollisionRectOutline(Entity, 0.0f, V3(0, 0, 1));
            }
            break;

                InvalidDefaultCase;
            }
        }
    }

    /*world_position WorldOrigin = {};
    v3 Diff = Subtract(SimRegion->World, &WorldOrigin, &SimRegion->Origin);
    // DrawRectangle(Buffer, Diff.XY, V2(10.0f, 10.0f), 1.0f, 1.0f, 0.0f);
    DrawRectangle({Diff.xy, V2(10.0f, 10.0f)}, 0.0f, V3(1, 0, 0));*/

    /*
    #if 1
        // glViewport(0, 0, Render->DisplayWidth, Render->DisplayHeight);
        // glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        // glEnable(GL_DEPTH_TEST);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // RenderDebugElements(World, Render, Player, PlayerClip);
    #else
        glEnable(GL_CLIP_DISTANCE0);

        // Render Reflection Texture
        glViewport(0, 0, Render->ReflWidth, Render->ReflHeight);
        r32 WaterZ = 0.0f;
        r32 TmpPlayerZ = 2.7f; // TODO(me): replace with real player Z
        r32 Distance = 2 * (TmpPlayerZ - WaterZ);
        TmpPlayerZ -= Distance;
        r32 NormalCameraPitch = Player->CameraPitch;
        Player->CameraPitch = Player->CameraPitchInversed;
        Render->CutPlane = V4(0, 0, 1, -WaterZ);
        glBindFramebuffer(GL_FRAMEBUFFER, Render->WaterReflFBO);
        RenderScene(World, Render, Render->DefaultShaderProgram, Player, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // RenderGrassVBO(Render, Player);
        TmpPlayerZ += Distance;
        Player->CameraPitch = NormalCameraPitch;

        // Render Refraction Texture
        glViewport(0, 0, (u32)(Render->RefrWidth), (u32)(Render->RefrHeight));
        Render->CutPlane = V4(0, 0, -1, WaterZ);
        glBindFramebuffer(GL_FRAMEBUFFER, Render->WaterRefrFBO);
        RenderScene(World, Render, Render->DefaultShaderProgram, Player, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // RenderGrassVBO(Render, Player);

        glDisable(GL_CLIP_DISTANCE0);
        Render->CutPlane = V4(0, 0, -1, 100000);

        // Render Depth Map for Shadows
        glViewport(0, 0, (u32)(Render->DepthMapWidth), (u32)(Render->DepthMapHeight));
        glBindFramebuffer(GL_FRAMEBUFFER, Render->DepthMapFBO);
        glCullFace(GL_FRONT);
        RenderScene(World, Render, Render->DepthMapShaderProgram, Player, GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render Game
        glViewport(0, 0, Render->DisplayWidth, Render->DisplayHeight);
        RenderScene(World, Render, Render->DefaultShaderProgram, Player, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        RenderGrassVBO(World, Render, Player);
        RenderWater(World, Render, Player, Input->dtForFrame, WaterZ);
        RenderDebugElements(World, Render, Player, PlayerClip);
    // DrawOrthoTexturedRectangle(Render, Render->WaterReflTexture, 320, 180, V2(340, 200));
    // DrawOrthoTexturedRectangle(Render, Render->WaterRefrTexture, 320, 180, V2(1000, 200));
    // DrawOrthoTexturedRectangle(Render, Render->DepthMap, 320, 180, V2(1550, 200));
    // DrawOrthoTexturedRectangle(Render, Render->ClipTexture, 320, 180, V2(1550, 200));

#endif
    */

    // NOTE(casey): Ground chunk rendering
    for(uint32 GroundBufferIndex = 0;                     //
        GroundBufferIndex < TranState->GroundBufferCount; //
        ++GroundBufferIndex)
    {
        ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
        if(IsValid(GroundBuffer->P))
        {
            // loaded_bitmap *Bitmap = &GroundBuffer->Bitmap;
            v3 Delta = Subtract(GameState->World, &GroundBuffer->P, &GameState->CameraP);

            if((Delta.z >= -1.0f) && (Delta.z < 1.0f))
            {
                // real32 GroundSideInMeters = World->ChunkDimInMeters.x;
                //  PushBitmap(RenderGroup, Bitmap, GroundSideInMeters, V3(0, 0, 0));
                // PushRectOutline(RenderGroup, V3(0, 0, 0), V2(GroundSideInMeters, GroundSideInMeters),
                //                 V4(1.0f, 1.0f, 0.0f, 1.0f));
                // PushRectOutline(RenderGroup, V3(0, 0, 0), World->ChunkDimInMeters.xy, V4(0, 1, 0, 1));
                // PushTexture(RenderGroup, V3(0, 0, 0), World->ChunkDimInMeters.xy, GameState->TestTexture1, true);
                // PushTexture(RenderGroup, V3(0, 0, 0), World->ChunkDimInMeters.xy, GroundBuffer->DrawBuffer.Texture);
                PushModel(RenderGroup, Delta, World->ChunkDimInMeters.xy, GroundBuffer->TerrainModel);
            }
        }
    }

    // NOTE(casey): Ground chunk updating
    if(Debug->DrawSimChunks || Debug->DrawChunkWhereCamera)
    {
        // v2 ScreenCenter = {0.5f * (real32)Render->DisplayWidth, //
        //                    0.5f * (real32)Render->DisplayHeight};

        world_position MinChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMinCorner(CameraBoundsInMeters));
        world_position MaxChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMaxCorner(CameraBoundsInMeters));

        for(s32 ChunkZ = MinChunkP.ChunkZ; //
            ChunkZ <= MaxChunkP.ChunkZ;    //
            ++ChunkZ)
        {
            for(s32 ChunkY = MinChunkP.ChunkY; //
                ChunkY <= MaxChunkP.ChunkY;    //
                ++ChunkY)
            {
                for(s32 ChunkX = MinChunkP.ChunkX; //
                    ChunkX <= MaxChunkP.ChunkX;    //
                    ++ChunkX)
                {
#if 0
                    // temporary_memory GroundMemory1 = BeginTemporaryMemory(&TranState->TranArena);
                    // render_group *RenderGroup1 =
                    //     AllocateRenderGroup(&TranState->TranArena, Megabytes(4), //
                    //                         GameState->CameraPitch, GameState->CameraYaw, GameState->CameraRenderZ);

                    // Clear(RenderGroup1, V4(1.0f, 1.0f, 0.0f, 1.0f));

                    world_position ChunkCenterP = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
                    v3 RelP = Subtract(World, &ChunkCenterP, &GameState->CameraP);

                    if(Debug->DrawSimChunks)
                    {
                        PushRectOutline(RenderGroup, V3(RelP.xy, 0), World->ChunkDimInMeters.xy, V4(1, 0, 0, 1));
                        // PushModel(RenderGroup, V3(0, 0, 0), World->ChunkDimInMeters.xy, GroundBuffer->TerrainModel,
                        // false);
                    }
                    /*if(Debug->DrawChunkWhereCamera &&           //
                       (GameState->CameraP.ChunkX == ChunkX) && //
                       (GameState->CameraP.ChunkY == ChunkY))
                    {
                        PushRect(RenderGroup, RelP, World->ChunkDimInMeters.xy,
                                 V4(140.0f / 255.0f, 140.0f / 255.0f, 218.0f / 255.0f, 1));
                    }*/

                    // RenderGroupToOutput(RenderGroup1);
                    // EndTemporaryMemory(GroundMemory1);
#else
                    world_position ChunkCenterP = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
                    v3 RelP = Subtract(World, &ChunkCenterP, &GameState->CameraP);

                    u32 TmpGroundBufferIndex = 0;
                    // TODO(casey): This is super inefficient fix it!
                    real32 FurthestBufferLengthSq = 0.0f;
                    ground_buffer *FurthestBuffer = 0;
                    for(uint32 GroundBufferIndex = 0;                     //
                        GroundBufferIndex < TranState->GroundBufferCount; //
                        ++GroundBufferIndex)
                    {
                        ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
                        if(AreInSameChunk(World, &GroundBuffer->P, &ChunkCenterP))
                        {
                            TmpGroundBufferIndex = 0;
                            FurthestBuffer = 0;
                            break;
                        }
                        else if(IsValid(GroundBuffer->P))
                        {
                            v3 RelP = Subtract(World, &GroundBuffer->P, &GameState->CameraP);
                            real32 BufferLengthSq = LengthSq(RelP.xy);
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
                        Log.AddLog("[fillgroundchunk] TmpGroundBufferIndex=%d (%d,%d)\n", //
                                   TmpGroundBufferIndex, ChunkCenterP.ChunkX, ChunkCenterP.ChunkY);
#if 0
                        FillGroundChunk(TranState, GameState, //
                                        FurthestBuffer,       //
                                        &ChunkCenterP);
#endif
                    }
#endif
                }
            }
        }
    }

    RenderGroupToOutput(RenderGroup, DrawBuffer);

    //
    // NOTE(me): Draw Buffer Texture
    //
    // TODO(me): Texture rendering through shader?
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, DrawBuffer->Width, DrawBuffer->Height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, DrawBuffer->Width, 0, DrawBuffer->Height, 0, 1);
    glMatrixMode(GL_MODELVIEW);

    r32 MinX = 0.0f;
    r32 MaxX = (r32)DrawBuffer->Width;
    r32 MinY = 0.0f;
    r32 MaxY = (r32)DrawBuffer->Height;

    r32 VRect[] = {
        MinX, MinY, // 0
        MaxX, MinY, // 1
        MaxX, MaxY, // 2
        MinX, MaxY  // 3
    };

    r32 Repeat = 1.0f;
    r32 UVRect[] = {
        0,      0,      // 0
        Repeat, 0,      // 1
        Repeat, Repeat, // 2
        0,      Repeat  // 3
    };

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, DrawBuffer->ID);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, 0, VRect);
    glTexCoordPointer(2, GL_FLOAT, 0, UVRect);
    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    glDisable(GL_TEXTURE_2D);

    RenderImGui(Input, GameState, TranState, Buffer, SimRegion);

    EndSim(SimRegion, GameState);
    EndTemporaryMemory(RenderMemory);
    EndTemporaryMemory(SimMemory);

    CheckArena(&GameState->WorldArena);
    CheckArena(&TranState->TranArena);

    END_TIMED_BLOCK(EngineUpdateAndRender);
}