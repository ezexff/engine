struct entity_player
{
    // v3 Position;
    v2 dP; // derivative от позиции игрока | производная от позиции игрока | v - скорость игрока
    world_position P;
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
    world_position P;
    // v2 P; // in camera space
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

struct low_entity
{
    // TODO(casey): It's kind of busted that P's can be invalid here,
    // AND we store whether they would be invalid in the flags field...
    // Can we do something better here?
    world_position P;
    sim_entity Sim;
};

#define InvalidP V2(100000.0f, 100000.0f)

inline bool32 IsSet(sim_entity *Entity, uint32 Flag)
{
    bool32 Result = Entity->Flags & Flag;

    return (Result);
}

inline void AddFlag(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags |= Flag;
}

inline void ClearFlag(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void MakeEntityNonSpatial(sim_entity *Entity)
{
    AddFlag(Entity, EntityFlag_Nonspatial);
    Entity->P = InvalidP;
}

inline void MakeEntitySpatial(sim_entity *Entity, v2 P, v2 dP)
{
    ClearFlag(Entity, EntityFlag_Nonspatial);
    Entity->P = P;
    Entity->dP = dP;
}