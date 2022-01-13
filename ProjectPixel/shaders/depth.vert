#version 330 core
layout (location = 0) in vec3 aPos;

uniform LightSpace {
    mat4 projection;
    mat4 view;   
    vec3 viewPos; 
};

uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}