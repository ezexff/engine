#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

// layout(location = 3) in vec3 Tangent;
// layout(location = 4) in vec3 Bitangent;

layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

// layout(location = 5) in vec3 TestOffset;
layout(location = 5) in mat4 MatModelInstance;

out vec2 TexCoord0;
out vec3 Normal0;
// out vec3 Tangent0;
out vec3 WorldPos0;
flat out ivec4 BoneIDs0;
out vec4 Weights0;
// shadows test
out vec4 FragPosLightSpace;

// uniform mat4 gWVP;
// параметры преобразований
uniform mat4 MatProj;  // матрица проекции
uniform mat4 MatView;  // матрица вида
uniform mat4 MatModel; // матрица модели

uniform mat4 MatProjShadows;
uniform mat4 MatViewShadows;

uniform bool WithAnimations;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform bool WithOffset;
// const int MAX_OFFSETS = 200;
// uniform vec3 Offsets[MAX_OFFSETS];

uniform vec4 CutPlane;

void main()
{
    vec4 PosL = vec4(Position, 1.0);

    if(WithAnimations)
    {
        mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
        BoneTransform += gBones[BoneIDs[1]] * Weights[1];
        BoneTransform += gBones[BoneIDs[2]] * Weights[2];
        BoneTransform += gBones[BoneIDs[3]] * Weights[3];

        PosL = BoneTransform * vec4(Position, 1.0);

        BoneIDs0 = BoneIDs;
        Weights0 = Weights;
    }

    if(WithOffset)
    {
        gl_Position = MatProj * MatView * MatModelInstance * vec4(Position, 1.0);
        Normal0 = (MatModelInstance * vec4(Normal, 0.0)).xyz;
        WorldPos0 = (MatModelInstance * PosL).xyz;
        FragPosLightSpace = MatProjShadows * MatViewShadows * MatModelInstance * vec4(Position, 1.0);
    }
    else
    {
        gl_Position = MatProj * MatView * MatModel * PosL;
        // Normal0 = Normal;
        Normal0 = (MatModel * vec4(Normal, 0.0)).xyz; // преобразование нормали из локальной в мировую систему координат
        // Tangent0 = (MatModel * vec4(Tangent, 0.0)).xyz;
        WorldPos0 = (MatModel * PosL).xyz;
        FragPosLightSpace = MatProjShadows * MatViewShadows * MatModel * PosL;
    }

    gl_ClipDistance[0] = dot(vec4(WorldPos0, 1.0), CutPlane);

    TexCoord0 = TexCoord;
}