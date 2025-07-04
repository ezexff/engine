#version 330 core

layout(location = 0) out vec4 FragColor;

flat in int Index0;
in vec2 TexCoords0;

uniform sampler2DArray GlyphTexture;
uniform int GlyphMapArray[400];
uniform vec4 GlyphColor;

void main()
{    
    // vec4 Glyph = vec4(1.0, 1.0, 1.0, texture(GlyphTexture, vec3(TexCoords0.xy, Index0)).r);
    vec4 Glyph = vec4(1.0, 1.0, 1.0, texture(GlyphTexture, vec3(TexCoords0.xy, GlyphMapArray[Index0])).r);
    // FragColor = Glyph;
    FragColor = GlyphColor * Glyph;
    // FragColor = GlyphColor;
}