#version 330

// in vec2 TexCoord0;
in vec4 ClipSpace;

out vec4 FragColor;

uniform sampler2D ReflectionTexture;
uniform sampler2D RefractionTexture;

void main(void)
{
    vec2 NDC = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;
    vec2 RefractTexCoord = vec2(NDC.x, NDC.y);
    vec2 ReflectTexCoord = vec2(NDC.x, -NDC.y);

    vec4 ReflectColor = texture(ReflectionTexture, ReflectTexCoord);
    vec4 RefractColor = texture(RefractionTexture, RefractTexCoord);

    // FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    FragColor = mix(ReflectColor, RefractColor, 0.5);
}