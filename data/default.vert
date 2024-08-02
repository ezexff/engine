#version 330

layout(location = 0) in vec3 iPosition;
layout(location = 1) in vec3 iNormal;
layout(location = 2) in vec2 iTexCoords;

// layout(location = 3) in vec3 Tangent;
// layout(location = 4) in vec3 Bitangent;
/*layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;
// layout(location = 5) in vec3 TestOffset;
layout(location = 5) in mat4 MatModelInstance;*/

uniform mat4 uProj;
uniform mat4 uView; 
uniform mat4 uModel;

uniform mat4 uProjShadowMap;
uniform mat4 uViewShadowMap;

/*uniform bool WithAnimations;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

uniform bool WithOffset;*/
// const int MAX_OFFSETS = 200;
// uniform vec3 Offsets[MAX_OFFSETS];

/*uniform vec4 CutPlane;*/

out vec3 vWorldP;
out vec3 vNormal;
out vec2 vTexCoords;
out vec4 vShadowMapCameraWorldP;
//out vec4 FragPosLightSpace0; // shadows test
// out vec3 Tangent0;
//flat out ivec4 BoneIDs0;
//out vec4 Weights0;

void main()
{
    vec4 LocalP = vec4(iPosition, 1.0);
    
    gl_Position = uProj * uView * uModel * LocalP;
    
    // NOTE(ezexff): To frag
    vWorldP = (uModel * LocalP).xyz;
    vNormal = (uModel * vec4(iNormal, 0.0)).xyz;
    vTexCoords = iTexCoords;
    vShadowMapCameraWorldP = uProjShadowMap * uViewShadowMap * uModel * LocalP;
    
    // Tangent0 = (Model * vec4(Tangent, 0.0)).xyz;
    /*if(WithAnimations)
    /*{
        mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
        BoneTransform += gBones[BoneIDs[1]] * Weights[1];
        BoneTransform += gBones[BoneIDs[2]] * Weights[2];
        BoneTransform += gBones[BoneIDs[3]] * Weights[3];

        LocalPosition = BoneTransform * vec4(Position, 1.0);

        BoneIDs0 = BoneIDs;
        Weights0 = Weights;
    }

    if(WithOffset)
    {
        gl_Position = MatProj * MatView * MatModelInstance * LocalPosition;
        Normal0 = (MatModelInstance * vec4(Normal, 0.0)).xyz;
        WorldPos0 = (MatModelInstance * LocalPosition).xyz;
        FragPosLightSpace = MatProjShadows * MatViewShadows * MatModelInstance * LocalPosition;
    }
    else*/
    {
        //
    }

    //gl_ClipDistance[0] = dot(vec4(WorldPos0, 1.0), CutPlane);
}