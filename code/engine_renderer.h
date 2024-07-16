// NOTE(ezexff): Platform-independent renderer
enum renderer_entry_type
{
    RendererEntryType_renderer_entry_clear,
    RendererEntryType_renderer_entry_rect_on_ground,
    RendererEntryType_renderer_entry_rect_outline_on_ground,
    RendererEntryType_renderer_entry_bitmap_on_ground,
    
    RendererEntryType_renderer_entry_rect_on_screen,
    RendererEntryType_renderer_entry_bitmap_on_screen,
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

struct renderer_entry_bitmap_on_ground
{
    loaded_bitmap *Bitmap;
    v2 P;
    v2 Dim;
    r32 Repeat;
    //b32 FlipVertically;
};

typedef renderer_entry_rect_on_ground renderer_entry_rect_on_screen;
typedef renderer_entry_bitmap_on_ground renderer_entry_bitmap_on_screen;