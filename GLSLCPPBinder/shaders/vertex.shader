#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 view[7];
uniform mat4 projection;
uniform mat4 model;

struct Attenuation {
    float aConstant;
    float aLinear;
    float aExp;
};

struct DirectionalLight {
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
    vec3 direction;
};

struct PointLight {
    vec3 color;
    float ambientIntensity;
    float diffuseIntensity;
    vec3 position;
    Attenuation att;
};

uniform DirectionalLight directionalLight;
uniform Attenuation atten;
uniform PointLight poi;

void main()
{
    TexCoords = aTexCoords;    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}