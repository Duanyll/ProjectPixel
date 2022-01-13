#pragma once

#include "pch.h"
#include "Texture.h"

struct DirLight {
    bool isActive = true;

    glm::vec3 direction = glm::vec3(0.3f, -0.3f, 1.0f);

    glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);

    glm::vec3 focus;

    void apply();

    pDepthMap depthMap;
    void init_depth_map();
    glm::mat4 get_view();
    glm::mat4 get_projection();
    void render_depth(std::function<void()> func);
};

struct PointLight {
    glm::vec3 position;

    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;

    glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 specular = glm::vec3(.0f, 1.0f, 1.0f);

    void apply(int index);
};

struct SpotLight {
    bool isActive = false;

    glm::vec3 position;
    glm::vec3 direction;

    float cutOff = 12.5f;
    float outerCutOff = 15.0f;

    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;

    glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    void apply();

    pDepthMap depthMap;
    void init_depth_map();
    glm::mat4 get_view();
    glm::mat4 get_projection();
    void render_depth(std::function<void()> func);
};

struct LightSet {
    DirLight dirLight;
    PointLight pointLights[4];
    int pointLightCount = 0;
    SpotLight spotLight;
    glm::vec3 ambient = {0.2, 0.2, 0.2};

    void apply();
};

extern LightSet Lights;