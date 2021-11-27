#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform Camera {
    mat4 projection;
    mat4 view;   
    vec3 viewPos; 
};

uniform mat4 model;
uniform mat3 normal;

out VS_OUT {
    vec3 FragPos;  
    vec3 Normal;
    vec2 TexCoord;
} vs_out;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normal * aNormal;
    vs_out.TexCoord = aTexCoord;
}