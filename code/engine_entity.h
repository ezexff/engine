struct entity_player
{
    v3 Position;
    v2 dP; // derivative от позиции игрока | производная от позиции игрока | v - скорость игрока

    r32 CameraXRot;    // поворот камеры по горизонтали
    r32 CameraZRot;    // поворот камеры по вертикали
    r32 CameraYOffset; // высота взгляда
};

struct entity_envobject
{
    v3 Position;
    r32 Scale;
    r32 Angle;
    v3 Rotate;

    // рендер
    u32 InstancingCount; // 0 - без инстансинга
    loaded_model *Model;
    // TODO(me) loaded_texture?
};