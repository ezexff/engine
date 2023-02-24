#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;

// layout(location = 3) in vec3 Tangent;
// layout(location = 4) in vec3 Bitangent;

layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

out vec2 TexCoord0;
out vec3 Normal0;
// out vec3 Tangent0;
out vec3 LocalPos0; // TODO(me): переименовать
flat out ivec4 BoneIDs0;
out vec4 Weights0;

// uniform mat4 gWVP;
// параметры преобразований
uniform mat4 MatProj;  // матрица проекции
uniform mat4 MatView;  // матрица вида
uniform mat4 MatModel; // матрица модели
// uniform vec3 ViewPosition; // позиция камеры

uniform bool WithAnimations;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform bool WithOffset;
const int MAX_OFFSETS = 200;
uniform vec3 Offsets[MAX_OFFSETS];

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
    }

    if(WithOffset)
    {
        vec3 Offset = Offsets[gl_InstanceID];
        //vec3 Offset = Offsets[0];
        PosL += vec4(Offset, 0.0);
    }

    gl_Position = MatProj * MatView * MatModel * PosL;
    TexCoord0 = TexCoord;
    // Normal0 = Normal;
    Normal0 = (MatModel * vec4(Normal, 0.0)).xyz; // преобразование нормали из локальной в мировую систему координат
    // Tangent0 = (MatModel * vec4(Tangent, 0.0)).xyz;
    // LocalPos0 = Position;
    LocalPos0 = (MatModel * vec4(Position, 0.0)).xyz;
    BoneIDs0 = BoneIDs;
    Weights0 = Weights;
}