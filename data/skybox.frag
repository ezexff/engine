#version 330 core

layout(location = 0) out vec4 FragColor;

in vec3 TexCoords0;

uniform samplerCube SkyboxTexture;

void main()
{    
    FragColor = texture(SkyboxTexture, TexCoords0);
}