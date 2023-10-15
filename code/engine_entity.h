struct entity_player
{
    // v3 Position;
    v2 dP; // derivative от позиции игрока | производная от позиции игрока | v - скорость игрока
    // world_position P;
    r32 TmpZ;

    // на плоскости (для коллизий)
    r32 Width;
    r32 Height;

    r32 CameraPitch; // pitch поворот камеры по вертикали
    r32 CameraYaw;   // yaw поворот камеры по горизонтали
    r32 CameraPitchInversed;

    r32 CameraZOffset; // высота взгляда
};

struct entity_clip
{
    // world_position P;
    //  v2 P; // in camera space
    r32 Side;
};

struct entity_envobject
{
    m4x4 TranslateMatrix;
    m4x4 RotateMatrix;
    m4x4 ScaleMatrix;
    m4x4 TransformMatrix;

    u32 InstancingCount; // 0 - без инстансинга
    m4x4 *InstancingTransformMatrices;
    loaded_model *Model;
};

struct entity_grassobject
{
    u32 InstancingCount;
    m4x4 *InstancingTransformMatrices;
    loaded_model *Model;
};

// movement
/*
internal void MovePlayerEOM(entity_player *Player, entity_clip *PlayerClip, //
                            v2 ddPFromKeys, r32 Speed, r32 dt);

// other
internal r32 TerrainGetHeight(entity_envobject *Terrain, r32 x, r32 y);
internal m4x4 *CreateInstancingTransformMatrices(memory_arena *WorldArena,  //
                                                 entity_envobject *Terrain, //
                                                 u32 Count,                 //
                                                 v3 SMM,                    // Scale rand() Min, Max, Precision
                                                 v3 Rotate,                 // Rotate X, Y, Z
                                                 v3 RXMM,                   // Rotate X rand() Min, Max, Precision
                                                 v3 RYMM,                   // Rotate Y rand() Min, Max, Precision
                                                 v3 RZMM);                  // Rotate Z rand() Min, Max, Precision
                                                 */
// TODO(me): remove above

#define InvalidP V3(100000.0f, 100000.0f, 100000.0f)

inline bool32 IsSet(sim_entity *Entity, uint32 Flag)
{
    bool32 Result = Entity->Flags & Flag;

    return (Result);
}

inline void AddFlags(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags |= Flag;
}

inline void ClearFlags(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void MakeEntityNonSpatial(sim_entity *Entity)
{
    AddFlags(Entity, EntityFlag_Nonspatial);
    Entity->P = InvalidP;
}

inline void MakeEntitySpatial(sim_entity *Entity, v3 P, v3 dP)
{
    ClearFlags(Entity, EntityFlag_Nonspatial);
    Entity->P = P;
    Entity->dP = dP;
}

inline v3 GetEntityGroundPoint(sim_entity *Entity, v3 ForEntityP)
{
    v3 Result = ForEntityP;

    return (Result);
}

inline v3 GetEntityGroundPoint(sim_entity *Entity)
{
    v3 Result = GetEntityGroundPoint(Entity, Entity->P);

    return (Result);
}

inline real32 GetStairGround(sim_entity *Entity, v3 AtGroundPoint)
{
    Assert(Entity->Type == EntityType_Stairwell);

    rectangle2 RegionRect = RectCenterDim(Entity->P.xy, Entity->WalkableDim);
    v2 Bary = Clamp01(GetBarycentric(RegionRect, AtGroundPoint.xy));
    real32 Result = Entity->P.z + Bary.y * Entity->WalkableHeight;

    return (Result);
}
