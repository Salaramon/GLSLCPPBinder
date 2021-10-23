#version 450

// multiple render targets, gbuffer
layout(location = 0) out vec3 gPosition;
layout(location = 1) out vec3 gNormal;
layout(location = 2) out vec4 gColor;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 TexCoords;

uniform sampler2D diffuse1;

void main()
{
    gPosition = fragPosition;
    gNormal = normalize(fragNormal);
    gColor = texture(diffuse1, TexCoords).rgba;
}
