struct entity_player
{
    v3 Position;
    v2 dP; // derivative от позиции игрока | производная от позиции игрока | v - скорость игрока

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
    v2 CenterPos;
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

// camera
internal void OGLSetCameraOnPlayer(entity_player *Player);
internal void RotatePlayerCamera(entity_player *Player, r32 ZAngle, r32 XAngle, r32 Sensitivity);

// movement
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