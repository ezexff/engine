// NOTE(ezexff): Platform-independent renderer
enum renderer_entry_type
{
    RendererEntryType_renderer_entry_clear,
    RendererEntryType_renderer_entry_rect_on_ground,
    RendererEntryType_renderer_entry_rect_outline_on_ground,
    //RenderGroupEntryType_render_entry_bitmap,
};

struct renderer_entry_header
{
    renderer_entry_type Type;
};

struct renderer_entry_clear
{
    v4 Color;
};

struct renderer_entry_rect_on_ground
{
    v4 Color;
    v2 P;
    v2 Dim;
};

struct renderer_entry_rect_outline_on_ground
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