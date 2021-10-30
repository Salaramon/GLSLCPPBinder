#version 330 core
in vec2 TexCoords;

out vec4 FragColor;

// light types

struct Attenuation {
    float aConstant;
    float aLinear;
    float aQuadratic;
};

struct BaseLight {
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
};

struct PointLight {
    BaseLight light;
    Attenuation att;
    vec3 position;
};

// gbuffer data

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;

// uniforms

const int MAX_LIGHTS = 32;

uniform vec3 worldCameraPos;

uniform PointLight pointLights[MAX_LIGHTS];

uniform float materialSpecularIntensity;
uniform float materialShininess;

// functions

vec4 calcLightInternal(BaseLight light, vec3 lightDirection, vec3 fragPosition, vec3 fragNormal) {
    vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;
    vec4 diffuseColor = vec4(0.0);
    vec4 specularColor = vec4(0.0);

    float diffuseFactor = dot(fragNormal, -lightDirection);
    if (diffuseFactor > 0.0) {
        diffuseColor = vec4(light.color * diffuseFactor * light.diffuseIntensity, 1.0);

        float shininess = materialShininess * 4;
        float kEnergyConservation = (8.0 + shininess) / (8.0 * 3.14159265);
        vec3 viewDir = normalize(worldCameraPos - fragPosition);

        // blinn-phong shading impl
        /*vec3 halfwayDir = normalize(viewDir - lightDirection);
        float specularFactor = max(dot(fragNormal, halfwayDir), 0.0);*/
        vec3 reflectDir = reflect(lightDirection, fragNormal);
        float specularFactor = max(dot(reflectDir, viewDir), 0.0);

        specularFactor = kEnergyConservation * pow(specularFactor, shininess);
        specularColor = vec4(light.color * materialSpecularIntensity * specularFactor, 1.0);
    }

    return (ambientColor + diffuseColor + specularColor);
}

vec4 calcPointLight(PointLight pointLight, vec3 fragPosition, vec3 fragNormal) {
    vec3 lightDirection = fragPosition - pointLight.position;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    float attenuation = (pointLight.att.aConstant
        + pointLight.att.aLinear * lightDistance
        + pointLight.att.aQuadratic * (lightDistance * lightDistance));

    return calcLightInternal(pointLight.light, lightDirection, fragPosition, fragNormal) / attenuation;
}


void main()
{
    if (texture(gPosition, TexCoords).w == 1.0) {
        FragColor = vec4(0.0);
        return;
    }
    vec3 fragPosition = texture(gPosition, TexCoords).xyz;
    vec3 fragNormal = texture(gNormal, TexCoords).xyz;
    vec3 diffuse = texture(gColor, TexCoords).rgb;

    vec4 totalLight = vec4(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++) {
        totalLight += calcPointLight(pointLights[i], fragPosition, fragNormal);
    }

    const float gamma = 2.2;
    vec4 fragColor = vec4(diffuse, 1.0) + totalLight;
    FragColor = vec4(pow(fragColor.xyz, vec3(1.0 / gamma)), 1.0);
}
