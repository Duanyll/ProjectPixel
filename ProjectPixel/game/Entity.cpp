#include "pch.h"
#include "Entity.h"

Entity::Entity(Level& level, const std::string& id) : level(level), id(id) {}

EntityInstruction Entity::get_instruction() {
    return {id, get_type(), get_state(), pos, speed, facing, rotationSpeed};
}

void MobEntity::step_motion(float time) {
    pos += speed * time;
    facing += rotationSpeed * time;
    if (facing > 180) {
        facing -= 360;
    }
    if (facing <= -180) {
        facing += 360;
    }

    // TODO: 实体与地形碰撞
}