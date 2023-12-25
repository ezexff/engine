//
// NOTE(me): Texture
//
struct loaded_texture
{
    string Name;
    int32 Width;
    int32 Height;
    int32 NrChannels;
    u32 ID; // OpenGL texture
    
    // NOTE(me): Only for buffer rendering
    // TODO(me): мб убрать отсюда?
    u32 DepthTexture;
    u32 FBO;
};

//
// NOTE(me): 3d-model
//
struct node
{
    string Name;
    m4x4 Transformation;
    
    node *Parent;
    
    u32 ChildrenCount;
    node **Children;
};

struct position_key
{
    r64 Time;
    v3 Value;
};

struct rotation_key
{
    r64 Time;
    v4 Value;
};

typedef position_key scaling_key;

struct node_anim
{
    string Name;
    
    u32 PositionKeysCount;
    position_key *PositionKeys;
    
    u32 RotationKeysCount;
    rotation_key *RotationKeys;
    
    u32 ScalingKeysCount;
    scaling_key *ScalingKeys;
};

struct animation // AiAnimation
{
    string Name;
    
    r64 Duration;        // Duration of the animation in ticks
    r64 TicksPerSeconds; // Ticks per second
    
    u32 ChannelsCount;   // The number of bone animation channels
    node_anim *Channels; // The node animation channels
};

struct material
{
    string Name;
    v4 Ambient;
    v4 Diffuse;
    v4 Specular;
    v4 Emission;
    r32 Shininess;
    
    b32 WithTexture;
    loaded_texture Texture;
    // string TextureName;
    // u32 Texture;
};

struct single_mesh
{
    string Name;
    u32 VerticesCount;
    
    v3 *Positions;
    v2 *TexCoords;
    v3 *Normals;
    v3 *Tangents; // Normal mapping
    
    u32 IndicesCount;
    u32 *Indices;
    
    u32 *BoneIDs; // index1, index2, index3, index4, ...
    r32 *Weights; // weight1, weight2, weight3, weight4, ...
    
    b32 WithMaterial;
    material Material;
    
    b32 WithAnimations; // модель с костями и маркерами времени
    
    u32 BonesCount;
    string *BoneNames;
    m4x4 *BoneOffsets; // локальные смещения костей (относительно друг друга)
    
    node *BoneTransformsHierarchy; // глобальные преобразования костей (относительно всего меша)
    
    u32 AnimationsCount;
    animation *Animations;
    
    b32 WithSceneTransform;
    m4x4 SceneTransform;
    m4x4 *FinalTransforms; // результат преобразований над костями при анимировании (передаётся в шейдер)
};

struct loaded_model
{
    // v3 WorldPos;
    
    string Name;
    u32 MeshesCount;
    single_mesh *Meshes;
    
    u32 TestNormalMap;
};

struct animator // таймер аниимации модели
{
    r32 Timer;
};

internal string ReadStringFromFile(memory_arena *WorldArena, FILE *In);
// internal u32 LoadTexture(string *FileName);

// 3d-model
internal loaded_model *LoadModel(memory_arena *WorldArena, char *FileName);
internal void GetBoneTransforms(single_mesh *Mesh, u32 AnimIndex, r32 TimeInSeconds);

// terrain
#define TMapW 100
#define TMapH 100
internal loaded_model *CreateTerrainModel(memory_arena *WorldArena);

// grass
internal loaded_model *CreateGrassModel(memory_arena *WorldArena);

// other
internal loaded_model *CreateTexturedSquareModel(memory_arena *WorldArena, char *TextureName);

// TODO(ezexff): Replace code above with new asset system code

struct bitmap_id
{
    u32 Value;
};

struct sound_id
{
    u32 Value;
};

struct loaded_bitmap
{
    v2 AlignPercentage;
    real32 WidthOverHeight;
    
    int32 Width;
    int32 Height;
    int32 Pitch;
    void *Memory;
};

struct loaded_sound
{
    uint32 SampleCount; // NOTE(casey): This is the sample count divided by 8
    uint32 ChannelCount;
    int16 *Samples[2];
};

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
    AssetState_Locked,
};

struct asset_slot
{
    asset_state State;
    union
    {
        loaded_bitmap *Bitmap;
        loaded_sound *Sound;
    };
};

struct asset
{
    eab_asset EAB;
    u32 FileIndex;
};

struct asset_type
{
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct asset_file
{
    platform_file_handle *Handle;
    
    // TODO(casey): If we ever do thread stacks, AssetTypeArray
    // doesn't actually need to be kept here probably.
    eab_header Header;
    eab_asset_type *AssetTypeArray;
    
    u32 TagBase;
};

struct game_assets
{
    // TODO(casey): Not thrilled about this back-pointer
    struct transient_state *TranState;
    memory_arena Arena;
    
    //r32 TagRange[Tag_Count];
    
    u32 FileCount;
    asset_file *Files;
    
    u32 TagCount;
    eab_tag *Tags;
    
    u32 AssetCount;
    asset *Assets;
    asset_slot *Slots;
    
    asset_type AssetTypes[Asset_Count];
};