#version 330 core
layout (location = 0) in vec2 Position;


flat out int Index0;
out vec2 TexCoords0;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 ModelArray[250];

void main()
{
    TexCoords0 = Position.xy;
    Index0 = gl_InstanceID;
    // TexCoords0.y = 1.0f - TexCoords0.y;
    
    vec4 LocalPosition = vec4(Position, 0.0, 1.0);
    gl_Position = uProj * uView * ModelArray[gl_InstanceID] * LocalPosition;
}