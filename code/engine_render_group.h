enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_bitmap,
    RenderGroupEntryType_render_entry_rectangle,
    RenderGroupEntryType_render_entry_rectangle_outline,
    RenderGroupEntryType_render_entry_coordinate_system,
    RenderGroupEntryType_render_entry_texture,
    RenderGroupEntryType_render_entry_model,
};

struct render_group_entry_header
{
    render_group_entry_type Type;
};

/*struct render_basis
{
    v3 P;
};

struct render_entity_basis
{
    render_basis *Basis;
    v3 Offset;
};*/

struct render_entry_clear
{
    v4 Color;
};

struct render_entry_rectangle
{
    v4 Color;
    v2 P;
    v2 Dim;
};

struct render_entry_rectangle_outline
{
    v4 Color;
    v2 P;
    v2 Dim;
    r32 LineWidth;
};

struct render_entry_texture
{
    u32 Texture; // OpenGl id
    v2 P;
    v2 Dim;
    b32 FlipVertically;
    r32 Repeat;
};

struct render_entry_model
{
    loaded_model *Model;
    v2 P;
    v2 Dim;
    b32 IsFill;
};

struct render_transform
{
    // NOTE(casey): Camera parameters
    // real32 MetersToPixels; // NOTE(casey): This translates meters _on the monitor_ into pixels _on the monitor_
    // v2 ScreenCenter;

    // real32 FocalLength;
    // real32 DistanceAboveTarget;

    // v3 OffsetP;
    // real32 Scale;

    b32 IsOrthogonal;

    r32 CameraPitch;
    r32 CameraYaw;
    r32 CameraRenderZ;

    // v2 ScreenCenter;

    v3 OffsetP;
    // r32 Scale;
};

struct render_group
{

    // v2 MonitorHalfDimInMeters;

    render_transform Transform;

    // render_basis *DefaultBasis;

    uint32 MaxPushBufferSize;
    uint32 PushBufferSize;
    uint8 *PushBufferBase;
};