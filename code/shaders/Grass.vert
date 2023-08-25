#version 330

layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
layout(location = 3) in mat4 MatModelInstance;

out vec2 TexCoord0;
out vec3 Normal0;
out vec3 WorldPos0;
out vec4 FragPosLightSpace; // shadows test

uniform mat4 MatProj;
uniform mat4 MatView;

uniform mat4 MatProjShadows;
uniform mat4 MatViewShadows;

//const vec4 Plane = vec4(0, -1, 0, 15);

void main()
{
    vec4 WorldPosition = MatModelInstance * vec4(Position, 1.0);
    gl_Position = MatProj * MatView * WorldPosition;
    Normal0 = (MatModelInstance * vec4(Normal, 0.0)).xyz;
    WorldPos0 = WorldPosition.xyz;
    FragPosLightSpace = MatProjShadows * MatViewShadows * WorldPosition;
    TexCoord0 = TexCoord;

    //gl_ClipDistance[0] = dot(vec4(WorldPos0, 1.0), Plane);
}