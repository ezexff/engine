struct entity_player
{
    v3 Position;
    v2 dP; // derivative от позиции игрока | производная от позиции игрока | v - скорость игрока

    // на плоскости (для коллизий)
    r32 Width;
    r32 Height;

    r32 CameraXRot;    // поворот камеры по горизонтали
    r32 CameraZRot;    // поворот камеры по вертикали
    r32 CameraYOffset; // высота взгляда
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

    // рендер
    u32 InstancingCount; // 0 - без инстансинга
    m4x4 *InstancingTransformMatrices;
    loaded_model *Model;
    // TODO(me) loaded_texture?
};