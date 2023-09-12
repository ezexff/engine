#include "engine.h"
#include "engine_asset.cpp"
#include "engine_world.cpp"
#include "engine_sim_region.cpp"
#include "engine_entity.cpp"
#include "engine_render.cpp"

struct add_low_entity_result
{
    low_entity *Low;
    u32 LowIndex;
};

internal add_low_entity_result AddLowEntity(game_state *GameState, entity_type Type, world_position P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities));
    u32 EntityIndex = GameState->LowEntityCount++;

    low_entity *EntityLow = GameState->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Sim.Type = Type;
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

internal add_low_entity_result AddWall(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    world_position P = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall, P);

    Entity.Low->Sim.Height = GameState->World->TileSideInMeters;
    Entity.Low->Sim.Width = Entity.Low->Sim.Height;
    AddFlag(&Entity.Low->Sim, EntityFlag_Collides);

    return (Entity);
}

internal void InitHitPoints(low_entity *EntityLow, u32 HitPointCount)
{
    Assert(HitPointCount <= ArrayCount(EntityLow->Sim.HitPoint));
    EntityLow->Sim.HitPointMax = HitPointCount;
    for(u32 HitPointIndex = 0; HitPointIndex < EntityLow->Sim.HitPointMax; ++HitPointIndex)
    {
        hit_point *HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
        HitPoint->Flags = 0;
        HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
    }
}

internal add_low_entity_result AddSword(game_state *GameState)
{
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());

    Entity.Low->Sim.Height = 0.5f;
    Entity.Low->Sim.Width = 1.0f;

    return (Entity);
}

internal add_low_entity_result AddPlayer(game_state *GameState)
{
    world_position P = GameState->CameraP;
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, P);

    Entity.Low->Sim.Height = 0.5f; // 1.4f;
    Entity.Low->Sim.Width = 1.0f;
    AddFlag(&Entity.Low->Sim, EntityFlag_Collides);

    InitHitPoints(Entity.Low, 3);

    add_low_entity_result Sword = AddSword(GameState);
    Entity.Low->Sim.Sword.Index = Sword.LowIndex;

    if(GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = Entity.LowIndex;
    }

    return (Entity);
}

