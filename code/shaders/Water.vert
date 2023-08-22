#version 330

layout(location = 0) in vec3 Position;

out vec4 ClipSpace;
out vec2 TextureCoords;
out vec3 ToCameraVector;
out vec3 FromLightVector;

uniform mat4 MatProj;
uniform mat4 MatView;
uniform mat4 MatModel;
uniform vec3 CameraPosition;
uniform vec3 LightPosition;
uniform float Tiling;

void main(void)
{
    vec4 WorldPosition = MatModel * vec4(Position.x, Position.y, 0.0, 1.0);
    ClipSpace = MatProj * MatView * WorldPosition;
    gl_Position = ClipSpace;
    TextureCoords = vec2(Position.x / 2.0 + 0.5, Position.y / 2.0 + 0.5) * Tiling;
    ToCameraVector = CameraPosition - WorldPosition.xyz; // PixelToCamera
    FromLightVector = WorldPosition.xyz - LightPosition; // LightDirection
}