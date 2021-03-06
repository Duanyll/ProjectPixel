#pragma once

#include "pch.h"
#include "../driver/AssetsHub.h"
#include "../game/Instructions.h"

class RenderObject {
   public:
    virtual void render() = 0;
    inline virtual void render_depth() {}
};

class FullScreenQuad : public RenderObject {
   public:
    FullScreenQuad(pTexture texture);
    FullScreenQuad(const std::string& str = "default");
    pTexture texture;
    void render();
};

class Skybox : public RenderObject {
   public:
    Skybox(pCubeTexture texture);
    Skybox(const std::string& str = "default");
    pCubeTexture texture;
    void render();
};

class EntityRenderer : public RenderObject {
   public:
    glm::vec3 position{0, 0, 0};
    float facing = 0;

    virtual glm::mat4 get_model();

    glm::vec3 speed{0, 0, 0};
    float rotationSpeed = 0;

    bool highlight = false;

    virtual void step(float time);
    virtual void update(EntityInstruction& instruction);
};

class LinearAnimation {
   public:
    float value = 0;
    float speed = 1;
    void step(float time);
};

class SineAnimation {
   public:
    float value = 0;
    float amplitude = 1, peirod = 1;
    void step(float time);

   private:
    float total = 0;
};

class AccelerateAdapter {
   public:
    float value = 0;
    float maxAcc = 1;
    void step_to(float time, float target);
};

class Paperman : public EntityRenderer {
   public:
    glm::mat4 get_model();
    Material material;
    bool isSlim = false;

    float headYaw = 0;
    float headPitch = 0;

    HandAction hand = HandAction::None;
    LegAction leg = LegAction::Standing;
    ItemType handItem = ItemType::None;

    void step(float time);
    void render();
    void render_depth();

    void update(EntityInstruction& instruction);
    void set_leg_action(LegAction action);
    void set_hand_action(HandAction action);

   protected:
    SineAnimation legBase;
    AccelerateAdapter legReal{.maxAcc = 360};
    LinearAnimation handBase{.speed = 240};
    AccelerateAdapter handReal{.maxAcc = 1440};

    LinearAnimation lieBase{.speed = 180};

    float pullBowTime = 0;

   private:
    glm::mat4 get_base_rarm();

    glm::mat4 get_head_model();
    glm::mat4 get_body_model();
    glm::mat4 get_larm_model();
    glm::mat4 get_rarm_model();
    glm::mat4 get_lleg_model();
    glm::mat4 get_rleg_model();
    glm::mat4 get_item_model();

    void get_item_resources(ItemType item, pVAO& vao, Material& material);
};

std::shared_ptr<EntityRenderer> get_entity_renderer(
    EntityInstruction instruction);

class ArrowRenderer : public EntityRenderer {
   public:
    glm::mat4 get_model();

    void render();
    void render_depth();
    void update(EntityInstruction& i);

   private:
    float pitch = 0;
};

class ItemRenderer : public EntityRenderer {
   public:
    glm::mat4 get_model();

    void render();
    void render_depth();
    void update(EntityInstruction& i);

    ItemType type;
};