internal void EngineUpdateAndRender(GLFWwindow *Window, game_memory *Memory, game_input *Input)
{
    Platform = Memory->PlatformAPI;

    //
    // NOTE(me): Memory init
    //
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    game_state *GameState = (game_state *)Memory->PermanentStorage;

    if(!GameState->IsInitialized)
    {
        // установка указателя выделенной памяти после структуры game_state
        InitializeArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *)Memory->PermanentStorage + sizeof(game_state));
        memory_arena *WorldArena = &GameState->WorldArena;
        //
        // NOTE(me): Push game_state structures in memory
        //
        GameState->World = PushStruct(&GameState->WorldArena, world);
        world *World = GameState->World;
        InitializeWorld(World, 1.4f);

        /*while(GameState->LowEntityCount < (ArrayCount(GameState->LowEntities) - 16))
        {
            uint32 Coordinate = 1024 + GameState->LowEntityCount;
            AddWall(GameState, Coordinate, Coordinate, Coordinate);
        }*/

        /*while(GameState->LowEntityCount < (ArrayCount(GameState->LowEntities) - 16))
        {
            // uint32 Coordinate = 1024 + GameState->LowEntityCount;
            AddWall(GameState, 1024 + GameState->LowEntityCount, 5, 0);
        }*/
        AddWall(GameState, 1024 + GameState->LowEntityCount, 5, 0);
        for(u32 i = 0; i < 20; i++)
        {
            if(i % 2)
            {
                AddWall(GameState, i, 5, 0);
            }
        }

        u32 TilesPerWidth = 17;
        u32 TilesPerHeight = 9;
        u32 ScreenBaseX = 0;
        u32 ScreenBaseY = 0;
        u32 ScreenBaseZ = 0;
        world_position NewCameraP = {};
        u32 CameraTileX = ScreenBaseX * TilesPerWidth + 17 / 2;
        u32 CameraTileY = ScreenBaseY * TilesPerHeight + 9 / 2;
        u32 CameraTileZ = ScreenBaseZ;
        NewCameraP = ChunkPositionFromTilePosition(GameState->World, CameraTileX, CameraTileY, CameraTileZ);
        GameState->CameraP = NewCameraP;

        GameState->Debug = PushStruct(WorldArena, debug);
        debug *Debug = GameState->Debug;
        Debug->ProcessAnimations = true;
        Debug->DrawSimRegionBounds = false;
        Debug->DrawSimRegionUpdatableBounds = false;
        Debug->DrawSimChunks = false;
        Debug->DrawChunkWhereCamera = false;
        Debug->DrawPlayerHitbox = true;

        GameState->Render = PushStruct(WorldArena, render);
        render *Render = GameState->Render;
        Render->FOV = 0.1f;
        Render->SStMeshesCount = 0;
        Render->SAnMeshesCount = 0;
        Render->MStMeshesCount = 0;
        Render->GrMeshesCount = 0;
        Render->Animator.Timer = 1.0f;

        GameState->Settings = PushStruct(WorldArena, app_settings);
        app_settings *Settings = GameState->Settings;
        Settings->RBFullscreenIsActive = false;
        Settings->RBWindowedIsActive = true;
        Settings->MouseSensitivity = 50.0;
        Settings->NewFrameRate = 60;
        Settings->RBCappedIsActive = true;
        Settings->RBUncappedIsActive = false;
        Settings->RBVSyncIsActive = false;

        GameState->Player = PushStruct(WorldArena, entity_player);
        entity_player *Player = GameState->Player;
        // Player->Position = V3(5, 5, 0);
        Player->P.ChunkX = -2;
        Player->P.ChunkY = -2;
        Player->P.Offset_ = V2(0, 0);
        Player->TmpZ = 2.7f; // высота камеры
        Player->dP = V2(0, 0);
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
        PlayerClip->Side = 50.0f;

        //
        // NOTE(me): Источники света
        //
        // directional light
        directional_light *DirLight = &Render->DirLight;
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
        AddEnvObjectsToRender(Render, EnvObjects);
        InitEnvVBOs(WorldArena, Render);

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
        AddGrassObjectsToRender(Render, GrassObjects);
        InitGrassVBO(WorldArena, Render);

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
        /*Render->ShadowMapSize = 10.0f;
        Render->NearPlane = 0.0f;
        Render->FarPlane = 100.0f;
        Render->ShadowCameraPitch = 65.0f;
        Render->ShadowCameraYaw = 315.0f;
        Render->SunPos = V3(-10.0f, -10.0f, 10.0f);*/
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
        Render->LightTexture = LoadTexture(&Render->LightgTextureName);

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

    // Local pointers for game_state sctructs
    render *Render = GameState->Render;
    app_settings *Settings = GameState->Settings;
    entity_player *Player = GameState->Player;
    entity_clip *PlayerClip = GameState->Clip;
    entity_envobject **EnvObjects = GameState->EnvObjects; // TODO(me): избавиться от 16
    entity_grassobject **GrassObjects = GameState->GrassObjects;
    world *World = GameState->World;
    world_position *CameraP = &GameState->CameraP;
    debug *Debug = GameState->Debug;

    //
    // NOTE(me): Inputs
    //
    if(!Input->ShowMouseCursorMode)
    {
        RotatePlayerCamera(Player, Input->MouseOffsetX, Input->MouseOffsetY, Settings->MouseSensitivity);
    }

    for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
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
        }
    }

    //
    // NOTE(me): Physics
    //
    u32 TileSpanX = 17 * 3;
    u32 TileSpanY = 9 * 3;
    rectangle2 CameraBounds = RectCenterDim(V2(0, 0), World->TileSideInMeters * V2((r32)TileSpanX, (r32)TileSpanY));

    memory_arena SimArena;
    InitializeArena(&SimArena, Memory->TransientStorageSize, Memory->TransientStorage);
    sim_region *SimRegion = BeginSim(&SimArena, GameState, GameState->World, GameState->CameraP, CameraBounds);

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
    // NOTE(me): ImGui init render
    //

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its
    // code to learn more about Dear ImGui!).
    if(GameState->ShowDemoWindow && Input->ShowMouseCursorMode)
    {
        ImGui::ShowDemoWindow(&GameState->ShowDemoWindow);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    if(Input->ShowMouseCursorMode)
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text."); // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window",
                        &GameState->ShowDemoWindow); // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &GameState->ShowAnotherWindow);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
        // ImGui::ColorEdit3("clear color", (float *)&GameState->ClearColor); // Edit 3 floats representing a color

        if(ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if(GameState->ShowAnotherWindow && Input->ShowMouseCursorMode)
    {
        ImGui::Begin("Another Window",
                     &GameState->ShowAnotherWindow); // Pass a pointer to our bool variable (the window will have a
                                                     // closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if(ImGui::Button("Close Me"))
            GameState->ShowAnotherWindow = false;
        ImGui::End();
    }

    // NOTE(me): Developer Menu
    if(Input->ShowMouseCursorMode)
    {
        ImGui::Begin("Developer Menu");
        ImGui::Text("This is test engine!");
        ImGui::Text("App avg %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("CursorPos=%f,%f", Input->MouseX, Input->MouseY);
        ImGui::Text("dtForFrame=%f", Input->dtForFrame);
        ImGui::Text("MouseOffset=%f,%f", Input->MouseOffsetX, Input->MouseOffsetY);
        ImGui::Spacing();

        ImGui::SetNextItemOpen(true);
        if(ImGui::CollapsingHeader("Settings"))
        {
            ImGui::Text("Change Display Mode");
            ImGui::SameLine();
            b32 ChangeDisplayMode = false; // менять ли текущий режим отображения окна win32?
            if(ImGui::RadioButton("Fullscreen", Settings->RBFullscreenIsActive))
            {
                if(!Settings->RBFullscreenIsActive) // если rb1 выключена и произошло нажатие
                {
                    Settings->RBFullscreenIsActive = true; // включаем rb1
                    Settings->RBWindowedIsActive = false;  // выключаем rb2
                    ChangeDisplayMode = true;
                }
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("Windowed", Settings->RBWindowedIsActive))
            {
                if(!Settings->RBWindowedIsActive) // если rb2 выключена и произошло нажатие
                {
                    Settings->RBWindowedIsActive = true;    // включаем rb2
                    Settings->RBFullscreenIsActive = false; // выключаем rb2
                    ChangeDisplayMode = true;
                }
            }

            if(ChangeDisplayMode)
            {
                Platform.ToggleFullscreen(Window);
            }
            ImGui::Spacing();

            b32 ToggleFrameRateCap = false;
            b32 ToggleVSync = false;
            ImGui::Text("Change Frame Rate Mode");
            ImGui::SameLine();
            if(ImGui::RadioButton("Capped", Settings->RBCappedIsActive))
            {
                if(!Settings->RBCappedIsActive)
                {
                    if(Settings->RBUncappedIsActive)
                    {
                        ToggleFrameRateCap = true;
                    }
                    if(Settings->RBVSyncIsActive)
                    {
                        ToggleVSync = true;
                        ToggleFrameRateCap = true;
                    }
                    Settings->RBCappedIsActive = true;
                    Settings->RBUncappedIsActive = false;
                    Settings->RBVSyncIsActive = false;
                }
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("Uncapped", Settings->RBUncappedIsActive))
            {
                if(!Settings->RBUncappedIsActive)
                {
                    if(Settings->RBCappedIsActive)
                    {
                        ToggleFrameRateCap = true;
                    }
                    if(Settings->RBVSyncIsActive)
                    {
                        ToggleVSync = true;
                    }
                    Settings->RBCappedIsActive = false;
                    Settings->RBUncappedIsActive = true;
                    Settings->RBVSyncIsActive = false;
                }
            }
            ImGui::SameLine();
            if(ImGui::RadioButton("VSync", Settings->RBVSyncIsActive))
            {
                if(!Settings->RBVSyncIsActive)
                {
                    if(Settings->RBCappedIsActive)
                    {
                        ToggleFrameRateCap = true;
                    }
                    Settings->RBUncappedIsActive = false;
                    Settings->RBCappedIsActive = false;
                    Settings->RBVSyncIsActive = true;
                    ToggleVSync = true;
                }
            }

            if(ToggleVSync)
            {
                Platform.ToggleVSync();
            }
            if(ToggleFrameRateCap)
            {
                Platform.ToggleFrameRateCap();
            }
            ImGui::Spacing();

            // TODO(me): vsync/unlimited fps/custom fps???
            if(Settings->RBCappedIsActive)
            {
                ImGui::Text("Set FrameRate");
                ImGui::SliderInt("##FrameRate", &Settings->NewFrameRate, 60, 240);
                Platform.SetFrameRate(Settings->NewFrameRate);
            }
            ImGui::Spacing();

            ImGui::Text("Change Mouse Sensitivity");
            ImGui::SliderFloat("##MouseSensitivity", &Settings->MouseSensitivity, 0.0f, 100.0f);
            ImGui::Spacing();

            ImGui::Checkbox("Process Animations", &Debug->ProcessAnimations);
        }

        if(ImGui::CollapsingHeader("Memory"))
        {
            ImGui::Text("PermanentStorage");
            ImGui::Text("Size=%d (Kb)", GameState->WorldArena.Size / 1024);
            ImGui::Text("Used=%d (Kb)", GameState->WorldArena.Used / 1024);

            ImGui::Text("TransientStorage");
            ImGui::Text("Size=%d (Kb)", SimArena.Size / 1024);
            ImGui::Text("Used=%d (Kb)", SimArena.Used / 1024);
        }

        if(ImGui::CollapsingHeader("World"))
        {
            // ImGui::Text("ChunkDimInMeters=%d", World->ChunkDimInMeters);
            // ImGui::Text("ChunksCount=%d", World->ChunksCount);
            ImGui::Checkbox("DrawSimRegionBounds", &Debug->DrawSimRegionBounds);
            ImGui::Checkbox("DrawSimRegionUpdatableBounds", &Debug->DrawSimRegionUpdatableBounds);
            ImGui::Checkbox("DrawSimChunks", &Debug->DrawSimChunks);
            ImGui::Checkbox("DrawChunkWhereCamera", &Debug->DrawChunkWhereCamera);
            ImGui::Checkbox("DrawPlayerHitbox", &Debug->DrawPlayerHitbox);
            /*world_position PlayerP;
            PlayerP.ChunkX = FloorReal32ToInt32(Player->Position.x / World->ChunkDimInMeters);
            PlayerP.ChunkY = FloorReal32ToInt32(Player->Position.y / World->ChunkDimInMeters);
            ImGui::Text("ChunkX=%d", PlayerP.ChunkX);
            ImGui::Text("ChunkY=%d", PlayerP.ChunkY);
            PlayerP.Offset_.x = Player->Position.x                         //
                                - PlayerP.ChunkX * World->ChunkDimInMeters // chunk in meters
                                - World->ChunkDimInMeters * 0.5f;          // to center
            PlayerP.Offset_.y = Player->Position.y                         //
                                - PlayerP.ChunkY * World->ChunkDimInMeters // chunk in meters
                                - World->ChunkDimInMeters * 0.5f;          // to center
            ImGui::Text("Offset_.x=%f", PlayerP.Offset_.x);
            ImGui::Text("Offset_.y=%f", PlayerP.Offset_.y);

            world_position TestWallP;
            TestWallP.ChunkX = -1;
            TestWallP.ChunkY = -1;
            TestWallP.Offset_ = V2(0, 0);
            v2 CameraSP = GetCameraSpaceP(World, &PlayerP, &TestWallP);

            ImGui::Text("TestWallP");
            ImGui::Text("CameraSPX=%f", CameraSP.x);
            ImGui::Text("CameraSPY=%f", CameraSP.y);*/
        }

        if(ImGui::CollapsingHeader("Camera"))
        {
            ImGui::Text("CameraP");
            ImGui::Text("ChunkX=%d", GameState->CameraP.ChunkX);
            ImGui::Text("ChunkY=%d", GameState->CameraP.ChunkY);
            ImGui::Text("Offset_.x=%f", GameState->CameraP.Offset_.x);
            ImGui::Text("Offset_.y=%f", GameState->CameraP.Offset_.y);
            /*ImGui::InputFloat("Player X", &Player->Position.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("Player Y", &Player->Position.y, 0.5, 2, "%.10f", 0);*/
            ImGui::InputFloat("CameraZOffset", &Player->CameraZOffset, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("CameraPitch", &Player->CameraPitch, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("CameraYaw", &Player->CameraYaw, 0.5, 2, "%.10f", 0);
        }

        if(ImGui::CollapsingHeader("Entities"))
        {
            ImGui::Text("EnvObjectsCount=%d", Render->EnvObjectsCount);
            if(ImGui::TreeNode("EnvObjects"))
            {
                for(u32 i = 0; i < Render->EnvObjectsCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)EnvObjects[i]->Model->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("GrassObjectsCount=%d", Render->GrassObjectsCount);
            if(ImGui::TreeNode("GrassObjects"))
            {
                for(u32 i = 0; i < Render->GrassObjectsCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)GrassObjects[i]->Model->Name.Data);
                }
                ImGui::TreePop();
            }
        }

        if(ImGui::CollapsingHeader("Render"))
        {
            ImGui::Text("Environment Objects Rendering System");
            ImGui::Text("SStMeshesCount=%d", Render->SStMeshesCount);
            if(ImGui::TreeNode("SStMeshes"))
            {
                for(u32 i = 0; i < Render->SStMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->SStMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("SStVerticesCountSum=%d", Render->SStVerticesCountSum);
            ImGui::Text("SAnMeshesCount=%d", Render->SAnMeshesCount);
            if(ImGui::TreeNode("SAnMeshes"))
            {
                for(u32 i = 0; i < Render->SAnMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->SAnMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("SAnVerticesCountSum=%d", Render->SAnVerticesCountSum);
            ImGui::Text("MStMeshesCount=%d", Render->MStMeshesCount);
            if(ImGui::TreeNode("MStMeshes"))
            {
                for(u32 i = 0; i < Render->MStMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->MStMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("MStVerticesCountSum=%d", Render->MStVerticesCountSum);
            ImGui::Text("MStInstancesCountSum=%d", Render->MStInstancesCountSum);
            ImGui::Spacing();
            ImGui::Text("Grass Objects Rendering System");
            ImGui::Text("GrMeshesCount=%d", Render->GrMeshesCount);
            if(ImGui::TreeNode("GrMeshes"))
            {
                for(u32 i = 0; i < Render->GrMeshesCount; i++)
                {
                    ImGui::Text("%d %s", i, (char *)Render->GrMeshes[i]->Name.Data);
                }
                ImGui::TreePop();
            }
            ImGui::Text("GrVerticesCountSum=%d", Render->GrVerticesCountSum);
            ImGui::Text("GrInstancesCountSum=%d", Render->GrInstancesCountSum);
        }

        if(ImGui::CollapsingHeader("Light Sources"))
        {
            ImGui::Text("Directional");
            ImGui::InputFloat("DLPosX", &Render->DirLight.WorldDirection.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLPosY", &Render->DirLight.WorldDirection.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLPosZ", &Render->DirLight.WorldDirection.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLAmbientIntensity", &Render->DirLight.Base.AmbientIntensity, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("DLDiffuseIntensity", &Render->DirLight.Base.DiffuseIntensity, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("DLColor", (float *)&Render->DirLight.Base.Color.E);
            ImGui::Spacing();

            ImGui::Text("Point");
            ImGui::InputFloat("PLPosX", &Render->PointLights[0].WorldPosition.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLPosY", &Render->PointLights[0].WorldPosition.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLPosZ", &Render->PointLights[0].WorldPosition.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAmbientIntensity", &Render->PointLights[0].Base.AmbientIntensity, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLDiffuseIntensity", &Render->PointLights[0].Base.DiffuseIntensity, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAttConstant", &Render->PointLights[0].Atten.Constant, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAttLinear", &Render->PointLights[0].Atten.Linear, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("PLAttExp", &Render->PointLights[0].Atten.Exp, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("PLColor", (float *)&Render->PointLights[0].Base.Color.E);
            ImGui::Spacing();

            ImGui::Text("Spot");
            ImGui::InputFloat("SLPosX", &Render->SpotLights[0].Base.WorldPosition.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SLPosY", &Render->SpotLights[0].Base.WorldPosition.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SLPosZ", &Render->SpotLights[0].Base.WorldPosition.z, 0.5, 2, "%.10f", 0);
            ImGui::ColorEdit3("SLColor", (float *)&Render->SpotLights[0].Base.Base.Color.E);
        }

        if(ImGui::CollapsingHeader("Shadows"))
        {
            ImGui::InputFloat("ShadowMapSize", &Render->ShadowMapSize, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("NearPlane", &Render->NearPlane, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("FarPlane", &Render->FarPlane, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowCameraPitch", &Render->ShadowCameraPitch, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("ShadowCameraYaw", &Render->ShadowCameraYaw, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SunPosX", &Render->SunPos.x, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SunPosY", &Render->SunPos.y, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("SunPosZ", &Render->SunPos.z, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("Bias", &Render->Bias, 0.001, 2, "%.10f", 0);
            ImGui::Text("ShadowMap");
            ImGui::Image((void *)(intptr_t)Render->DepthMap, ImVec2(1920 / 4, 1080 / 4), ImVec2(0, 0), ImVec2(1, -1));
        }

        if(ImGui::CollapsingHeader("Water"))
        {
            ImGui::InputFloat("WaterWaveSpeed", &Render->WaterWaveSpeed, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("WaterTiling", &Render->WaterTiling, 0.1, 2, "%.10f", 0);
            ImGui::InputFloat("WaterWaveStrength", &Render->WaterWaveStrength, 0.1, 2, "%.10f", 0);
            ImGui::InputFloat("WaterShineDamper", &Render->WaterShineDamper, 0.5, 2, "%.10f", 0);
            ImGui::InputFloat("WaterReflectivity", &Render->WaterReflectivity, 0.1, 2, "%.10f", 0);
            ImGui::Text("WaterReflTexture");
            ImGui::Image((void *)(intptr_t)Render->WaterReflTexture, ImVec2(1920 / 4, 1080 / 4), //
                         ImVec2(0, 0), ImVec2(1, -1));
            ImGui::Text("WaterRefrTexture");
            ImGui::Image((void *)(intptr_t)Render->WaterRefrTexture, ImVec2(1920 / 4, 1080 / 4), //
                         ImVec2(0, 0), ImVec2(1, -1));
        }

        ImGui::End();
    }

    // NOTE(me): Draw Log App
    if(Input->ShowMouseCursorMode)
    {
        Log.Draw("Log");
    }

    //
    // NOTE(me): Render
    //
    glfwGetFramebufferSize(Window, &Render->DisplayWidth, &Render->DisplayHeight);
    Render->AspectRatio = (r32)Render->DisplayWidth / (r32)Render->DisplayHeight;
    r32 ScreenCenterX = 0.5f * Render->DisplayWidth;
    r32 ScreenCenterY = 0.5f * Render->DisplayHeight;

    glViewport(0, 0, Render->DisplayWidth, Render->DisplayHeight);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-Render->AspectRatio * Render->FOV, Render->AspectRatio * Render->FOV, //
              -Render->FOV, Render->FOV, Render->FOV * 2, 1000);

    // set camera
    glRotatef(-Player->CameraPitch, 1.0f, 0.0f, 0.0f);
    glRotatef(-Player->CameraYaw, 0.0f, 0.0f, 1.0f);
    v2 WorldOrigin1 = {};
    glTranslatef(-WorldOrigin1.x, -WorldOrigin1.y, -10.0f);
    // glTranslatef(-CameraP->ChunkX * World->ChunkSideInMeters, -CameraP->ChunkY * World->ChunkSideInMeters, -10.0f);

    // draw oxyz
    glPushMatrix();
    glScalef(5, 5, 5);
    OGLDrawLinesOXYZ(V3(0, 0, 1), 1);
    glPopMatrix();

    // draw sim region bounds
    glLineWidth(1);
    glEnable(GL_LINE_SMOOTH);
    if(Debug->DrawSimRegionBounds)
    {
        DrawRectangleOutline(SimRegion->Bounds, 0.0f, V3(1, 1, 0));
    }
    if(Debug->DrawSimRegionUpdatableBounds)
    {
        DrawRectangleOutline(SimRegion->UpdatableBounds, 0.0f, V3(0, 1, 0));
    }
    glDisable(GL_LINE_SMOOTH);

    // draw border for chunks from sim region
    if(Debug->DrawSimChunks || Debug->DrawChunkWhereCamera)
    {
        world_position MinChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
        world_position MaxChunkP = MapIntoChunkSpace(World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));
        for(s32 ChunkY = MinChunkP.ChunkY; ChunkY <= MaxChunkP.ChunkY; ++ChunkY)
        {
            for(s32 ChunkX = MinChunkP.ChunkX; ChunkX <= MaxChunkP.ChunkX; ++ChunkX)
            {
                world_position ChunkP = {};
                ChunkP.ChunkX = ChunkX;
                ChunkP.ChunkY = ChunkY;
                world_difference Diff = Subtract(SimRegion->World, &ChunkP, &SimRegion->Origin);
                v2 ChunkMin = Diff.dXY - V2(0.5f * World->ChunkSideInMeters, 0.5f * World->ChunkSideInMeters);
                v2 ChunkMax = Diff.dXY + V2(0.5f * World->ChunkSideInMeters, 0.5f * World->ChunkSideInMeters);
                rectangle2 ChunkRect = {ChunkMin, ChunkMax};
                if(Debug->DrawSimChunks)
                {
                    DrawRectangleOutline(ChunkRect, -0.1f, V3(1, 0, 0));
                }
                if(Debug->DrawChunkWhereCamera && (CameraP->ChunkX == ChunkX) && (CameraP->ChunkY == ChunkY))
                {
                    // draw chunk rectangle from CameraP
                    DrawRectangle(ChunkRect, -0.1f, V3(0, 1, 1));
                }
            }
        }
    }

    // TODO(casey): Move this out into handmade_entity.cpp!
    entity_visible_piece_group PieceGroup;
    PieceGroup.GameState = GameState;
    sim_entity *Entity = SimRegion->Entities;
    for(u32 EntityIndex = 0; EntityIndex < SimRegion->EntityCount; ++EntityIndex, ++Entity)
    {
        if(Entity->Updatable)
        {
            v2 EntityMinCenterP = V2(Entity->P.x - 0.5f * Entity->Width, Entity->P.y - 0.5f * Entity->Height);
            v2 EntityMaxCenterP = V2(Entity->P.x + 0.5f * Entity->Width, Entity->P.y + 0.5f * Entity->Height);
            rectangle2 EntityRect = {EntityMinCenterP, EntityMaxCenterP};

            PieceGroup.PieceCount = 0;
            r32 dt = Input->dtForFrame;

            // TODO(casey): This is incorrect, should be computed after update!!!!
            /*r32 ShadowAlpha = 1.0f - 0.5f * Entity->Z;
            if(ShadowAlpha < 0)
            {
                ShadowAlpha = 0.0f;
            }*/

            move_spec MoveSpec = DefaultMoveSpec();
            v2 ddP = {};

            // hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];
            switch(Entity->Type)
            {
            case EntityType_Hero: {
                // TODO(casey): Now that we have some real usage examples, let's solidify
                // the positioning system!
                for(u32 ControlIndex = 0; ControlIndex < ArrayCount(GameState->ControlledHeroes); ++ControlIndex)
                {
                    controlled_hero *ConHero = GameState->ControlledHeroes + ControlIndex;

                    if(Entity->StorageIndex == ConHero->EntityIndex)
                    {
                        if(ConHero->dZ != 0.0f)
                        {
                            Entity->dZ = ConHero->dZ;
                        }

                        MoveSpec.UnitMaxAccelVector = true;
                        MoveSpec.Speed = 50.0f;
                        // MoveSpec.Drag = 8.0f;
                        MoveSpec.Drag = 1.0f;
                        // ddP = ConHero->ddP;
                        r32 Angle = Player->CameraYaw * Pi32 / 180;
                        ddP.x = ConHero->ddP.x * Cos(Angle) - ConHero->ddP.y * Sin(Angle);
                        ddP.y = ConHero->ddP.x * Sin(Angle) + ConHero->ddP.y * Cos(Angle);

                        /*if((ConHero->dSword.x != 0.0f) || (ConHero->dSword.y != 0.0f))
                        {
                            sim_entity *Sword = Entity->Sword.Ptr;
                            if(Sword && IsSet(Sword, EntityFlag_Nonspatial))
                            {
                                Sword->DistanceRemaining = 5.0f;
                                MakeEntitySpatial(Sword, Entity->P, 5.0f * ConHero->dSword);
                            }
                        }*/
                    }
                }

                // TODO(casey): Z!!!
                /*PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
                PushBitmap(&PieceGroup, &HeroBitmaps->Torso, V2(0, 0), 0, HeroBitmaps->Align);
                PushBitmap(&PieceGroup, &HeroBitmaps->Cape, V2(0, 0), 0, HeroBitmaps->Align);
                PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0, 0), 0, HeroBitmaps->Align);*/

                // DrawHitpoints(Entity, &PieceGroup);
                if(Debug->DrawPlayerHitbox)
                {
                    DrawRectangle(EntityRect, 0.0f, V3(0.0f, 1.0f, 0.0f));
                }
            }
            break;

            case EntityType_Wall: {
                DrawRectangle(EntityRect, 0.0f, V3(0.0f, 0.0f, 1.0f));
                // PushBitmap(&PieceGroup, &GameState->Tree, V2(0, 0), 0, V2(40, 80));
            }
            break;

                /*case EntityType_Sword:
                {
                    MoveSpec.UnitMaxAccelVector = false;
                    MoveSpec.Speed = 0.0f;
                    MoveSpec.Drag = 0.0f;

                    // TODO(casey) IMPORTANT(casey): Add the ability in the collision
                    // routines to understand a movement limit for an entity, and
                    // then update this routine to use that to know when to kill the
                    // sword.
                    // TODO(casey): Need to handle the fact that DistanceTraveled
                    // might not have enough distance for the total entity move
                    // for the frame!
                    v2 OldP = Entity->P;
                    real32 DistanceTraveled = Length(Entity->P - OldP);
                    Entity->DistanceRemaining -= DistanceTraveled;
                    if(Entity->DistanceRemaining < 0.0f)
                    {
                        MakeEntityNonSpatial(Entity);
                    }

                    PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
                    PushBitmap(&PieceGroup, &GameState->Sword, V2(0, 0), 0, V2(29, 10));
                } break;

                case EntityType_Familiar: {
                    sim_entity *ClosestHero = 0;
                    r32 ClosestHeroDSq = Square(10.0f); // NOTE(casey): Ten meter maximum search!

                    // TODO(casey): Make spatial queries easy for things!
                    sim_entity *TestEntity = SimRegion->Entities;
                    for(u32 TestEntityIndex = 0; TestEntityIndex < SimRegion->EntityCount;
                        ++TestEntityIndex, ++TestEntity)
                    {
                        if(TestEntity->Type == EntityType_Hero)
                        {
                            r32 TestDSq = LengthSq(TestEntity->P - Entity->P);
                            if(TestEntity->Type == EntityType_Hero)
                            {
                                TestDSq *= 0.75f;
                            }

                            if(ClosestHeroDSq > TestDSq)
                            {
                                ClosestHero = TestEntity;
                                ClosestHeroDSq = TestDSq;
                            }
                        }
                    }

                    if(ClosestHero && (ClosestHeroDSq > Square(3.0f)))
                    {
                        r32 Acceleration = 0.5f;
                        r32 OneOverLength = Acceleration / SquareRoot(ClosestHeroDSq);
                        ddP = OneOverLength * (ClosestHero->P - Entity->P);
                    }

                    MoveSpec.UnitMaxAccelVector = true;
                    MoveSpec.Speed = 50.0f;
                    MoveSpec.Drag = 8.0f;

                    Entity->tBob += dt;
                    if(Entity->tBob > (2.0f * Pi32))
                    {
                        Entity->tBob -= (2.0f * Pi32);
                    }
                    r32 BobSin = Sin(2.0f * Entity->tBob);
                    PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align,
                               (0.5f * ShadowAlpha) + 0.2f * BobSin, 0.0f);
                    PushBitmap(&PieceGroup, &HeroBitmaps->Head, V2(0, 0), 0.25f * BobSin, HeroBitmaps->Align);
                }
                break;

                case EntityType_Monstar: {
                    PushBitmap(&PieceGroup, &GameState->Shadow, V2(0, 0), 0, HeroBitmaps->Align, ShadowAlpha, 0.0f);
                    PushBitmap(&PieceGroup, &HeroBitmaps->Torso, V2(0, 0), 0, HeroBitmaps->Align);

                    DrawHitpoints(Entity, &PieceGroup);
                }
                break;*/

            default: {
                InvalidCodePath;
            }
            break;
            }

            if(!IsSet(Entity, EntityFlag_Nonspatial))
            {
                MoveEntity(SimRegion, Entity, Input->dtForFrame, &MoveSpec, ddP);
            }

            // r32 EntityGroundPointX = ScreenCenterX + MetersToPixels * Entity->P.x;
            // r32 EntityGroundPointY = ScreenCenterY - MetersToPixels * Entity->P.y;
            // r32 EntityZ = -MetersToPixels * Entity->Z;
            r32 EntityGroundPointX = ScreenCenterX + Entity->P.x;
            r32 EntityGroundPointY = ScreenCenterY - Entity->P.y;
            r32 EntityZ = -Entity->Z;

            for(u32 PieceIndex = 0; PieceIndex < PieceGroup.PieceCount; ++PieceIndex)
            {
                entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;
                v2 Center = {EntityGroundPointX + Piece->Offset.x,
                             EntityGroundPointY + Piece->Offset.y + Piece->OffsetZ + Piece->EntityZC * EntityZ};
                /*if(Piece->Bitmap)
                {
                    DrawBitmap(Buffer, Piece->Bitmap, Center.x, Center.y, Piece->A);
                }
                else
                {*/
                // v2 HalfDim = 0.5f * MetersToPixels * Piece->Dim;
                v2 HalfDim = 0.5f * Piece->Dim;
                // DrawRectangle(Buffer, Center - HalfDim, Center + HalfDim, Piece->R, Piece->G, Piece->B);
                r32 TmpZ = 0.0f;
                // DrawRectangle(Center - HalfDim, Center + HalfDim, TmpZ, V3(Piece->R, Piece->G, Piece->B));
                // }
            }
        }
    }

    // world_position WorldOrigin = {};
    // world_difference Diff = Subtract(SimRegion->World, &WorldOrigin, &SimRegion->Origin);
    //   DrawRectangle(Buffer, Diff.dXY, V2(10.0f, 10.0f), 1.0f, 1.0f, 0.0f);
    //  DrawRectangle(Diff.dXY, V2(10.0f, 10.0f), TmpZ, V3(1.0f, 0.0f, 0.0f));
    // v2 CameraChunkMin = Diff.dXY;
    // v2 CameraChunkMax = Diff.dXY + V2(10, 10);
    // rectangle2 CameraChunkRect = {CameraChunkMin, CameraChunkMax};
    // DrawRectangle(CameraChunkRect, -0.1f, V3(1, 0, 0));

    EndSim(SimRegion, GameState);

#if 1
    /*
    glViewport(0, 0, Render->DisplayWidth, Render->DisplayHeight);
    glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    RenderDebugElements(World, Render, Player, PlayerClip);
    */
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
    // ImGui rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(Window);
}