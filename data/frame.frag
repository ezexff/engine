#version 330

layout(location = 0) out vec4 FragColor;

in vec2 TexCoords0;

uniform sampler2D ColorTexture;
uniform int EffectID;

#define EFFECT_ABBERATION 0
#define EFFECT_BLUR 1
#define EFFECT_EMBOSS 2
#define EFFECT_GRAYSCALE 3
#define EFFECT_INVERSE 4
#define EFFECT_SEPIA 5

//~ NOTE(ezexff): Aberration
vec3 Aberration(in vec2 TexCoord)
{
    vec3 Result = vec3(textureOffset(ColorTexture, TexCoord, ivec2(2,2)).r,
                       texture(ColorTexture, TexCoord).g,
                       textureOffset(ColorTexture, TexCoord, ivec2(4,4)).b);
    return(Result);
}

//~ NOTE(ezexff): Blur
#define BLUR_KERNEL_SIZE 9
const float BlurKernel[BLUR_KERNEL_SIZE] = float[](0.0625, 0.1250, 0.0625,
                                                   0.1250, 0.2500, 0.1250,
                                                   0.0625, 0.1250, 0.0625);
const vec2 BlurOffset[BLUR_KERNEL_SIZE] = vec2[](vec2(-1.0,-1.0), vec2( 0.0,-1.0), vec2( 1.0,-1.0),
                                                 vec2(-1.0, 0.0), vec2( 0.0, 0.0), vec2( 1.0, 0.0),
                                                 vec2(-1.0, 1.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0));
vec3 Blur(in vec2 TexCoord)
{
    vec4 Result = vec4(0.0);
    vec2 pstep = vec2(2.0) / vec2(textureSize(ColorTexture, 0));
    
	for(int i = 0; i < BLUR_KERNEL_SIZE; ++i)
    {
        Result += texture(ColorTexture, TexCoord + BlurOffset[i] * pstep) * BlurKernel[i];
    }
    
	return vec3(Result);
}

//~ NOTE(ezexff): Emboss
#define EMBOSS_KERNEL_SIZE 9
const float EmbossKernel[EMBOSS_KERNEL_SIZE] = float[](2.0,  0.0,  0.0,
                                                       0.0, -1.0,  0.0,
                                                       0.0,  0.0, -1.0);

const vec2 EmbossOffset[EMBOSS_KERNEL_SIZE] = vec2[](vec2(-1.0,-1.0), vec2( 0.0,-1.0), vec2( 1.0,-1.0),
                                                     vec2(-1.0, 0.0), vec2( 0.0, 0.0), vec2( 1.0, 0.0),
                                                     vec2(-1.0, 1.0), vec2( 0.0, 1.0), vec2( 1.0, 1.0));

const vec3 EmbossFactor = vec3(0.27, 0.67, 0.06);

vec3 Emboss(in vec2 TexCoord)
{
	vec4 Result = vec4(0.5);
	vec2 pstep = vec2(1.0) / vec2(textureSize(ColorTexture, 0));
    
	for (int i = 0; i < EMBOSS_KERNEL_SIZE; ++i)
    {
        Result += texture(ColorTexture, TexCoord + EmbossOffset[i] * pstep) * EmbossKernel[i];
    }
    
	return vec3(dot(EmbossFactor, vec3(Result)));
}

//~ NOTE(ezexff): Grayscale
const vec3 GrayscaleFactor = vec3(0.27, 0.67, 0.06);
vec3 Grayscale(in vec2 TexCoord)
{
    vec3 Result = vec3(dot(GrayscaleFactor, texture(ColorTexture, TexCoord).rgb));
    return(Result);
}

//~ NOTE(ezexff): Inverse
vec3 Inverse(in vec2 TexCoord)
{
    vec3 Result = vec3(1.0) - texture(ColorTexture, TexCoord).rgb;
    return(Result);
}

//~ NOTE(ezexff): Sepia
const vec3 SepiaFactor     = vec3(0.27, 0.67, 0.06);
const vec3 SepiaDarkColor  = vec3(0.2, 0.05, 0.0);
const vec3 SepiaLightColor = vec3(1.0,  0.9, 0.5);

vec3 Sepia(in vec2 TexCoord)
{
    vec3 Result = mix(SepiaDarkColor, SepiaLightColor, dot(SepiaFactor, texture(ColorTexture, TexCoord).rgb));
	return(Result);
}

void main()
{
    vec3 Texel;
    
    switch(EffectID)
    {
        case EFFECT_ABBERATION:
        {
            Texel = Aberration(TexCoords0);
        } break;
        
        case EFFECT_BLUR:
        {
            Texel = Blur(TexCoords0);
        } break;
        
        case EFFECT_EMBOSS:
        {
            Texel = Emboss(TexCoords0);
        } break;
        
        case EFFECT_GRAYSCALE:
        {
            Texel = Grayscale(TexCoords0);
        } break;
        
        case EFFECT_INVERSE:
        {
            Texel = Inverse(TexCoords0);
        } break;
        
        case EFFECT_SEPIA:
        {
            Texel = Sepia(TexCoords0);
        } break;
        
        default:
        {
            //Texel = TexCoords0.x < 0.5 ? Grayscale(TexCoords0) : Aberration(TexCoords0);
            Texel = texture(ColorTexture, TexCoords0).rgb;
        } break;
    };
    
    // задаем цвет фрагмента
    FragColor = vec4(Texel, 1.0);
    
    //FragColor = texture2D(ColorTexture, TexCoords0.xy);
}