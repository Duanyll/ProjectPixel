#include "pch.h"
#include "Objects.h"

#include "../game/Instructions.h"

FullScreenQuad::FullScreenQuad(pTexture texture) : texture(texture) {}

FullScreenQuad::FullScreenQuad(const std::string& str)
    : texture(AssetsHub::get_texture_2d(str)) {}

void FullScreenQuad::render() {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    auto shader = AssetsHub::get_shader<QuadShader>();
    auto vao = AssetsHub::get_vao("quad");
    shader->configure(texture);
    vao->draw();
    glDisable(GL_BLEND);
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

glm::mat4 EntityRenderer::get_model() {
    return glm::rotate(glm::translate(glm::mat4(), position),
                       glm::radians(facing), {0.0, -1.0, 0.0});
}

void EntityRenderer::step(float time) {
    position += speed * time;
    facing += rotationSpeed * time;
    if (facing < -180) facing += 360;
    if (facing >= 180) facing -= 360;
}

void EntityRenderer::update(EntityInstruction& instruction) {
    position = instruction.pos;
    facing = instruction.facing;
    speed = instruction.speed;
    rotationSpeed = instruction.rotationSpeed;
}

glm::mat4 Paperman::get_model() {
    return glm::scale(EntityRenderer::get_model(),
                      {0.05, 0.05, 0.05});
}

void Paperman::step(float time) {
    EntityRenderer::step(time);
    animationTotal += time;
    if (animationTotal > glm::pi<float>() * 2) {
        animationTotal -= glm::pi<float>() * 2;
    }
    switch (animation) {
        case AnimationType::Standing:
            animationTimer = 0;
            break;
        case AnimationType::Walking:
            animationTimer = glm::sin(animationTotal * 8);
            break;
        case AnimationType::Running:
            animationTimer = glm::sin(animationTotal * 12);
            break;
        case AnimationType::ZombieWalking:
            animationTimer = glm::sin(animationTotal * 5);
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
    } else if (key == "zombie") {
        return {AssetsHub::get_texture_2d("paperman-zombie-diffuse"),
                AssetsHub::get_texture_2d("paperman-zombie-specular"),
                AssetsHub::get_texture_2d("no-emission"), 32, false};
    }
    return {AssetsHub::get_texture_2d("paperman-default"),
            AssetsHub::get_texture_2d("no-specular"),
            AssetsHub::get_texture_2d("no-emission"), 32, false};
}

void Paperman::update(EntityInstruction& instruction) {
    EntityRenderer::update(instruction);
    if (instruction.state == "standing") {
        animation = AnimationType::Standing;
    } else if (instruction.state == "walking") {
        animation = AnimationType::Walking;
    } else if (instruction.state == "running") {
        animation = AnimationType::Running;
    }
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
    if (animation == AnimationType::Walking) {
        base = glm::rotate(base, glm::radians(-30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animation == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(-80.0f - 5 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animation == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(-60 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

glm::mat4 Paperman::get_rarm_model() {
    auto base = glm::translate(glm::mat4(), {0, 22, 0});
    if (animation == AnimationType::Walking) {
        base = glm::rotate(base, glm::radians(30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animation == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(-80.0f + 5 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animation == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(60 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

glm::mat4 Paperman::get_lleg_model() {
    auto base = glm::translate(glm::mat4(), {0, 12, 0});
    if (animation == AnimationType::Walking ||
        animation == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animation == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(45 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

glm::mat4 Paperman::get_rleg_model() {
    auto base = glm::translate(glm::mat4(), {0, 12, 0});
    if (animation == AnimationType::Walking ||
        animation == AnimationType::ZombieWalking) {
        base = glm::rotate(base, glm::radians(-30 * animationTimer),
                           {1.0, 0.0, 0.0});
    } else if (animation == AnimationType::Running) {
        base = glm::rotate(base, glm::radians(-45 * animationTimer),
                           {1.0, 0.0, 0.0});
    }
    return base;
}

std::shared_ptr<EntityRenderer> get_entity_renderer(
    EntityInstruction instruction) {
    std::shared_ptr<EntityRenderer> e;
    if (instruction.type == "player") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = Paperman::get_material_preset("droid");
        paperman->animation = Paperman::AnimationType::Walking;
        e = paperman;
    } else if (instruction.type == "zombie") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = Paperman::get_material_preset("zombie");
        paperman->animation = Paperman::AnimationType::ZombieWalking;
        e = paperman;
    }

    e->update(instruction);

    return e;
}
