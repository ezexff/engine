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
*/

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

struct render
{
    u32 ShaderProgram;
    u32 Shaders[10];

    // Single Static Meshes for rendering
#define SINGLE_STATIC_MESHES_MAX 10
    u32 SStMeshesCount;
    single_mesh *SStMeshes[SINGLE_STATIC_MESHES_MAX];
    v3 *SStPositions[SINGLE_STATIC_MESHES_MAX];
    r32 *SStScales[SINGLE_STATIC_MESHES_MAX];
    r32 *SStAngles[SINGLE_STATIC_MESHES_MAX];
    v3 *SStRotations[SINGLE_STATIC_MESHES_MAX];
    u32 *SStInstancingCounters[SINGLE_STATIC_MESHES_MAX];
    u32 SstVerticesCount[256];
    u32 SStVerticesCountSum;

    // Single Animated Meshes for rendering

    // Список моделей для рендеринга без инстансинга
    // u32 SingleModelCount; // TODO(me): удалить

    // loaded_model *SingleModel[256];
    // u32 MeshesCount;
    // single_mesh *Meshes;

    u32 SStVAO;
    u32 SStPosVBO;
    u32 SStTexCoordsVBO;
    u32 SStNormalsVBO;
    u32 SStIndicesVBO;
    // u32 SStBoneIDsVBO;
    // u32 SStWeightsVBO;

    directional_light DirLight;
    u32 PointLightsCount;
    point_light *PointLights;
    u32 SpotLightsCount;
    spot_light *SpotLights;

    animator Animator;
};