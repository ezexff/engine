// NOTE(ezexff): Platform-independent renderer
enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_rect_on_ground,
    RenderGroupEntryType_render_entry_rect_outline_on_ground,
    //RenderGroupEntryType_render_entry_bitmap,
};

struct render_group_entry_header
{
    render_group_entry_type Type;
};

struct render_entry_clear
{
    v4 Color;
};

struct render_entry_rect_on_ground
{
    v4 Color;
    v2 P;
    v2 Dim;
};

struct render_entry_rect_outline_on_ground
{
    v4 Color;
    v2 P;
    v2 Dim;
    r32 LineWidth;
};

/*struct render_entry_texture
{
    u32 Texture; // OpenGl id
    v2 P;
    v2 Dim;
    b32 FlipVertically;
    r32 Repeat;
};*/