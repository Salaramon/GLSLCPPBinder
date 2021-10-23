#version 330 core
out vec4 FragColor;

uniform sampler2D shadowMap;

in vec2 TexCoords;

void main()
{
    float Depth = texture(shadowMap, TexCoords).x;
    Depth = (1.0 - Depth) * 15.0;
    FragColor = vec4(1 - Depth);
}

