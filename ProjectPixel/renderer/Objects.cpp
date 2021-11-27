#include "pch.h"
#include "Objects.h"

FullScreenQuad::FullScreenQuad(pTexture texture) : texture(texture) {}

FullScreenQuad::FullScreenQuad(const std::string& str)
    : texture(AssetsHub::get_texture_2d(str)) {}

void FullScreenQuad::render() {
    glDisable(GL_DEPTH_TEST);
    auto shader = AssetsHub::get_shader<QuadShader>();
    auto vao = AssetsHub::get_vao("quad");
    shader->configure(texture);
    vao->draw();
    glEnable(GL_DEPTH_TEST);
}

Skybox::Skybox(pCubeTexture texture) : texture(texture) {}

Skybox::Skybox(const std::string& str) : texture(AssetsHub::get_skybox(str)) {}

void Skybox::render() {
    glDepthFunc(GL_LEQUAL);
    auto shader = AssetsHub::get_shader<SkyboxShader>();
    auto vao = AssetsHub::get_vao("skybox");
    shader->configure(texture);
    vao->draw();
    glDepthFunc(GL_LESS);
}

glm::mat4 RenderObjectWithPosition::get_model() {
    return glm::rotate(glm::translate(glm::mat4(), position),
                       glm::radians(facing), {0.0, -1.0, 0.0});
}

void RenderObjectWithSpeed::step(float time) {
    position += speed * time;
    facing += angle_speed * time;
    if (facing < -180) facing += 360;
    if (facing >= 180) facing -= 360;
}

glm::mat4 Paperman::get_model() {
    return glm::scale(RenderObjectWithPosition::get_model(),
                      {0.05, 0.05, 0.05});
}

void Paperman::step(float time) {
    RenderObjectWithSpeed::step(time);
    animationTotal += time;
    if (animationTotal > glm::pi<float>() * 2) {
        animationTotal -= glm::pi<float>() * 2;
    }
    switch (animationType) {
        case AnimationType::Standing:
            animationTimer = 0;
            break;
        case AnimationType::Walking:
            animationTimer = glm::sin(animationTotal * 5);
            break;
        case AnimationType::Running:
            animationTimer = glm::sin(animationTotal * 8);
            break;
        case AnimationType::ZombieWalking:
            animationTimer = glm::sin(animationTotal * 3);
            break;
        default:
            break;
    }
}

void Paperman::render() {
    auto headVAO = AssetsHub::get_vao("paperman-head");
    auto bodyVAO = AssetsHub::get_vao("paperman-body");
    auto larmVAO = AssetsHub::get_vao(material.is_slim ? "paperman-larm-slim"
                                                       : "paperman-larm");
    auto rarmVAO = AssetsHub::get_vao(material.is_slim ? "paperman-rarm-slim"
                                                       : "paperman-rarm");
    auto llegVAO = AssetsHub::get_vao("paperman-lleg");
    auto rlegVAO = AssetsHub::get_vao("paperman-rleg");

    auto baseModel = get_model();
    auto shader = AssetsHub::get_shader<EntityShader>();
    if (!material.emission)
        material.emission = AssetsHub::get_texture_2d("no-emission");
    if (!material.specular)
        material.specular = AssetsHub::get_texture_2d("no-specular");

    shader->configure(material, baseModel * get_head_model());
    headVAO->draw();

    shader->configure(material, baseModel);
    bodyVAO->draw();

    shader->configure(material, baseModel * get_larm_model());
    larmVAO->draw();

    shader->configure(material, baseModel * get_rarm_model());
    rarmVAO->draw();

    shader->configure(material, baseModel * get_lleg_model());
    llegVAO->draw();

    shader->configure(material, baseModel * get_rleg_model());
    rlegVAO->draw();
}

PapermanMatraial Paperman::get_material_preset(const std::string& key) {
    if (key == "droid") {
        return {AssetsHub::get_texture_2d("paperman-droid-diffuse"),
                AssetsHub::get_texture_2d("paperman-droid-specular"),
                AssetsHub::get_texture_2d("paperman-droid-emission"), 64,
                false};
    }
    return {AssetsHub::get_texture_2d("paperman-default"),
            AssetsHub::get_texture_2d("no-specular"),
            AssetsHub::get_texture_2d("no-emission"), 32, false};
}

glm::mat4 Paperman::get_head_model() {
    return glm::rotate(
        glm::rotate(glm::translate(glm::mat4(), {0, 28, 0}),
                    glm::radians(glm::clamp(headYaw, -90.0f, 90.0f)),
                    {0.0, -1.0, 0.0}),
        glm::radians(glm::clamp(headPitch, -75.0f, 75.0f)), {-1.0, 0.0, 0.0});
}

glm::mat4 Paperman::get_body_model() { return glm::mat4(); }

glm::mat4 Paperman::get_larm_model() {
    auto base = glm::translate(glm::mat4(), {0, 22, 0});
    if (animationType == AnimationType::Walking) {
        base = glm::rotate(base, glm::radians(-30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animationType == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(-80.0f - 5 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animationType == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(-60 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

glm::mat4 Paperman::get_rarm_model() {
    auto base = glm::translate(glm::mat4(), {0, 22, 0});
    if (animationType == AnimationType::Walking) {
        base = glm::rotate(base, glm::radians(30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animationType == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(-80.0f + 5 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animationType == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(60 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

glm::mat4 Paperman::get_lleg_model() {
    auto base = glm::translate(glm::mat4(), {0, 12, 0});
    if (animationType == AnimationType::Walking ||
        animationType == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animationType == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(45 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

glm::mat4 Paperman::get_rleg_model() {
    auto base = glm::translate(glm::mat4(), {0, 12, 0});
    if (animationType == AnimationType::Walking ||
        animationType == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(-30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animationType == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(-45 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}
