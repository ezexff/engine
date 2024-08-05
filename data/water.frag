#version 330

layout(location = 0) out vec4 FragColor;

in vec4 vClipSpace;
in vec2 vTexCoords;
in vec3 vToCameraVector;
in vec3 vFromSunVector;

uniform float uMoveFactor;
uniform float uWaveStrength;
uniform float uShineDamper;
uniform float uReflectivity;

uniform vec3 uLightColor;

uniform sampler2D uReflectionColorTexture;
uniform sampler2D uRefractionColorTexture;
uniform sampler2D uDUDVTexture;
uniform sampler2D uNormalMapTexture;
uniform sampler2D uRefractionDepthTexture;

void main(void)
{
    vec2 NDC = (vClipSpace.xy / vClipSpace.w) / 2.0 + 0.5;
    vec2 RefractTexCoord = vec2(NDC.x, NDC.y);
    vec2 ReflectTexCoord = vec2(NDC.x, -NDC.y);

    float Near = 0.1;
    float Far = 1000.0;
    float Depth = texture(uRefractionDepthTexture, RefractTexCoord).r;
    float FloorDistance = 2.0 * Near * Far / (Far + Near - (2.0 * Depth - 1.0) * (Far - Near));

    Depth = gl_FragCoord.z;
    float WaterDistance = 2.0 * Near * Far / (Far + Near - (2.0 * Depth - 1.0) * (Far - Near));
    float WaterDepth = FloorDistance - WaterDistance;

    vec2 DistortedTexCoords = texture(uDUDVTexture, vec2(vTexCoords.x + uMoveFactor, vTexCoords.y)).rg * 0.1;
    DistortedTexCoords = vTexCoords + vec2(DistortedTexCoords.x, DistortedTexCoords.y + uMoveFactor);
    vec2 TotalDistortion = (texture(uDUDVTexture, DistortedTexCoords).rg * 2.0 - 1.0) * uWaveStrength;
    //vec2 TotalDistortion =
    //    (texture(uDUDVTexture, DistortedTexCoords).rg * 2.0 - 1.0) * uWaveStrength * clamp(WaterDepth / 20.0, 0.0, 1.0);

    RefractTexCoord += TotalDistortion;
    RefractTexCoord = clamp(RefractTexCoord, 0.001, 0.999);

    ReflectTexCoord += TotalDistortion;
    ReflectTexCoord.x = clamp(ReflectTexCoord.x, 0.001, 0.999);
    ReflectTexCoord.y = clamp(ReflectTexCoord.y, -0.999, -0.001);

    vec4 ReflectColor = texture(uReflectionColorTexture, ReflectTexCoord);
    vec4 RefractColor = texture(uRefractionColorTexture, RefractTexCoord);

    vec4 uNormalMapTextureColor = texture(uNormalMapTexture, DistortedTexCoords);
    vec3 Normal = vec3(uNormalMapTextureColor.r * 2.0 - 1.0, uNormalMapTextureColor.b, uNormalMapTextureColor.g * 2.0 - 1.0);
    //vec3 Normal = vec3(uNormalMapTextureColor.r * 2.0 - 1.0, uNormalMapTextureColor.b * 3.0, uNormalMapTextureColor.g * 2.0 - 1.0);
    Normal = normalize(Normal);

    vec3 ViewVector = normalize(vToCameraVector);
    float RefractiveFactor = dot(ViewVector, vec3(0.0, 0.0, 1.0));
    //float RefractiveFactor = dot(ViewVector, Normal);
    RefractiveFactor = pow(RefractiveFactor, 0.5);

    vec3 ReflectedLight = reflect(normalize(vFromSunVector), Normal);
    float Specular = max(dot(ReflectedLight, ViewVector), 0.0);
    Specular = pow(Specular, uShineDamper);
    vec3 SpecularHighlights = uLightColor * Specular * uReflectivity;
    //vec3 SpecularHighlights = uLightColor * Specular * uReflectivity * clamp(WaterDepth / 5.0, 0.0, 1.0);

    FragColor = mix(ReflectColor, RefractColor, RefractiveFactor);
    FragColor = mix(FragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(SpecularHighlights, 0.0);
    //FragColor.a = clamp(WaterDepth / 5.0, 0.0, 1.0);
    //  FragColor = uNormalMapTextureColor;
    //  FragColor = vec4(WaterDepth / 50.0);
}