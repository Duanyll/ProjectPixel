#pragma once

#include "pch.h"
#include "Instructions.h"
#include "Level.h"
#include "../utils/Geomentry.h"

class Entity {
   public:
    Level& level;
    Entity(Level& level, const std::string& id, glm::vec3 pos);

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

    void apply_gravity(float time, float g);
    void apply_friction(float time, float acc);

    virtual EntityInstruction get_instruction();
};

typedef std::shared_ptr<Entity> pEntity;

enum class HurtType { Melee, Sweep, Arrow };

class MobEntity : public Entity {
   public:
    inline MobEntity(Level& level, const std::string& id, glm::vec3 pos, int hp)
        : Entity(level, id, pos), hp(hp) {}

    virtual glm::vec3 get_bounding_box_size() = 0;
    virtual TileBoundingBox get_bounding_box();
    virtual glm::vec3 get_head_pos() = 0;
    void step_motion(float time);
    void tick(float time);

    void walk(float time, glm::vec3 dir, float walkSpeed, float walkAcc);
    void turn(float time, float angle, float maxSpeed);

    int ticksToHurt = 0;
    int ticksToJump = 0;
    int ticksToRemove = 0;

    int hp = 20;

    virtual void hitback(glm::vec3 source, float strength);
    virtual bool hurt(int hits, HurtType type, std::string sender);
    virtual void jump();

    virtual void on_die();
};

class Player : public MobEntity {
   public:
    inline Player(Level& level, const std::string& id, glm::vec3 pos)
        : MobEntity(level, id, pos, 50) {}

    inline std::string get_type() { return "player"; }
    inline glm::vec3 get_bounding_box_size() { return {0.5, 1.6, 0.5}; }
    inline glm::vec3 get_head_pos() { return pos + glm::vec3(0, 1.5, 0); }

    bool isHealing = false;
    bool isAiming = false;
    bool isSweeping = false;
    bool isRunning = false;

    int ticksAttackHold = 0;
    int ticksHealHold = 0;
    int ticksToAttack = 0;
    int ticksToRegenerate = 0;

    ItemType weapon = ItemType::DiamondSword;
    std::unordered_map<ItemType, int> inventory;
    std::string mainTargetId = "";

    inline const static float moveSpeed = 2.5;
    inline const static float maxAcceleration = 20.0f;
    inline const static float maxRotationSpeed = 1080;

    void tick(float time);
    void attack();
    void sweep();
    void walk(float time, glm::vec3 direction);

    bool hurt(int hits, HurtType type, std::string sender);
    std::shared_ptr<MobEntity> get_main_target();

    void handle_heal_input(bool heal);
    void handle_attack_input(bool hold);

    EntityInstruction get_instruction();
};

typedef std::shared_ptr<Player> pPlayer;

class Zombie : public MobEntity {
   public:
    inline Zombie(Level& level, glm::vec3 pos,
                  const std::string& id = generate_unique_id("zombie"))
        : MobEntity(level, id, pos, 20) {}
    inline std::string get_type() { return "zombie"; }
    inline glm::vec3 get_bounding_box_size() { return {0.5, 1.6, 0.5}; }
    inline glm::vec3 get_head_pos() { return pos + glm::vec3(0, 1.5, 0); }

    inline const static float moveSpeed = 1.6;
    inline const static float maxAcceleration = 10.0f;
    inline const static float maxRotationSpeed = 720;

    int ticksToAttack = 0;

    void tick(float time);

    void on_die();

    EntityInstruction get_instruction();
};

class Skeleton : public MobEntity {
   public:
    inline Skeleton(Level& level, glm::vec3 pos,
                    const std::string& id = generate_unique_id("skeleton"))
        : MobEntity(level, id, pos, 20) {}
    inline std::string get_type() { return "skeleton"; }
    inline glm::vec3 get_bounding_box_size() { return {0.5, 1.6, 0.5}; }
    inline glm::vec3 get_head_pos() { return pos + glm::vec3(0, 1.5, 0); }

    inline const static float moveSpeed = 1.6;
    inline const static float maxAcceleration = 10.0f;
    inline const static int jumpCooldown = 8;
    inline const static float jumpSpeed = 6;
    inline const static float maxRotationSpeed = 720;

    bool isAiming = false;
    int ticksAttackHold = 0;
    int ticksToChangeMovement = 0;
    int ticksToShoot = 0;
    glm::vec3 randomMovement;

    void tick(float time);

    void on_die();

    EntityInstruction get_instruction();
};

class Arrow : public Entity {
   public:
    inline Arrow(Level& level, glm::vec3 pos, glm::vec3 speed,
                 const std::string& id = generate_unique_id("arrow"))
        : Entity(level, id, pos) {
        this->speed = speed;
    }
    inline std::string get_type() { return "arrow"; }

    int ticksToDecay = 600;
    bool canPickUp = false;
    std::string sender = "";

    void step_motion(float time);
    void tick(float time);
};

class Item : public Entity {
   public:
    inline Item(Level& level, ItemType type, glm::vec3 pos,
                const std::string& id = generate_unique_id("item"))
        : Entity(level, id, pos), type(type) {
        rotationSpeed = 180;
        std::uniform_real_distribution<float> angle(-180, 180);
        speed = angle_to_front(angle(level.random)) + glm::vec3{0, 2, 0};
    }
    inline std::string get_type() { return "item"; }

    ItemType type;

    int ticksToDecay = 600;

    void step_motion(float time);
    void tick(float time);

    EntityInstruction get_instruction();
};