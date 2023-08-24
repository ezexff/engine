#version 330

in vec4 ClipSpace;
in vec2 TextureCoords;
in vec3 ToCameraVector;
in vec3 FromSunVector;
uniform sampler2D ReflectionTexture;
uniform sampler2D RefractionTexture;
uniform sampler2D DUDVMap;
uniform sampler2D NormalMap;
uniform sampler2D DepthMap;
uniform vec3 LightColor;

uniform float MoveFactor;
uniform float WaveStrength;
uniform float ShineDamper;
uniform float Reflectivity;

out vec4 FragColor;

void main(void)
{
    vec2 NDC = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;
    vec2 RefractTexCoord = vec2(NDC.x, NDC.y);
    vec2 ReflectTexCoord = vec2(NDC.x, -NDC.y);

    float Near = 0.1;
    float Far = 1000.0;
    float Depth = texture(DepthMap, RefractTexCoord).r;
    float FloorDistance = 2.0 * Near * Far / (Far + Near - (2.0 * Depth - 1.0) * (Far - Near));

    Depth = gl_FragCoord.z;
    float WaterDistance = 2.0 * Near * Far / (Far + Near - (2.0 * Depth - 1.0) * (Far - Near));
    float WaterDepth = FloorDistance - WaterDistance;

    vec2 DistortedTexCoords = texture(DUDVMap, vec2(TextureCoords.x + MoveFactor, TextureCoords.y)).rg * 0.1;
    DistortedTexCoords = TextureCoords + vec2(DistortedTexCoords.x, DistortedTexCoords.y + MoveFactor);
    vec2 TotalDistortion = (texture(DUDVMap, DistortedTexCoords).rg * 2.0 - 1.0) * WaveStrength;
    //vec2 TotalDistortion =
    //    (texture(DUDVMap, DistortedTexCoords).rg * 2.0 - 1.0) * WaveStrength * clamp(WaterDepth / 20.0, 0.0, 1.0);

    RefractTexCoord += TotalDistortion;
    RefractTexCoord = clamp(RefractTexCoord, 0.001, 0.999);

    ReflectTexCoord += TotalDistortion;
    ReflectTexCoord.x = clamp(ReflectTexCoord.x, 0.001, 0.999);
    ReflectTexCoord.y = clamp(ReflectTexCoord.y, -0.999, -0.001);

    vec4 ReflectColor = texture(ReflectionTexture, ReflectTexCoord);
    vec4 RefractColor = texture(RefractionTexture, RefractTexCoord);

    vec4 NormalMapColor = texture(NormalMap, DistortedTexCoords);
    vec3 Normal = vec3(NormalMapColor.r * 2.0 - 1.0, NormalMapColor.b, NormalMapColor.g * 2.0 - 1.0);
    //vec3 Normal = vec3(NormalMapColor.r * 2.0 - 1.0, NormalMapColor.b * 3.0, NormalMapColor.g * 2.0 - 1.0);
    Normal = normalize(Normal);

    vec3 ViewVector = normalize(ToCameraVector);
    float RefractiveFactor = dot(ViewVector, vec3(0.0, 0.0, 1.0));
    //float RefractiveFactor = dot(ViewVector, Normal);
    RefractiveFactor = pow(RefractiveFactor, 0.5);

    vec3 ReflectedLight = reflect(normalize(FromSunVector), Normal);
    float Specular = max(dot(ReflectedLight, ViewVector), 0.0);
    Specular = pow(Specular, ShineDamper);
    vec3 SpecularHighlights = LightColor * Specular * Reflectivity;
    //vec3 SpecularHighlights = LightColor * Specular * Reflectivity * clamp(WaterDepth / 5.0, 0.0, 1.0);

    FragColor = mix(ReflectColor, RefractColor, RefractiveFactor);
    FragColor = mix(FragColor, vec4(0.0, 0.3, 0.5, 1.0), 0.2) + vec4(SpecularHighlights, 0.0);
    //FragColor.a = clamp(WaterDepth / 5.0, 0.0, 1.0);
    //  FragColor = NormalMapColor;
    //  FragColor = vec4(WaterDepth / 50.0);
}