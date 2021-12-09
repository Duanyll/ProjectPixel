#pragma once

#include "pch.h"
#include "Instructions.h"
#include "Level.h"
#include "../utils/Geomentry.h"

class Entity {
   public:
    Level& level;
    Entity(Level& level, const std::string& id);

    bool destroyFlag = false;

    std::string id;
    glm::vec3 pos{0, 0, 0}, speed{0, 0, 0};
    float facing = 0, rotationSpeed = 0;
    glm::vec3 get_front();

    virtual std::string get_type() = 0;

    inline virtual void step_motion(float time) {}
    BoxClipping clipping = BoxClipping::None;
    void clip_speed();

    inline virtual void tick(float time) {}

    virtual EntityInstruction get_instruction();
};

typedef std::shared_ptr<Entity> pEntity;

enum class HurtType { Melee, Arrow };

class MobEntity : public Entity {
   public:
    inline MobEntity(Level& level, const std::string& id) : Entity(level, id) {}

    virtual glm::vec3 get_bounding_box_size() = 0;
    virtual TileBoundingBox get_bounding_box();
    virtual glm::vec3 get_head_pos() = 0;
    void step_motion(float time);
    void tick(float time);

    void walk(float time, glm::vec3 dir, float walkSpeed, float walkAcc);
    void turn(float time, float angle, float maxSpeed);

    int ticksToHurt = 0;

    virtual void hitback(glm::vec3 source, float strength);
    virtual bool hurt(int hits, HurtType type);
};

class Player : public MobEntity {
   public:
    inline Player(Level& level, const std::string& id) : MobEntity(level, id) {}

    inline std::string get_type() { return "player"; }
    inline glm::vec3 get_bounding_box_size() { return {0.5, 1.6, 0.5}; }
    inline glm::vec3 get_head_pos() { return pos + glm::vec3(0, 1.5, 0); }

    bool isAiming = false;
    bool isSweeping = false;
    int ticksToJump = 0;
    int ticksAttackHold = 0;
    int ticksToAttack = 0;

    Item weapon = Item::DiamondSword;

    inline const static float moveSpeed = 2.5;
    inline const static float maxAcceleration = 20.0f;
    inline const static int jumpCooldown = 5;
    inline const static float jumpSpeed = 6;
    inline const static float maxRotationSpeed = 720;

    void tick(float time);
    void jump();
    void attack();
    void sweep();

    void handle_attack_input(bool hold);

    EntityInstruction get_instruction();
};

typedef std::shared_ptr<Player> pPlayer;

class Zombie : public MobEntity {
   public:
    inline Zombie(Level& level,
                  const std::string& id = generate_unique_id("zombie"))
        : MobEntity(level, id) {}
    inline std::string get_type() { return "zombie"; }
    inline glm::vec3 get_bounding_box_size() { return {0.5, 1.6, 0.5}; }
    inline glm::vec3 get_head_pos() { return pos + glm::vec3(0, 1.5, 0); }

    inline const static float moveSpeed = 1.6;
    inline const static float maxAcceleration = 10.0f;
    inline const static int jumpCooldown = 8;
    inline const static float jumpSpeed = 6;
    inline const static float maxRotationSpeed = 720;

    int ticksToJump = 0;
    int ticksToAttack = 0;

    void tick(float time);
    void jump();

    EntityInstruction get_instruction();
};

class Arrow : public Entity {
   public:
    inline Arrow(Level& level,
                 const std::string& id = generate_unique_id("arrow"))
        : Entity(level, id) {}
    inline std::string get_type() { return "arrow"; }

    int ticksToDecay = 600;

    void step_motion(float time);
    void tick(float time);
};