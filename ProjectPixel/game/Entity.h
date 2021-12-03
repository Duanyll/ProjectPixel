#pragma once

#include "pch.h"
#include "Instructions.h"
#include "Level.h"
#include "../utils/Geomentry.h"

class Entity {
   public:
    Level& level;
    Entity(Level& level, const std::string& id);

    std::string id;
    glm::vec3 pos{0, 0, 0}, speed{0, 0, 0};
    float facing = 0, rotationSpeed = 0;

    virtual std::string get_type() = 0;
    virtual std::string get_state() = 0;

    inline virtual void step_motion(float time) {}
    BoxClipping clipping = BoxClipping::None;
    void clip_speed();

    inline virtual void tick(float time){};

    virtual EntityInstruction get_instruction();
};

typedef std::shared_ptr<Entity> pEntity;

class MobEntity : public Entity {
   public:
    inline MobEntity(Level& level, const std::string& id) : Entity(level, id) {}

    virtual glm::vec3 get_bounding_box_size() = 0;
    virtual TileBoundingBox get_bounding_box();
    void step_motion(float time);
    void tick(float time);
};

class Player : public MobEntity {
   public:
    inline Player(Level& level, const std::string& id) : MobEntity(level, id) {}

    inline std::string get_type() { return "player"; }
    inline std::string get_state() { return ""; }
    inline glm::vec3 get_bounding_box_size() { return {0.5, 1.6, 0.5}; }

    int ticksToJump = 0;

    inline const static float moveSpeed = 2.5;
    inline const static float maxAcceleration = 20.0f;
    inline const static int jumpCooldown = 5;
    inline const static int jumpSpeed = 6;

    void tick(float time);
};

typedef std::shared_ptr<Player> pPlayer;