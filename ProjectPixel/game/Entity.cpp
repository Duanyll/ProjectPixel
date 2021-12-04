#include "pch.h"
#include "Entity.h"

Entity::Entity(Level& level, const std::string& id) : level(level), id(id) {}

EntityInstruction Entity::get_instruction() {
    return {id, get_type(), get_state(), pos, speed, facing, rotationSpeed};
}

TileBoundingBox MobEntity::get_bounding_box() {
    auto s = get_bounding_box_size();
    return {{pos.x - s.x / 2, pos.y, pos.z - s.z / 2}, s};
}

void MobEntity::step_motion(float time) {
    facing += rotationSpeed * time;
    if (facing > 180) {
        facing -= 360;
    }
    if (facing <= -180) {
        facing += 360;
    }

    auto posDelta = speed * time;
    float dis = -1;
    auto box = get_bounding_box();
    if (glm::length(posDelta) > 1e-4 &&
        level.terrain->test_box_movement_intersection(box, posDelta, dis)) {
        box.a += glm::normalize(posDelta) * dis;
    } else {
        box.a += posDelta;
    }
    clipping = level.terrain->clip_box(box);
    pos = {box.a.x + box.s.x / 2, box.a.y, box.a.z + box.s.z / 2};
    clip_speed();
}

const float GRAVITY = 15.0f;
const float DEFAULT_FRICTION = 0.5f * GRAVITY;

void MobEntity::tick(float time) {
    Entity::tick(time);
    speed.y -= GRAVITY * time;
    if (clipping & BoxClipping::NegY) {
        glm::vec3 groundSpeed{speed.x, 0, speed.z};
        if (glm::length(groundSpeed) > DEFAULT_FRICTION * time) {
            groundSpeed -=
                glm::normalize(groundSpeed) * DEFAULT_FRICTION * time;
        } else {
            groundSpeed = {0, 0, 0};
        }
        speed.x = groundSpeed.x;
        speed.z = groundSpeed.z;
    }
}

void Entity::clip_speed() {
    if ((clipping & BoxClipping::PosX) && speed.x > 0) {
        speed.x = 0;
    }
    if ((clipping & BoxClipping::NegX) && speed.x < 0) {
        speed.x = 0;
    }
    if ((clipping & BoxClipping::PosY) && speed.y > 0) {
        speed.y = 0;
    }
    if ((clipping & BoxClipping::NegY) && speed.y < 0) {
        speed.y = 0;
    }
    if ((clipping & BoxClipping::PosZ) && speed.z > 0) {
        speed.z = 0;
    }
    if ((clipping & BoxClipping::NegZ) && speed.z < 0) {
        speed.z = 0;
    }
}

std::string Player::get_state() {
    if (clipping & BoxClipping::NegY) {
        if (glm::length(speed) > 3) {
            return "running";
        } else if (glm::length(speed) > 1) {
            return "walking";
        } else {
            return "standing";
        }
    } else {
        if (speed.y > 4) {
            return "running";
        } else if (glm::length(glm::vec3(speed.x, 0, speed.z)) > 1) {
            return "walking";
        } else {
            return "standing";
        }
    }
}

void Player::tick(float time) {
    MobEntity::tick(time);
    if (ticksToJump > 0) ticksToJump--;
}
