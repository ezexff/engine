#version 330

layout(location = 0) in vec3 iPosition;

out vec4 vClipSpace;
out vec2 vTexCoords;
out vec3 vToCameraVector;
out vec3 vFromSunVector;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uModel;

uniform vec3 uCameraP;
uniform vec3 uSunP;
uniform float uTiling;

void main(void)
{
    vec4 WorldP = uModel * vec4(iPosition.x, iPosition.y, 0.0, 1.0);
    vClipSpace = uProj * uView * WorldP;
    gl_Position = vClipSpace;
    vTexCoords = vec2(iPosition.x / 2.0 + 0.5, iPosition.y / 2.0 + 0.5) * uTiling;
    vToCameraVector = uCameraP - WorldP.xyz; // PixelToCamera
    vFromSunVector = WorldP.xyz - uSunP; // LightDirection
}