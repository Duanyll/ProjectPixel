#pragma once

#include "pch.h"
#include "AssetsHub.h"

class RenderObject {
   public:
    virtual void render() = 0;
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

struct EntityInstruction;
class EntityRenderer : public RenderObject {
   public:
    glm::vec3 position{0, 0, 0};
    float facing = 0;

    virtual glm::mat4 get_model();

    glm::vec3 speed{0, 0, 0};
    float rotationSpeed = 0;

    virtual void step(float time);
    virtual void update(EntityInstruction& instruction);
};

struct PapermanMatraial : Material {
    bool is_slim;
};

class Paperman : public EntityRenderer {
   public:
    glm::mat4 get_model();
    PapermanMatraial material;

    float headYaw = 0;
    float headPitch = 0;

    enum class AnimationType {
        Standing,
        Walking,
        Running,
        ZombieWalking
    } animationType = AnimationType::Standing;
    void step(float time);
    void render();

    static PapermanMatraial get_material_preset(const std::string& key);

   protected:
    float animationTimer = 0;
    float animationTotal = 0;

   private:
    glm::mat4 get_head_model();
    glm::mat4 get_body_model();
    glm::mat4 get_larm_model();
    glm::mat4 get_rarm_model();
    glm::mat4 get_lleg_model();
    glm::mat4 get_rleg_model();
};

std::shared_ptr<EntityRenderer> get_entity_renderer(
    EntityInstruction instruction);