// NOTE(ezexff): Platform-independent renderer
enum renderer_entry_type
{
    RendererEntryType_renderer_entry_clear,
    
    RendererEntryType_renderer_entry_rect_on_ground,
    RendererEntryType_renderer_entry_rect_outline_on_ground,
    RendererEntryType_renderer_entry_bitmap_on_ground,
    
    RendererEntryType_renderer_entry_cube,
    RendererEntryType_renderer_entry_cube_outline,
    
    RendererEntryType_renderer_entry_rect_on_screen,
    RendererEntryType_renderer_entry_bitmap_on_screen,
    
    RendererEntryType_renderer_entry_terrain_chunk,
    RendererEntryType_renderer_entry_line,
    
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

struct renderer_entry_cube
{
    v4 Color;
    v3 P;
    v3 Dim;
};

struct renderer_entry_cube_outline
{
    v4 Color;
    v3 P;
    v3 Dim;
    r32 LineWidth;
};

struct renderer_entry_line
{
    v4 Color;
    v3 P1;
    v3 P2;
    r32 LineWidth;
};

typedef renderer_entry_rect_on_ground renderer_entry_rect_on_screen;
typedef renderer_entry_bitmap_on_ground renderer_entry_bitmap_on_screen;

struct renderer_entry_terrain_chunk
{
    u32 PositionsCount;
    v3 *Positions;
    u32 IndicesCount;
    u32 *Indices;
};