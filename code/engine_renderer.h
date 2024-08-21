struct renderer_camera
{
    v3 P;
    v3 Angle;
};

struct renderer
{
    r32 FOV;
    v4 ClearColor;
    u32 Flags; // renderer_flags
    
    renderer_camera Camera;
};

enum renderer_flags
{
    RendererFlag_Skybox = (1 << 0),
    RendererFlag_Lighting = (1 << 1),
    RendererFlag_Shadows = (1 << 2),
    RendererFlag_Water = (1 << 3),
    RendererFlag_Terrain = (1 << 4),
    RendererFlag_PolygonFill = (1 << 5),
};

inline b32 IsSet(renderer *Renderer, u32 Flag)
{
    b32 Result = Renderer->Flags & Flag;
    
    return(Result);
}

inline void AddFlags(renderer *Renderer, u32 Flag)
{
    Renderer->Flags |= Flag;
}

inline void ClearFlags(renderer *Renderer, u32 Flag)
{
    Renderer->Flags &= ~Flag;
}

inline void FlipFlag(renderer *Renderer, u32 Flag)
{
    if(IsSet(Renderer, Flag))
    {
        ClearFlags(Renderer, Flag);
    }
    else
    {
        AddFlags(Renderer, Flag);
    }
}

//~ NOTE(ezexff): Push buffer
enum renderer_entry_type
{
    RendererEntryType_renderer_entry_clear,
    
    RendererEntryType_renderer_entry_rect_on_ground,
    RendererEntryType_renderer_entry_rect_outline_on_ground,
    RendererEntryType_renderer_entry_bitmap_on_ground,
    RendererEntryType_renderer_entry_cube,
    RendererEntryType_renderer_entry_cube_outline,
    RendererEntryType_renderer_entry_terrain_chunk,
    RendererEntryType_renderer_entry_line,
    RendererEntryType_renderer_entry_water,
    
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

struct renderer_entry_terrain_chunk
{
    u32 PositionsCount;
    v3 *Positions;
    u32 IndicesCount;
    u32 *Indices;
};

// TODO(ezexff): Complete implementation
struct renderer_entry_water
{
    v3 P;
    v2 Dim;
};

typedef renderer_entry_rect_on_ground renderer_entry_rect_on_screen;
typedef renderer_entry_bitmap_on_ground renderer_entry_bitmap_on_screen;

struct renderer_water_list
{
    renderer_entry_water *Entry;
    renderer_entry_water *Next;
};