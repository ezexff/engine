#version 330
layout(location = 0) in vec3 iPosition;
//layout(location = 3) in ivec4 BoneIDs;
//layout(location = 4) in vec4 Weights;
//layout(location = 5) in mat4 MatModelInstance;

// uniform mat4 lightSpaceMatrix;
// uniform mat4 model;

// параметры преобразований
//uniform mat4 Proj;  // матрица проекции
//uniform mat4 View;  // матрица вида
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel; // матрица модели

//uniform vec3 uOffsetP;
// uniform vec3 ViewPosition; // позиция камеры

//uniform bool WithAnimations;

//const int MAX_BONES = 100;
//uniform mat4 gBones[MAX_BONES];

//uniform bool WithOffset;

void main()
{
    vec4 LocalP = vec4(iPosition, 1.0);
    
    gl_Position = uProj * uView * uModel * LocalP;
    
    // gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
    // vec4 PosL = vec4(Position, 1.0);

    /*if(WithAnimations)
    {
        mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
        BoneTransform += gBones[BoneIDs[1]] * Weights[1];
        BoneTransform += gBones[BoneIDs[2]] * Weights[2];
        BoneTransform += gBones[BoneIDs[3]] * Weights[3];

        PosL = BoneTransform * vec4(Position, 1.0);
    }

    if(WithOffset)
    {
        // gl_Position = MatProj * MatView * MatModelInstance * vec4(Position, 1.0);
        gl_Position = MatProjShadows * MatViewShadows * MatModelInstance * vec4(Position, 1.0);
    }
    else
    {
        // gl_Position = MatProj * MatView * MatModel * PosL;
        gl_Position = MatProjShadows * MatViewShadows * MatModel * PosL;
    }*/
}