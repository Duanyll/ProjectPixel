#pragma once

#include "pch.h"

struct DirLight {
    glm::vec3 direction = glm::vec3(0.3f, -0.3f, 1.0f);

    glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);

    void apply();

    static void disable();
    static void set_ambient(glm::vec3 color);
};

struct PointLight {
    glm::vec3 position;

    float constant = 1.0;
    float linear = 0.09;
    float quadratic = 0.032;

    glm::vec3 diffuse = glm::vec3(0.4f, 0.4f, 0.4f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);

    void apply(int index);

    static void set_active_count(int count);
};

struct SpotLight {
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

    static void disable();
};