#include "pch.h"
#include "Objects.h"

#include "../game/Instructions.h"
#include "../utils/Geomentry.h"
#include "../driver/Flags.h"

FullScreenQuad::FullScreenQuad(pTexture texture) : texture(texture) {}

FullScreenQuad::FullScreenQuad(const std::string& str)
    : texture(AssetsHub::get_texture_2d(str)) {}

void FullScreenQuad::render() {
    DepthTest d(false);
    Blend b(true);
    auto shader = AssetsHub::get_shader<BlitShader>();
    auto vao = AssetsHub::get_vao("quad");
    shader->configure(texture);
    vao->draw();
}

Skybox::Skybox(pCubeTexture texture) : texture(texture) {}

Skybox::Skybox(const std::string& str) : texture(AssetsHub::get_skybox(str)) {}

void Skybox::render() {
    DepthFunc f(GL_LEQUAL);
    auto shader = AssetsHub::get_shader<SkyboxShader>();
    auto vao = AssetsHub::get_vao("skybox");
    shader->configure(texture);
    vao->draw();
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
    highlight = instruction.highlight;
}

glm::mat4 Paperman::get_model() {
    auto base = EntityRenderer::get_model();
    if (leg == LegAction::Lying) {
        base = glm::rotate(base, glm::radians(90 - lieBase.value), {0, 0, 1});
    }
    return base;
}

void Paperman::step(float time) {
    EntityRenderer::step(time);
    legBase.step(time);
    legReal.step_to(time, legBase.value);
    handBase.step(time);
    handReal.step_to(time, handBase.value);
    lieBase.step(time);

    if (handItem == ItemType::Bow && hand == HandAction::Attacking) {
        pullBowTime += time;
    }
}

void Paperman::render() {
    auto headVAO = AssetsHub::get_vao("paperman-head");
    auto bodyVAO = AssetsHub::get_vao("paperman-body");
    auto larmVAO =
        AssetsHub::get_vao(isSlim ? "paperman-larm-slim" : "paperman-larm");
    auto rarmVAO =
        AssetsHub::get_vao(isSlim ? "paperman-rarm-slim" : "paperman-rarm");
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

    shader->configure(material, baseModel * get_body_model());
    bodyVAO->draw();

    shader->configure(material, baseModel * get_larm_model());
    larmVAO->draw();

    shader->configure(material, baseModel * get_rarm_model());
    rarmVAO->draw();

    shader->configure(material, baseModel * get_lleg_model());
    llegVAO->draw();

    shader->configure(material, baseModel * get_rleg_model());
    rlegVAO->draw();

    if (handItem != ItemType::None) {
        pVAO itemVAO;
        Material itemMaterial;
        get_item_resources(handItem, itemVAO, itemMaterial);
        shader->configure(itemMaterial, baseModel * get_item_model());
        itemVAO->draw();
    }
}

void Paperman::render_depth() {
    auto headVAO = AssetsHub::get_vao("paperman-head");
    auto bodyVAO = AssetsHub::get_vao("paperman-body");
    auto larmVAO =
        AssetsHub::get_vao(isSlim ? "paperman-larm-slim" : "paperman-larm");
    auto rarmVAO =
        AssetsHub::get_vao(isSlim ? "paperman-rarm-slim" : "paperman-rarm");
    auto llegVAO = AssetsHub::get_vao("paperman-lleg");
    auto rlegVAO = AssetsHub::get_vao("paperman-rleg");

    auto baseModel = get_model();
    auto shader = AssetsHub::get_shader<DepthShader>();

    shader->configure(baseModel * get_head_model());
    headVAO->draw();

    shader->configure(baseModel * get_body_model());
    bodyVAO->draw();

    shader->configure(baseModel * get_larm_model());
    larmVAO->draw();

    shader->configure(baseModel * get_rarm_model());
    rarmVAO->draw();

    shader->configure(baseModel * get_lleg_model());
    llegVAO->draw();

    shader->configure(baseModel * get_rleg_model());
    rlegVAO->draw();

    if (handItem != ItemType::None) {
        pVAO itemVAO;
        Material itemMaterial;
        get_item_resources(handItem, itemVAO, itemMaterial);
        shader->configure(baseModel * get_item_model());
        itemVAO->draw();
    }
}

void Paperman::update(EntityInstruction& i) {
    EntityRenderer::update(i);

    set_leg_action((LegAction)i.state[0]);
    set_hand_action((HandAction)i.state[1]);
    handItem = (ItemType)i.state[2];
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
        case LegAction::Lying:
            lieBase.value = 90;
            break;
        default:
            break;
    }
}

