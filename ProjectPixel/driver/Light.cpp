#include "pch.h"
#include "Light.h"

#include "Uniform.h"

void DirLight::apply() {
    if (isActive) {
        Uniform::set_data("Lights", "useDirLight", true);
        Uniform::set_data("Lights", "dirLight.direction", direction);
        Uniform::set_data("Lights", "dirLight.diffuse", diffuse);
        Uniform::set_data("Lights", "dirLight.specular", specular);
        Uniform::set_data("Lights", "dirLight.lightSpace",
                          get_projection() * get_view());
    } else {
        Uniform::set_data("Lights", "useDirLight", false);
    }
}

void DirLight::render_depth(std::function<void()> func) {
    Uniform::set_data("LightSpace", "view", get_view());
    Uniform::set_data("LightSpace", "projection", get_projection());
    Uniform::set_data("LightSpace", "viewPos", glm::vec3(0, 0, 0));

    depthMap->draw_inside(func);
}

glm::mat4 DirLight::get_view() {
    return glm::lookAt(focus - glm::normalize(direction) * 20.0f, focus,
                       {0, 0, 1});
}

void DirLight::init_depth_map() {
    depthMap = std::make_shared<DepthMap>(1024, 1024);
}

glm::mat4 DirLight::get_projection() {
    return glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.0f, 40.0f);
}

void PointLight::apply(int index) {
    auto structName = std::format("pointLights[{}].", index);
    Uniform::set_data("Lights", structName + "position", position);
    Uniform::set_data("Lights", structName + "constant", constant);
    Uniform::set_data("Lights", structName + "linear", linear);
    Uniform::set_data("Lights", structName + "quadratic", quadratic);
    Uniform::set_data("Lights", structName + "diffuse", diffuse);
    Uniform::set_data("Lights", structName + "specular", specular);
}

void SpotLight::apply() {
    if (isActive) {
        Uniform::set_data("Lights", "useSpotLight", true);
        Uniform::set_data("Lights", "spotLight.position", position);
        Uniform::set_data("Lights", "spotLight.direction", direction);
        Uniform::set_data("Lights", "spotLight.cutOff",
                          glm::cos(glm::radians(cutOff)));
        Uniform::set_data("Lights", "spotLight.outerCutOff",
                          glm::cos(glm::radians(outerCutOff)));
        Uniform::set_data("Lights", "spotLight.constant", constant);
        Uniform::set_data("Lights", "spotLight.linear", linear);
        Uniform::set_data("Lights", "spotLight.quadratic", quadratic);
        Uniform::set_data("Lights", "spotLight.diffuse", diffuse);
        Uniform::set_data("Lights", "spotLight.specular", specular);
        Uniform::set_data("Lights", "spotLight.lightSpace",
                          get_projection() * get_view());
    } else {
        Uniform::set_data("Lights", "useSpotLight", false);
    }
}

void SpotLight::init_depth_map() {
    depthMap = std::make_shared<DepthMap>(1024, 1024);
}

glm::mat4 SpotLight::get_view() {
    return glm::lookAt(position, position + direction, {0, 0, 1});
}

glm::mat4 SpotLight::get_projection() {
    return glm::perspective(glm::radians(outerCutOff * 2), 1.0f, 1.0f, 10.0f);
}

void SpotLight::render_depth(std::function<void()> func) {
    Uniform::set_data("LightSpace", "view", get_view());
    Uniform::set_data("LightSpace", "projection", get_projection());
    Uniform::set_data("LightSpace", "viewPos", position);

    depthMap->draw_inside(func);
}

void LightSet::apply() {
    Uniform::set_data("Lights", "ambientLight", ambient);
    dirLight.apply();
    Uniform::set_data("Lights", "pointLightCount", pointLightCount);
    for (int i = 0; i < pointLightCount; i++) {
        pointLights[i].apply(i);
    }
    spotLight.apply();
}

LightSet Lights;