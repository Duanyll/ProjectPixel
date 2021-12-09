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
    return glm::scale(EntityRenderer::get_model(), {0.05, 0.05, 0.05});
}

void Paperman::step(float time) {
    EntityRenderer::step(time);
    legBase.step(time);
    legReal.step_to(time, legBase.value);
    handBase.step(time);
    handReal.step_to(time, handBase.value);
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

    if (handItem != Item::None) {
        pVAO itemVAO;
        Material itemMaterial;
        get_item_resources(handItem, itemVAO, itemMaterial);
        shader->configure(itemMaterial,
                          EntityRenderer::get_model() * get_item_model());
        itemVAO->draw();
    }
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

void Paperman::update(EntityInstruction& i) {
    EntityRenderer::update(i);

    set_leg_action((LegAction)i.state[0]);
    set_hand_action((HandAction)i.state[1]);
    handItem = (Item)i.state[2];
}

void Paperman::set_leg_action(LegAction action) {
    if (leg == action) return;
    leg = action;
    switch (action) {
        case LegAction::Standing:
            legBase.amplitude = 0;
            break;
        case LegAction::Walking:
            legBase.amplitude = 30;
            legBase.peirod = 1;
            break;
        case LegAction::Running:
            legBase.amplitude = 60;
            legBase.peirod = 0.5;
            break;
        default:
            break;
    }
}

void Paperman::set_hand_action(HandAction action) {
    if (hand == action) return;
    hand = action;
    switch (action) {
        case HandAction::None:
            break;
        case HandAction::ZombieHanging:
            break;
        case HandAction::Holding:
            break;
        case HandAction::Attacking:
            handBase.value = 60;
            handBase.speed = 360;
            handReal.maxAcc = 1440;
            break;
        case HandAction::ZombieAttacking:
            handBase.value = 60;
            handBase.speed = 180;
            handReal.maxAcc = 720;
            break;
        default:
            break;
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
    if (hand == HandAction::ZombieHanging) {
        base = glm::rotate(base, glm::radians(-80.0f - 0.05f * legReal.value),
                           {1, 0, 0});

    } else if (hand == HandAction::ZombieAttacking) {
        base = glm::rotate(base, glm::radians(-80.0f - handReal.value),
                           {1, 0, 0});
    } else {
        base =
            glm::rotate(base, glm::radians(-0.8f * legReal.value), {1, 0, 0});
    }
    return base;
}

glm::mat4 Paperman::get_rarm_model() {
    auto base = glm::translate(glm::mat4(), {0, 22, 0});
    if (hand == HandAction::ZombieHanging) {
        base = glm::rotate(base, glm::radians(-80.0f + 0.05f * legReal.value),
                           {1, 0, 0});
    } else if (hand == HandAction::Holding) {
        base = glm::rotate(base, glm::radians(-45.0f + 0.05f * legReal.value),
                           {1, 0, 0});
    } else if (hand == HandAction::Attacking) {
        base = glm::rotate(base, glm::radians(-45.0f - handReal.value * 2),
                           {1, 0, 0});
    } else if (hand == HandAction::ZombieAttacking) {
        base =
            glm::rotate(base, glm::radians(-80.0f - handReal.value), {1, 0, 0});
    } else {
        base = glm::rotate(base, glm::radians(0.8f * legReal.value), {1, 0, 0});
    }
    return base;
}

glm::mat4 Paperman::get_lleg_model() {
    auto base = glm::translate(glm::mat4(), {0, 12, 0});
    base = glm::rotate(base, glm::radians(legReal.value), {1, 0, 0});
    return base;
}

glm::mat4 Paperman::get_rleg_model() {
    auto base = glm::translate(glm::mat4(), {0, 12, 0});
    base = glm::rotate(base, glm::radians(-legReal.value), {1, 0, 0});
    return base;
}

glm::mat4 Paperman::get_item_model() {
    if (hand == HandAction::None) {
        glm::mat4 base;
        base = glm::translate(base, {-0.5, 1.35, -0.1});
        base = glm::rotate(base, glm::radians(-90.0f), {1, 0, 0});
        base = glm::translate(base, {0, 0, -1});
        return base;
    } else {
        glm::mat4 base;
        base = glm::translate(base, {-0.3, 1.2, 0});
        if (hand == HandAction::ZombieHanging) {
            base = glm::rotate(
                base, glm::radians(-80.0f + 0.05f * legReal.value), {1, 0, 0});
        } else if (hand == HandAction::Holding) {
            base = glm::rotate(
                base, glm::radians(-45.0f + 0.05f * legReal.value), {1, 0, 0});
        } else if (hand == HandAction::Attacking) {
            base = glm::rotate(base, glm::radians(-45.0f - handReal.value * 2),
                               {1, 0, 0});
        } else if (hand == HandAction::ZombieAttacking) {
            base = glm::rotate(base, glm::radians(-80.0f - handReal.value),
                               {1, 0, 0});
        }
        base = glm::translate(base, {0, -0.4, 0});
        base = glm::rotate(base, glm::radians(handReal.value - 75), {-1, 0, 0});
        base = glm::rotate(base, glm::radians(10.0f), {0, 1, 0});
        base = glm::translate(base, {0, -0.2, 0.8});
        base = glm::rotate(base, glm::radians(180.0f), {0, 1, 0});
        base = glm::rotate(base, glm::radians(90.0f), {0, 0, 1});
        return base;
    }
}

void Paperman::get_item_resources(Item item, pVAO& vao, Material& material) {
    switch (item) {
        case Item::None:
            break;
        case Item::DiamondSword:
            vao = AssetsHub::get_vao("item-diamond-sword");
            material = {
                AssetsHub::get_texture_2d("item-diamond-sword"),
                AssetsHub::get_texture_2d("item-diamond-sword-specular"),
                AssetsHub::get_texture_2d("no-emission"), 32};
            break;
        case Item::DiamondAxe:
            vao = AssetsHub::get_vao("item-diamond-axe");
            material = {AssetsHub::get_texture_2d("item-diamond-axe"),
                        AssetsHub::get_texture_2d("item-diamond-axe-specular"),
                        AssetsHub::get_texture_2d("no-emission"), 32};
            break;
        case Item::Bow:
            vao = AssetsHub::get_vao("item-bow");
            material = {AssetsHub::get_texture_2d("item-bow"),
                        AssetsHub::get_texture_2d("no-specular"),
                        AssetsHub::get_texture_2d("no-emission"), 32};
            break;
        default:
            break;
    }
}

std::shared_ptr<EntityRenderer> get_entity_renderer(
    EntityInstruction instruction) {
    std::shared_ptr<EntityRenderer> e;
    if (instruction.type == "player") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = Paperman::get_material_preset("droid");
        paperman->handItem = Item::DiamondSword;
        e = paperman;
    } else if (instruction.type == "zombie") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = Paperman::get_material_preset("zombie");
        paperman->hand = HandAction::ZombieHanging;
        e = paperman;
    }

    e->update(instruction);

    return e;
}

void SineAnimation::step(float time) {
    if (amplitude == 0) {
        value = 0;
        return;
    }
    total += time * 2 * glm::pi<float>() / peirod;
    if (total > 2 * glm::pi<float>()) {
        total -= 2 * glm::pi<float>();
    }
    value = amplitude * std::sin(total);
}

void AccelerateAdapter::step_to(float time, float target) {
    auto delta = target - value;
    auto acc = maxAcc * time;
    if (std::abs(delta) <= acc) {
        value = target;
    } else if (delta > 0) {
        value += acc;
    } else {
        value -= acc;
    }
}

void LinearAnimation::step(float time) {
    if (value > 0) {
        auto delta = speed * time;
        if (delta >= value) {
            value = 0;
        } else {
            value -= delta;
        }
    }
}
