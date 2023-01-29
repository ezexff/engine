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
    - TODO(me): добавить 3д модельку кубика в ассеты
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

    // Список мешей для рендеринга без инстансинга
    u32 SingleVerticesCountSum;
    u32 SingleMeshCount;
    single_mesh *SingleMeshes[256];
    v3 *SinglePositions[256];
    r32 *SingleScales[256];
    v3 *SingleRotations[256];
    r32 *SingleAngles[256];
    u32 *SingleInstancingCounters[256];

    // Список моделей для рендеринга без инстансинга
    //u32 SingleModelCount; // TODO(me): удалить

    //loaded_model *SingleModel[256];
    // u32 MeshesCount;
    // single_mesh *Meshes;
    u32 SingleVerticesCount[256];

    u32 SingleVAO;
    u32 SinglePosVBO;
    u32 SingleTexCoordsVBO;
    u32 SingleNormalsVBO;
    u32 SingleIndicesVBO;

    directional_light DirLight;
    u32 PointLightsCount;
    point_light *PointLights;
    u32 SpotLightsCount;
    spot_light *SpotLights;
};