enum frame_effect
{
    FrameEffect_Abberation,
    FrameEffect_Blur,
    FrameEffect_Emboss,
    FrameEffect_Grayscale,
    FrameEffect_Inverse,
    FrameEffect_Sepia,
    FrameEffect_Normal,
    
    FrameEffect_Count,
};

struct renderer_camera
{
    v3 P;
    v3 Angle;
};

struct renderer_skybox
{
    u32 Texture;
    u32 VAO;
    u32 VBO;
    b32 IsTextureParametersInitialized;
    
    loaded_bitmap Bitmaps[6];
};

struct point_light_array
{
    u32 Count;
    point_light Point;
};

struct spot_light_array
{
    u32 Count;
    spot_light Spot;
};

struct renderer_lighting
{
    v3 TestSunP; // TODO(ezexff): remove (lighting shine)
    
    directional_light DirLight;
    
    point_light_array *PointLights;
    spot_light_array *SpotLights;
};

struct renderer_shadowmap
{
    v2u Dim;
    m4x4 Proj, View, Model;
    u32 Texture;
    u32 FBO;
    r32 Size;
    r32 NearPlane, FarPlane;
    r32 CameraPitch, CameraYaw;
    v3 CameraP;
    r32 Bias;
};

struct renderer_water_reflection
{
    v2u Dim;
    u32 FBO;
    u32 ColorTexture;
    u32 DepthRBO;
    loaded_bitmap *DuDv;
    loaded_bitmap *NormalMap;
};

struct renderer_water_refraction
{
    v2u Dim;
    u32 FBO;
    u32 ColorTexture;
    u32 DepthTexture;
};

struct renderer_water
{
    r32 WaveSpeed;
    r32 MoveFactor;
    r32 Tiling;
    r32 WaveStrength;
    r32 ShineDamper;
    r32 Reflectivity;
    
    renderer_water_reflection Reflection;
    renderer_water_refraction Refraction;
    
    // TODO(ezexff): Test
    //world_position TestWaterP;
    //v3 TestWaterRelP;
    //world_position TestSunP;
};

struct index_array
{
    u32 Count;
    u32 *Indices;
};

struct renderer_terrain
{
    u32 TileCount; // NOTE(ezexff): Tile count per row!!!
    r32 MaxHeight;
    
    // TODO(ezexff): Mb move somewhere else?
    r32 TileWidth;
    r32 TileHeight;
    
    ground_buffer_array GroundBufferArray;
    index_array IndexArray;
    
    material Material;
    loaded_bitmap *Bitmap;
};

struct tile_sort_entry
{
    u32 Offset;
    r32 SortKey;
};

struct renderer_push_buffer
{
    u32 Size;
    u8 *Base;
    u8 Memory[65536];
    
    u32 ElementCount;
    tile_sort_entry *SortEntryArray;
};

struct renderer
{
    m4x4 Proj, View, Model;
    r32 FOV;
    v4 ClearColor;
    v4 CutPlane;
    u32 Flags; // renderer_flags
    
    renderer_camera Camera;
    
    renderer_skybox *Skybox;
    renderer_lighting *Lighting;
    renderer_shadowmap *ShadowMap;
    renderer_water *Water;
    renderer_terrain *Terrain;
    
    renderer_push_buffer PushBufferUI;
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
    RendererEntryType_renderer_entry_rect_on_ground,
    RendererEntryType_renderer_entry_rect_outline_on_ground,
    RendererEntryType_renderer_entry_bitmap_on_ground,
    RendererEntryType_renderer_entry_cube,
    RendererEntryType_renderer_entry_cube_outline,
    //RendererEntryType_renderer_entry_terrain_chunk,
    RendererEntryType_renderer_entry_line,
    //RendererEntryType_renderer_entry_water,
    
    RendererEntryType_renderer_entry_rect_on_screen,
    RendererEntryType_renderer_entry_bitmap_on_screen,
};

// TODO(ezexff): Test ortho push buffer with sort
//~

/* 
enum
{
    RendererPushBufferType_Ortho,
    RendererPushBufferType_Frustum,
    
    RendererPushBufferType_Count
};

PushRect(RendererPushBufferType_Ortho, Pos, Dim, ...);
 */

struct renderer_ortho_entry_rect
{
    v4 Color;
    v2 P;
    v2 Dim;
};

enum renderer_ortho_entry_type
{
    RendererOrthoEntryType_renderer_ortho_entry_rect,
    RendererOrthoEntryType_renderer_ortho_entry_bitmap,
};

struct renderer_entry_header
{
    union
    {
        renderer_entry_type Type;
        renderer_ortho_entry_type OrthoType;
    };
};
//~

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

/* 
struct renderer_entry_terrain_chunk
{
    u32 PositionsCount;
    v3 *Positions;
    u32 IndicesCount;
    u32 *Indices;
};
 */

typedef renderer_entry_rect_on_ground renderer_entry_rect_on_screen;
typedef renderer_entry_bitmap_on_ground renderer_entry_bitmap_on_screen;

struct renderer_entry_water
{
    v3 P;
    v2 Dim;
};