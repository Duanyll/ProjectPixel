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

struct DirLight {
    vec3 direction;

    vec3 diffuse;
    vec3 specular;

    mat4 lightSpace;
};  

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 diffuse;
    vec3 specular;
};  

struct SpotLight {
    vec3 position;
    vec3 direction;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 diffuse;
    vec3 specular;

    mat4 lightSpace;
};

#define NR_POINT_LIGHTS 4
uniform Lights {
    bool useDirLight;
    DirLight dirLight;
    int pointLightCount;
    PointLight pointLights[NR_POINT_LIGHTS];
    bool useSpotLight;
    SpotLight spotLight;
    vec3 ambientLight;
};

out VS_OUT {
    vec3 FragPos;  
    vec3 Normal;
    vec2 TexCoord;
    vec3 FragPosDirLight;
    vec3 FragPosSpotLight;
} vs_out;

vec3 projection_division(vec4 x) {
    return x.xyz / x.w;
}

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
    vs_out.Normal = normal * aNormal;
    vs_out.TexCoord = aTexCoord;
    if (useDirLight) {
        vs_out.FragPosDirLight = projection_division(dirLight.lightSpace * vec4(aPos, 1.0)) * 0.5 + 0.5;
    }
    if (useSpotLight) {
        vs_out.FragPosSpotLight = projection_division(spotLight.lightSpace * vec4(aPos, 1.0)) * 0.5 + 0.5;
    }
}