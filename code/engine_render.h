struct vertex_static
{
    v3 Position;
    v3 Normal;
    v2 TexCoords;
};

struct vertex_animated
{
    v3 Position;
    v3 Normal;
    v2 TexCoords;
    u32 BoneIDs[4];
    r32 Weights[4];
};

struct base_light
{
    v3 Color;
    r32 AmbientIntensity;
    r32 DiffuseIntensity;
};

struct directional_light
{
    base_light Base;
    v3 WorldDirection;
};

struct attenuation
{
    r32 Constant;
    r32 Linear;
    r32 Exp;
};

struct point_light
{
    base_light Base;
    v3 WorldPosition;
    attenuation Atten;
};

struct spot_light
{
    point_light Base;
    v3 WorldDirection;
    r32 Cutoff;
};

struct point_light_var_names
{
    char *VarNames[7];
};

struct spot_light_var_names
{
    char *VarNames[9];
};

struct offset_var_name
{
    char *VarName;
};

struct render
{
    // Render Scene Width & Height
    s32 DisplayWidth, DisplayHeight;
    r32 AspectRatio;
    r32 FOV;

    //
    // NOTE(me): Shader programs
    //
    u32 DefaultShaderProgram;
    u32 WaterShaderProgram;
    u32 DepthMapShaderProgram;
    u32 GrassShaderProgram;

    // Shader cut plane (for water refl & refr)
    v4 CutPlane;

    //
    // NOTE(me): Water
    //
    r32 WaterWaveSpeed;
    r32 WaterMoveFactor;
    r32 WaterTiling;
    r32 WaterWaveStrength;
    r32 WaterShineDamper;
    r32 WaterReflectivity;

    // Reflection
    s32 ReflWidth, ReflHeight;
    u32 WaterReflFBO;
    u32 WaterReflTexture;
    u32 WaterReflDepthRBO;
    string WaterDUDVTextureName;
    u32 WaterDUDVTexture;
    string WaterNormalMapName;
    u32 WaterNormalMap;

    // Refraction
    s32 RefrWidth, RefrHeight;
    u32 WaterRefrFBO;
    u32 WaterRefrTexture;
    u32 WaterRefrDepthTexture;

    //
    // NOTE(me): Shadows (Depth Map)
    //
    s32 DepthMapWidth, DepthMapHeight;
    u32 DepthMapFBO;
    u32 DepthMap;
    r32 ShadowMapSize;
    r32 NearPlane, FarPlane;
    r32 ShadowCameraPitch, ShadowCameraYaw;
    v3 SunPos;
    r32 Bias;

    //
    // NOTE(me): Environment Objects Rendering System
    //
    u32 EnvObjectsCount; // число моделей объектов окружения в очереди

    // Single Static Meshes list for rendering
#define SINGLE_STATIC_MESHES_MAX 256

    u32 SStMeshesCount;
    single_mesh *SStMeshes[SINGLE_STATIC_MESHES_MAX];

    m4x4 *SStTransformMatrices[SINGLE_STATIC_MESHES_MAX];

    u32 SStVerticesCountSum;
    u32 SStIndicesCountSum;

    u32 SStVAO;
    u32 SStVBO;
    u32 SStEBO;

    // Single Animated Meshes list for rendering
#define SINGLE_ANIMATED_MESHES_MAX 256

    u32 SAnMeshesCount;
    single_mesh *SAnMeshes[SINGLE_ANIMATED_MESHES_MAX];

    m4x4 *SAnTransformMatrices[SINGLE_ANIMATED_MESHES_MAX];

    u32 SAnVerticesCountSum;
    u32 SAnIndicesCountSum;

    u32 SAnVAO;
    u32 SAnVBO;
    u32 SAnEBO;

    // TODO(me): for testing single vbo vs multiple vbo's performance
    u32 *TestSAnVAO;
    u32 *TestSAnVBO;
    u32 *TestSAnEBO;

    // Multiple Static Meshes list for rendering
#define MULTIPLE_STATIC_MESHES_MAX 256

    u32 MStMeshesCount;
    single_mesh *MStMeshes[MULTIPLE_STATIC_MESHES_MAX];

    u32 *MStInstancingCounters[MULTIPLE_STATIC_MESHES_MAX];
    m4x4 *MStInstancingTransformMatrices[MULTIPLE_STATIC_MESHES_MAX];

    u32 MStVerticesCountSum;
    u32 MStIndicesCountSum;
    u32 MStInstancesCountSum;

    u32 MStVAO;
    u32 MStVBO;
    u32 MStEBO;

    //
    // NOTE(me): Grass Objects Rendering System
    //
    u32 GrassObjectsCount; // число моделей объектов окружения в очереди
#define GRASS_MESHES_MAX 256

    u32 GrMeshesCount;
    single_mesh *GrMeshes[GRASS_MESHES_MAX];

    u32 *GrInstancingCounters[GRASS_MESHES_MAX];
    m4x4 *GrInstancingTransformMatrices[GRASS_MESHES_MAX];

    u32 GrVerticesCountSum;
    u32 GrIndicesCountSum;
    u32 GrInstancesCountSum;

    u32 GrVAO;
    u32 GrVBO;
    u32 GrEBO;

    //
    // NOTE(me): Light Sources
    //
    directional_light DirLight;

    u32 PointLightsCount;
    point_light *PointLights;
    point_light_var_names *PLVarNames; // имена переменных для передачи в шейдер

    u32 SpotLightsCount;
    spot_light *SpotLights;
    spot_light_var_names *SLVarNames; // имена переменных для передачи в шейдер

    //
    // NOTE(me): Animations
    //
    animator Animator; // timers

    //
    // NOTE(me): Textures for Debug Elements
    //
    string ClipTextureName;
    u32 ClipTexture;
    string LightgTextureName;
    u32 LightTexture;
};

// shaders
GLuint LoadShader(char *Path, GLuint Type);
u32 LinkShaderProgram(u32 ShaderVert, u32 ShaderFrag);

// environment objects
internal void AddEnvObjectsToRender(render *Render, entity_envobject *EnvObjects[]);
void InitEnvVBOs(memory_arena *WorldArena, render *Render);
internal void RenderEnvVBOs(render *Render, u32 ShaderProg, entity_player *Player);

// grass objects
internal void AddGrassObjectsToRender(render *Render, entity_grassobject *GrassObjects[]);
void InitGrassVBO(memory_arena *WorldArena, render *Render);
internal void RenderGrassVBO(render *Render, entity_player *Player);

// shadows
void InitDepthMapFBO(render *Render);

// water
void InitWaterFBOs(render *Render);
internal void RenderWater(render *Render, entity_player *Player, r32 dtForFrame, r32 WaterZ);

// debug elements
void DrawOrthoTexturedRectangle(render *Render, u32 Texture, r32 TextureWidth, s32 TextureHeight, v2 Offset);
internal void OGLDrawLinesOXYZ(v3 Normal, r32 LineWidth, r32 LineMin, r32 LineMax, r32 Offset);
internal void DrawRectangularParallelepiped(r32 MinX, r32 MinY, r32 MaxX, r32 MaxY, //
                                            r32 MinZ, r32 MaxZ,                     //
                                            v3 Color);
void DrawTexturedRectangle(r32 VRectangle[], u32 Texture, r32 Repeat);

// render vbo's
void RenderScene(render *Render, u32 ShaderProg, entity_player *Player, GLbitfield glClearMask);
void RenderDebugElements(render *Render, entity_player *Player, entity_clip *PlayerClip);