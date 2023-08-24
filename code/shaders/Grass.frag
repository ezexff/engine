#version 330

const int MAX_POINT_LIGHTS = 1;
const int MAX_SPOT_LIGHTS = 1;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;
in vec4 FragPosLightSpace;

out vec4 FragColor;

struct BaseLight
{
    vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
};

struct DirectionalLight
{
    BaseLight Base;
    vec3 Direction;
};

struct Attenuation
{
    float Constant;
    float Linear;
    float Exp;
};

struct PointLight
{
    BaseLight Base;
    vec3 WorldPos;
    Attenuation Atten;
};

struct SpotLight
{
    PointLight Base;
    vec3 Direction;
    float Cutoff;
};

struct Material
{
    vec3 AmbientColor;
    vec3 DiffuseColor;
    vec3 SpecularColor;
};

uniform DirectionalLight gDirectionalLight;
uniform int gNumPointLights;
uniform PointLight gPointLights[MAX_POINT_LIGHTS];
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];
uniform Material gMaterial;
uniform sampler2D gSampler;
uniform sampler2D gSamplerSpecularExponent;
uniform vec3 gCameraWorldPos;
uniform bool gWithTexture;
uniform sampler2D ShadowMap;
uniform float Bias;

vec4 CalcLightInternalWS(BaseLight Light, vec3 LightDirection, vec3 Normal, float Shadow)
{
    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity * vec4(gMaterial.AmbientColor, 1.0f);

    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if(DiffuseFactor > 0)
    {
        DiffuseColor =
            vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * vec4(gMaterial.DiffuseColor, 1.0f) * DiffuseFactor;

        vec3 PixelToCamera = normalize(gCameraWorldPos - WorldPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(PixelToCamera, LightReflect);
        if(SpecularFactor > 0)
        {
            float SpecularExponent = texture2D(gSamplerSpecularExponent, TexCoord0).r * 255.0;
            SpecularFactor = pow(SpecularFactor, SpecularExponent);
            SpecularColor = vec4(Light.Color, 1.0f) *
                            Light.DiffuseIntensity * // using the diffuse intensity for diffuse/specular
                            vec4(gMaterial.SpecularColor, 1.0f) * SpecularFactor;
        }
    }

    return (AmbientColor + (1.0 - Shadow) * (DiffuseColor + SpecularColor));
}

vec4 CalcDirectionalLightWS(vec3 Normal, float Shadow) // with shadow
{
    return CalcLightInternalWS(gDirectionalLight.Base, gDirectionalLight.Direction, Normal, Shadow);
}

vec4 CalcPointLight(PointLight l, vec3 Normal, float Shadow)
{
    vec3 LightDirection = WorldPos0 - l.WorldPos;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 Color = CalcLightInternalWS(l.Base, LightDirection, Normal, Shadow);
    float Attenuation = l.Atten.Constant + l.Atten.Linear * Distance + l.Atten.Exp * Distance * Distance;

    return Color / Attenuation;
}

vec4 CalcSpotLight(SpotLight l, vec3 Normal, float Shadow)
{
    vec3 LightToPixel = normalize(WorldPos0 - l.Base.WorldPos);
    float SpotFactor = dot(LightToPixel, l.Direction);

    if(SpotFactor > l.Cutoff)
    {
        vec4 Color = CalcPointLight(l.Base, Normal, Shadow);
        float SpotLightIntensity = (1.0 - (1.0 - SpotFactor) / (1.0 - l.Cutoff));
        return Color * SpotLightIntensity;
    }
    else
    {
        return vec4(0, 0, 0, 0);
    }
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 Normal)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(ShadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    //vec3 normal = normalize(Normal0);
    //vec3 lightDir = normalize(gDirectionalLight.Direction - WorldPos0);
    //float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(ShadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(ShadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth - Bias > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    vec3 Normal = normalize(Normal0);

    float Shadow = ShadowCalculation(FragPosLightSpace, Normal);

    vec4 TotalLight = CalcDirectionalLightWS(Normal, Shadow);

    for(int i = 0; i < gNumPointLights; i++)
    {
        TotalLight += CalcPointLight(gPointLights[i], Normal, Shadow);
    }

    for(int i = 0; i < gNumSpotLights; i++)
    {
        TotalLight += CalcSpotLight(gSpotLights[i], Normal, Shadow);
    }

    FragColor = TotalLight;

    if(gWithTexture)
    {
        FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;
    }
}