void Paperman::set_hand_action(HandAction action) {
    if (hand == action) return;
    hand = action;
    pullBowTime = 0;
    switch (action) {
        case HandAction::None:
            break;
        case HandAction::ZombieHanging:
            break;
        case HandAction::Holding:
            break;
        case HandAction::Attacking:
            handBase.value = 60;
            handBase.speed = 240;
            handReal.maxAcc = 1440;
            break;
        case HandAction::ZombieAttacking:
            handBase.value = 60;
            handBase.speed = 180;
            handReal.maxAcc = 720;
            break;
        case HandAction::Sweeping:
            break;
        default:
            break;
    }
}

glm::mat4 Paperman::get_base_rarm() {
    glm::mat4 base;
    if (hand == HandAction::ZombieHanging) {
        base = glm::rotate(base, glm::radians(-80.0f + 0.05f * legReal.value),
                           {1, 0, 0});
    } else if (hand == HandAction::Holding || hand == HandAction::Sweeping) {
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

glm::mat4 Paperman::get_head_model() {
    return glm::rotate(
        glm::rotate(glm::translate(glm::scale(glm::mat4(), {0.05, 0.05, 0.05}),
                                   {0, 28, 0}),
                    glm::radians(glm::clamp(headYaw, -90.0f, 90.0f)),
                    {0.0, -1.0, 0.0}),
        glm::radians(glm::clamp(headPitch, -75.0f, 75.0f)), {-1.0, 0.0, 0.0});
}

glm::mat4 Paperman::get_body_model() {
    return glm::scale(glm::mat4(), {0.05, 0.05, 0.05});
}

glm::mat4 Paperman::get_larm_model() {
    glm::mat4 base;
    base = glm::scale(base, {0.05, 0.05, 0.05});
    base = glm::translate(base, {6, 22, 0});
    if (handItem == ItemType::Bow &&
        (hand == HandAction::Attacking || hand == HandAction::Holding)) {
        base = glm::translate(base, {0, 0, 3.5});
        base = glm::rotate(base, glm::radians(-22.5f), {0, 1, 0});
        base = glm::rotate(base, glm::radians(-90.0f), {1, 0, 0});
    } else if (hand == HandAction::ZombieHanging) {
        base = glm::rotate(base, glm::radians(-80.0f - 0.05f * legReal.value),
                           {1, 0, 0});

    } else if (hand == HandAction::ZombieAttacking) {
        base =
            glm::rotate(base, glm::radians(-80.0f - handReal.value), {1, 0, 0});
    } else {
        base =
            glm::rotate(base, glm::radians(-0.8f * legReal.value), {1, 0, 0});
    }
    return base;
}

glm::mat4 Paperman::get_rarm_model() {
    glm::mat4 base;
    base = glm::scale(base, {0.05, 0.05, 0.05});
    base = glm::translate(base, {-6, 22, 0});
    if (handItem == ItemType::Bow &&
        (hand == HandAction::Attacking || hand == HandAction::Holding)) {
        base = glm::rotate(base, glm::radians(30.0f), {0, 1, 0});
        base = glm::rotate(base, glm::radians(-80.0f), {1, 0, 0});
        return base;
    }
    return base * get_base_rarm();
}

glm::mat4 Paperman::get_lleg_model() {
    glm::mat4 base;
    base = glm::scale(base, {0.05, 0.05, 0.05});
    base = glm::translate(base, {0, 12, 0});
    base = glm::rotate(base, glm::radians(legReal.value), {1, 0, 0});
    return base;
}

glm::mat4 Paperman::get_rleg_model() {
    glm::mat4 base;
    base = glm::scale(base, {0.05, 0.05, 0.05});
    base = glm::translate(base, {0, 12, 0});
    base = glm::rotate(base, glm::radians(-legReal.value), {1, 0, 0});
    return base;
}

glm::mat4 Paperman::get_item_model() {
    glm::mat4 base;
    if (hand == HandAction::None) {
        base = glm::translate(base, {-0.5, 1.35, -0.1});
        base = glm::scale(base, {0.8, 0.8, 0.8});
        base = glm::rotate(base, glm::radians(-90.0f), {1, 0, 0});
        base = glm::translate(base, {0, 0, -1});
    } else if (handItem == ItemType::Bow &&
               (hand == HandAction::Attacking || hand == HandAction::Holding)) {
        base = glm::translate(base, {0, 0.4, 0.5});
        base = glm::rotate(base, glm::radians(-45.0f), {1, 0, 0});
        base = glm::scale(base, {0.8, 0.8, 0.8});
        base = glm::translate(base, {0, 0, 1});
        base = glm::rotate(base, glm::radians(180.0f), {0, 1, 0});
        base = glm::rotate(base, glm::radians(90.0f), {0, 0, 1});
    } else if (handItem == ItemType::LifePotion) {
        base = glm::translate(base, {0, 0.75, 0.25});
        base = glm::scale(base, {0.6, 0.6, 0.6});
        base = glm::translate(base, {-0.5, 0, 0});
        base = glm::rotate(base, glm::radians(90.0f), {1, 0, 0});
        base = glm::translate(base, {0, 0, -1});
    } else{
        base = glm::translate(base, {-0.3, 1.2, 0});
        base = base * get_base_rarm();
        base = glm::translate(base, {0, -0.4, 0});
        if (hand != HandAction::Sweeping) {
            base = glm::rotate(base, glm::radians(handReal.value - 75),
                               {-1, 0, 0});
        } else {
            base = glm::rotate(base, glm::radians(45.0f), {1, 0, 0});
            base = glm::rotate(base, glm::radians(90.0f), {0, 0, 1});
            base = glm::rotate(base, glm::radians(45.0f), {1, 0, 0});
        }
        base = glm::rotate(base, glm::radians(10.0f), {0, 1, 0});
        base = glm::scale(base, {0.8, 0.8, 0.8});
        base = glm::translate(base, {0, -0.2, 0.8});
        base = glm::rotate(base, glm::radians(180.0f), {0, 1, 0});
        base = glm::rotate(base, glm::radians(90.0f), {0, 0, 1});
    }
    return base;
}

void Paperman::get_item_resources(ItemType item, pVAO& vao,
                                  Material& material) {
    std::string resid = AssetsHub::get_item_resid(item);
    if (item == ItemType::Bow) {
        if (pullBowTime == 0) {
            resid = "bow";
        } else if (pullBowTime <= 0.3) {
            resid = "bow1";
        } else if (pullBowTime <= 0.6) {
            resid = "bow2";
        } else {
            resid = "bow3";
        }
    }
    vao = AssetsHub::get_vao(resid);
    material = AssetsHub::get_material(resid);
}

std::shared_ptr<EntityRenderer> get_entity_renderer(
    EntityInstruction instruction) {
    std::shared_ptr<EntityRenderer> e;
    if (instruction.type == "player") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = AssetsHub::get_material("player");
        e = paperman;
    } else if (instruction.type == "zombie") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = AssetsHub::get_material("zombie");
        e = paperman;
    } else if (instruction.type == "skeleton") {
        auto paperman = std::make_shared<Paperman>();
        paperman->material = AssetsHub::get_material("skeleton");
        e = paperman;
    } else if (instruction.type == "arrow") {
        e = std::make_shared<ArrowRenderer>();
    } else if (instruction.type == "item") {
        e = std::make_shared<ItemRenderer>();
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

glm::mat4 ArrowRenderer::get_model() {
    glm::mat4 base;
    base = glm::rotate(base, pitch - glm::radians(45.0f), {-1, 0, 0});
    base = glm::scale(base, {0.8, 0.8, 0.8});
    base = glm::translate(base, {0, -0.7, 0.3});
    base = glm::rotate(base, glm::radians(180.0f), {0, 1, 0});
    base = glm::rotate(base, glm::radians(90.0f), {0, 0, 1});
    return EntityRenderer::get_model() * base;
}

void ArrowRenderer::render() {
    auto shader = AssetsHub::get_shader<EntityShader>();
    auto vao = AssetsHub::get_vao("arrow");
    shader->configure(AssetsHub::get_material("arrow"), get_model());
    vao->draw();
}

void ArrowRenderer::render_depth() {
    auto shader = AssetsHub::get_shader<DepthShader>();
    auto vao = AssetsHub::get_vao("arrow");
    shader->configure(get_model());
    vao->draw();
}

void ArrowRenderer::update(EntityInstruction& i) {
    EntityRenderer::update(i);
    if (glm::length(speed) != 0) {
        facing = horizonal_angle({0, 0, 1}, speed);
        auto vSpeed = speed.y;
        auto hSpeed = glm::length(glm::vec3(speed.x, 0, speed.z));
        pitch = std::atan2(vSpeed, hSpeed);
    }
}

glm::mat4 ItemRenderer::get_model() {
    glm::mat4 base;
    base = glm::scale(base, {0.6, 0.6, 0.6});
    base = glm::translate(base, {-0.5, 0, 0});
    base = glm::rotate(base, glm::radians(90.0f), {1, 0, 0});
    base = glm::translate(base, {0, 0, -1});
    return EntityRenderer::get_model() * base;
}

void ItemRenderer::render() {
    std::string resid = AssetsHub::get_item_resid(type);
    auto shader = AssetsHub::get_shader<EntityShader>();
    auto vao = AssetsHub::get_vao(resid);
    shader->configure(AssetsHub::get_material(resid), get_model());
    vao->draw();
}

void ItemRenderer::render_depth() {
    std::string resid = AssetsHub::get_item_resid(type);
    auto shader = AssetsHub::get_shader<DepthShader>();
    auto vao = AssetsHub::get_vao(resid);
    shader->configure(get_model());
    vao->draw();
}

void ItemRenderer::update(EntityInstruction& i) {
    EntityRenderer::update(i);
    rotationSpeed = 180;
    type = (ItemType)i.state[0];
}
