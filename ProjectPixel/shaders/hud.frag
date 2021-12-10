#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D img;

void main()
{
    vec4 color = texture(img, TexCoords);
    if (color.a < 0.1) discard;
    FragColor = color;
}