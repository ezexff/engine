/*
    Система рендеринга:
    - в постоянной памяти движка хранятся сущности, например, объекты окружения
    - у каджой сущности есть указательнь на 3d-модель
    - рендерер имеет 2 очереди: с инстансингом и без
    - у каждой сущности есть переменная InstancingCount
    - если эта переменная равна нулю, то модель добавляется в очередь без инстансинга
    - если эта переменная больше нуля, то модель добавляется в очередь с инстансингом
    - без инстансинга: один VBO и на каждую модель по вызову отрисовки

    - TODO(me): каждый меш как отдельная модель или в рендерер помещать только меши?
    - TODO(me): менять позиуию источников света стрелочками через ImGui?
    - TODO(me): рендерить кубик в позиции источника света?
    - TODO(me): переместить после тестов таймер анимаций из render в envobjects
    - TODO(me): 2 сингловых VBO: 1 для статичных моделей и 2 для анимированных?
    - TODO(me): AddEnvObjectsToRender
*/

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
    //
    // NOTE(me): Shaders
    //
    u32 ShaderProgram;
    u32 Shaders[10];

    //
    // NOTE(me): Single Static Meshes list for rendering
    //
#define SINGLE_STATIC_MESHES_MAX 256

    // меши
    u32 SStMeshesCount;
    single_mesh *SStMeshes[SINGLE_STATIC_MESHES_MAX];

    // атрибуты модели
    // v3 *SStPositions[SINGLE_STATIC_MESHES_MAX];
    // r32 *SStScales[SINGLE_STATIC_MESHES_MAX];
    // r32 *SStAngles[SINGLE_STATIC_MESHES_MAX];
    // v3 *SStRotations[SINGLE_STATIC_MESHES_MAX];
    m4x4 *SStTransformMatrices[SINGLE_STATIC_MESHES_MAX];
    // u32 *SStInstancingCounters[SINGLE_STATIC_MESHES_MAX];

    // данные для отрисовки
    u32 SStVerticesCountSum;
    u32 SStIndicesCountSum;
    // u32 SstIndicesCount[SINGLE_STATIC_MESHES_MAX];

    u32 SStVAO;
    u32 SStVBO;
    u32 SStEBO;

    //
    // NOTE(me): Single Animated Meshes list for rendering
    //
#define SINGLE_ANIMATED_MESHES_MAX 256

    // меши
    u32 SAnMeshesCount;
    single_mesh *SAnMeshes[SINGLE_ANIMATED_MESHES_MAX];

    // атрибуты модели
    // v3 *SAnPositions[SINGLE_ANIMATED_MESHES_MAX];
    // r32 *SAnScales[SINGLE_ANIMATED_MESHES_MAX];
    // r32 *SAnAngles[SINGLE_ANIMATED_MESHES_MAX];
    // v3 *SAnRotations[SINGLE_ANIMATED_MESHES_MAX];
    m4x4 *SAnTransformMatrices[SINGLE_ANIMATED_MESHES_MAX];
    // u32 *SAnInstancingCounters[SINGLE_ANIMATED_MESHES_MAX];

    // данные для отрисовки
    u32 SAnVerticesCountSum;
    u32 SAnIndicesCountSum;
    // u32 SAnIndicesCount[SINGLE_ANIMATED_MESHES_MAX];

    u32 SAnVAO;
    u32 SAnVBO;
    u32 SAnEBO;

    // TODO(me): for testing single vbo vs multiple vbo's performance
    u32 *TestSAnVAO;
    u32 *TestSAnVBO;
    u32 *TestSAnEBO;

#define MULTIPLE_STATIC_MESHES_MAX 256

    // меши
    u32 MStMeshesCount;
    single_mesh *MStMeshes[MULTIPLE_STATIC_MESHES_MAX];

    // атрибуты модели
    // v3 *MStPositions[MULTIPLE_STATIC_MESHES_MAX];
    // r32 *MStScales[MULTIPLE_STATIC_MESHES_MAX];
    // r32 *MStAngles[MULTIPLE_STATIC_MESHES_MAX];
    // v3 *MStRotations[MULTIPLE_STATIC_MESHES_MAX];
    u32 *MStInstancingCounters[MULTIPLE_STATIC_MESHES_MAX];
    // v3 *MStInstancingTranslations[MULTIPLE_STATIC_MESHES_MAX];
    m4x4 *MStInstancingTransformMatrices[MULTIPLE_STATIC_MESHES_MAX];

    // данные для отрисовки
    u32 MStVerticesCountSum;
    u32 MStIndicesCountSum;
    u32 MStInstancesCountSum;
    // u32 SstIndicesCount[SINGLE_STATIC_MESHES_MAX];

    u32 MStVAO;
    u32 MStVBO;
    u32 MStEBO;

    // u32 MaxInstancingCount;
    // offset_var_name *InstancingVarNames;

    //
    // NOTE(me): Light
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
    animator Animator;
};