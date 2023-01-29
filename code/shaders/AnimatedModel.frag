#version 330

const int MAX_POINT_LIGHTS = 1;
const int MAX_SPOT_LIGHTS = 1;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 Tangent0;
in vec3 LocalPos0; // TODO(me): переименовать
flat in ivec4 BoneIDs0;
in vec4 Weights0;

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
    vec3 LocalPos; // TODO(me): переименовать
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

uniform DirectionalLight gDirectionalLight;        // completed
uniform int gNumPointLights;                       // completed
uniform PointLight gPointLights[MAX_POINT_LIGHTS]; // completed
uniform int gNumSpotLights;
uniform SpotLight gSpotLights[MAX_SPOT_LIGHTS];
uniform Material gMaterial;                 // completed
uniform sampler2D gSampler;                 // completed
uniform sampler2D gSamplerSpecularExponent; // completed
// uniform vec3 gCameraLocalPos;               // completed
uniform vec3 gCameraWorldPos; // completed
uniform bool gWithTexture;    // new
uniform sampler2D gNormalMap; // bind the normal map before the draw

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)
{
    vec4 AmbientColor = vec4(Light.Color, 1.0f) * Light.AmbientIntensity * vec4(gMaterial.AmbientColor, 1.0f);

    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if(DiffuseFactor > 0)
    {
        DiffuseColor =
            vec4(Light.Color, 1.0f) * Light.DiffuseIntensity * vec4(gMaterial.DiffuseColor, 1.0f) * DiffuseFactor;

        vec3 PixelToCamera = normalize(gCameraWorldPos - LocalPos0);
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

    return (AmbientColor + DiffuseColor + SpecularColor);
}

vec4 CalcDirectionalLight(vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal);
}

vec4 CalcPointLight(PointLight l, vec3 Normal)
{
    vec3 LightDirection = LocalPos0 - l.LocalPos;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 Color = CalcLightInternal(l.Base, LightDirection, Normal);
    float Attenuation = l.Atten.Constant + l.Atten.Linear * Distance + l.Atten.Exp * Distance * Distance;

    return Color / Attenuation;
}

vec4 CalcSpotLight(SpotLight l, vec3 Normal)
{
    vec3 LightToPixel = normalize(LocalPos0 - l.Base.LocalPos);
    float SpotFactor = dot(LightToPixel, l.Direction);

    if(SpotFactor > l.Cutoff)
    {
        vec4 Color = CalcPointLight(l.Base, Normal);
        float SpotLightIntensity = (1.0 - (1.0 - SpotFactor) / (1.0 - l.Cutoff));
        return Color * SpotLightIntensity;
    }
    else
    {
        return vec4(0, 0, 0, 0);
    }
}

vec3 CalcBumpedNormal()
{
    vec3 Normal = normalize(Normal0);
    vec3 Tangent = normalize(Tangent0);
    Tangent = normalize(Tangent - dot(Tangent, Normal) * Normal);
    vec3 Bitangent = cross(Tangent, Normal);
    vec3 BumpMapNormal = texture(gNormalMap, TexCoord0).xyz;
    BumpMapNormal = 2.0 * BumpMapNormal - vec3(1.0, 1.0, 1.0);
    vec3 NewNormal;
    mat3 TBN = mat3(Tangent, Bitangent, Normal);
    NewNormal = TBN * BumpMapNormal;
    NewNormal = normalize(NewNormal);
    return NewNormal;
}

void main()
{
    vec3 Normal = normalize(Normal0);
    // vec3 Normal = CalcBumpedNormal();
    vec4 TotalLight = CalcDirectionalLight(Normal);

    for(int i = 0; i < gNumPointLights; i++)
    {
        TotalLight += CalcPointLight(gPointLights[i], Normal);
    }

    for(int i = 0; i < gNumSpotLights; i++)
    {
        TotalLight += CalcSpotLight(gSpotLights[i], Normal);
    }

    FragColor = TotalLight;

    if(gWithTexture)
    {
        FragColor = texture2D(gSampler, TexCoord0.xy) * TotalLight;
        //FragColor = texture2D(gSampler, TexCoord0.xy);
    }

    //FragColor = texture2D(gSampler, TexCoord0.xy);
}