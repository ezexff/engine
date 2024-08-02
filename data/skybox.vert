#version 330 core

layout (location = 0) in vec3 Position;

out vec3 TexCoords0;

uniform mat4 Proj;
uniform mat4 View;
uniform mat4 Model;

void main()
{
    TexCoords0 = Position;
    
    vec4 LocalPosition = vec4(Position, 1.0);
    vec4 WorldPosition = Proj * View * Model * LocalPosition;
    gl_Position = WorldPosition.xyww;
    //gl_Position = WorldPosition;
}