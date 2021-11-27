#include "pch.h"
#include "Light.h"

#include "Uniform.h"

void DirLight::apply() {
    Uniform::set_data("Lights", "useDirLight", true);
    Uniform::set_data("Lights", "dirLight.direction", direction);
    Uniform::set_data("Lights", "dirLight.diffuse", diffuse);
    Uniform::set_data("Lights", "dirLight.specular", specular);
}

void DirLight::disable() { Uniform::set_data("Lights", "useDirLight", false); }

void DirLight::set_ambient(glm::vec3 color) {
    Uniform::set_data("Lights", "ambientLight", color);
}

void PointLight::apply(int index) { 
    auto structName = std::format("pointLight[{}].", index);
    Uniform::set_data("Lights", structName + "position", position);
    Uniform::set_data("Lights", structName + "constant", constant);
    Uniform::set_data("Lights", structName + "linear", linear);
    Uniform::set_data("Lights", structName + "quadratic", quadratic);
    Uniform::set_data("Lights", structName + "diffuse", diffuse);
    Uniform::set_data("Lights", structName + "specular", specular);
}

void PointLight::set_active_count(int count) {
    Uniform::set_data("Lights", "pointLightCount", count);
}

void SpotLight::apply() {
    Uniform::set_data("Lights", "useSpotLight", true);
    Uniform::set_data("Lights", "spotLight.position", position);
    Uniform::set_data("Lights", "spotLight.direction", direction);
    Uniform::set_data("Lights", "spotLight.cutOff", glm::cos(glm::radians(cutOff)));
    Uniform::set_data("Lights", "spotLight.outerCutOff", glm::cos(glm::radians(outerCutOff)));
    Uniform::set_data("Lights", "spotLight.constant", constant);
    Uniform::set_data("Lights", "spotLight.linear", linear);
    Uniform::set_data("Lights", "spotLight.quadratic", quadratic);
    Uniform::set_data("Lights", "spotLight.diffuse", diffuse);
    Uniform::set_data("Lights", "spotLight.specular", specular);
}

void SpotLight::disable() {
    Uniform::set_data("Lights", "useSpotLight", false);
}
