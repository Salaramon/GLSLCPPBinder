#version 330 core
out vec4 FragColor;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;

uniform sampler2D diffuse1;

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{    
    //FragColor = vec4(rand(WorldPos0.xy), rand(WorldPos0.xz), rand(WorldPos0.yz), 0);
    FragColor = vec4(255 - Normal0.x, 255 - Normal0.y, 255 - Normal0.z, 0);
}

/*
#version 330 core
out vec4 FragColor;

in vec2 TexCoord0;
in vec3 Normal0;
in vec3 WorldPos0;


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
    vec3 Position;
    Attenuation Atten;
};

uniform int numPointLights;

uniform sampler2D diffuse1;

uniform DirectionalLight directionalLight;
uniform PointLight pointLights;
uniform vec3 eyeWorldPos;
uniform float matSpecularIntensity;
uniform float specularPower;

vec4 CalcLightInternal(BaseLight Light, vec3 LightDirection, vec3 Normal)
{
    vec4 AmbientColor = vec4(Light.Color * Light.AmbientIntensity, 1.0f);
    float DiffuseFactor = dot(Normal, -LightDirection);

    vec4 DiffuseColor = vec4(0, 0, 0, 0);
    vec4 SpecularColor = vec4(0, 0, 0, 0);

    if (DiffuseFactor > 0) {
        DiffuseColor = vec4(Light.Color * Light.DiffuseIntensity * DiffuseFactor, 1.0f);

        vec3 VertexToEye = normalize(gEyeWorldPos - WorldPos0);
        vec3 LightReflect = normalize(reflect(LightDirection, Normal));
        float SpecularFactor = dot(VertexToEye, LightReflect);
        if (SpecularFactor > 0) {
            SpecularFactor = pow(SpecularFactor, gSpecularPower);
            SpecularColor = vec4(Light.Color * gMatSpecularIntensity * SpecularFactor, 1.0f);
        }
    }

    return (AmbientColor + DiffuseColor + SpecularColor);
}

vec4 CalcDirectionalLight(vec3 Normal)
{
    return CalcLightInternal(gDirectionalLight.Base, gDirectionalLight.Direction, Normal);
}

vec4 CalcPointLight(int Index, vec3 Normal)
{
    vec3 LightDirection = WorldPos0 - gPointLights[Index].Position;
    float Distance = length(LightDirection);
    LightDirection = normalize(LightDirection);

    vec4 Color = CalcLightInternal(gPointLights[Index].Base, LightDirection, Normal);
    float Attenuation = gPointLights[Index].Atten.Constant +
        gPointLights[Index].Atten.Linear * Distance +
        gPointLights[Index].Atten.Exp * Distance * Distance;

    return Color / Attenuation;
}

void main()
{
    vec3 Normal = normalize(Normal0);
    vec4 TotalLight = CalcDirectionalLight(Normal);

    for (int i = 0; i < gNumPointLights; i++) {
        TotalLight += CalcPointLight(i, Normal);
    }

    //FragColor = texture2D(diffuse1, TexCoord0.xy) * TotalLight;
    FragColor = TotalLight;
}
*/