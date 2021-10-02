#version 330 core
out vec4 FragColor;

in vec3 fragPosition;
in vec3 fragNormal;
in float fragShininess;
in vec3 fragC;
in vec2 TexCoords;

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

struct DirectionalLight {
    BaseLight light;
    vec3 direction;
};

struct PointLight {
    BaseLight light;
    Attenuation att;
    vec3 position;
};

struct SpotLight {
    PointLight base;
    vec3 direction;
    float cutoff;
};

// uniforms

uniform DirectionalLight directionalLight;
uniform PointLight pointLight;
uniform SpotLight spotLight;

uniform float materialSpecularIntensity;
uniform float materialShininess;

uniform vec3 worldCameraPos;

// functions

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

vec4 calcLightInternal(BaseLight light, vec3 lightDirection, vec3 normal) {
    vec4 ambientColor = vec4(light.color, 1.0f) * light.ambientIntensity;
    vec4 diffuseColor = vec4(0.0);
    vec4 specularColor = vec4(0.0);

    float diffuseFactor = dot(normalize(normal), -lightDirection);

    if (diffuseFactor > 0) {
        diffuseColor = vec4(light.color * diffuseFactor * light.diffuseIntensity, 1.0);

        float shininess = materialShininess * 4;

        float kEnergyConservation = (8.0 + shininess) / (8.0 * 3.14159265);

        vec3 viewDir =    normalize(worldCameraPos - fragPosition);

        // blinn-phong shading:
        vec3 halfwayDir = normalize(viewDir - lightDirection);
        float specularFactor = max(dot(fragNormal, halfwayDir), 0.0);

        // phong shading:
        /*vec3 reflectDir = reflect(lightDirection, fragNormal);
        float specularFactor = max(dot(reflectDir, viewDir), 0.0);*/

        specularFactor = kEnergyConservation * pow(specularFactor, shininess);

        //specularColor = vec4(specularFactor, specularFactor, specularFactor, 1.0);
        specularColor = vec4(light.color * materialSpecularIntensity * specularFactor, 1.0);
    }

    //return specularColor;
    return (ambientColor + diffuseColor + specularColor);
}

vec4 calcPointLight(PointLight pointLight, vec3 normal) {
    vec3 lightDirection = fragPosition - pointLight.position;
    float lightDistance = length(lightDirection);
    lightDirection = normalize(lightDirection);

    float attenuation = (pointLight.att.aConstant
                       + pointLight.att.aLinear * lightDistance
                       + pointLight.att.aQuadratic * (lightDistance * lightDistance));

    return calcLightInternal(pointLight.light, lightDirection, normal) / attenuation;
}

vec4 calcSpotLight(SpotLight spotLight, vec3 normal) {
    vec3 lightToFragment = normalize(fragPosition - spotLight.base.position);
    float spotFactor = dot(lightToFragment, spotLight.direction);

    if (spotFactor > spotLight.cutoff) {
        vec4 color = calcPointLight(spotLight.base, normal);
        return color * (1.0 - (1.0 - spotFactor) * 1.0 / (1.0 - spotLight.cutoff));
    }
    else {
        return vec4(0, 0, 0, 0);
    }
}

void main() {
    vec3 normal = fragNormal;
    vec4 spotColor = calcSpotLight(spotLight, normal);
    FragColor = spotColor + calcPointLight(pointLight, normal);
}