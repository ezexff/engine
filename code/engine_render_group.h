struct render_basis
{
    v3 P;
};

struct render_entity_basis
{
    render_basis *Basis;
    v3 Offset;
};

struct render_entry_clear
{
    v4 Color;
};

struct render_entry_rectangle
{
    render_entity_basis EntityBasis;
    v4 Color;
    v2 Dim;
};

struct render_entry_rectangle_outline
{
    render_entity_basis EntityBasis;
    v4 Color;
    v2 Dim;
    r32 LineWidth;
};

struct render_entry_texture
{
    render_entity_basis EntityBasis;
    u32 Texture;
    v2 Dim;
    b32 FlipVertically;
    r32 Repeat;
};

enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_bitmap,
    RenderGroupEntryType_render_entry_rectangle,
    RenderGroupEntryType_render_entry_rectangle_outline,
    RenderGroupEntryType_render_entry_coordinate_system,
    RenderGroupEntryType_render_entry_texture,
};

struct render_group_entry_header
{
    render_group_entry_type Type;
};

struct render_group
{
    b32 IsOrthogonal;
    r32 CameraPitch;
    r32 CameraYaw;
    r32 CameraRenderZ;

    render_basis *DefaultBasis;

    uint32 MaxPushBufferSize;
    uint32 PushBufferSize;
    uint8 *PushBufferBase;
};