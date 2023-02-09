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
    string TextureName;
    u32 Texture;
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
    //v3 WorldPos;

    string Name;
    u32 MeshesCount;
    single_mesh *Meshes;

    u32 TestNormalMap;
};

struct animator // таймер аниимации модели
{
    r32 Timer;
};
