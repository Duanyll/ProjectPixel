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
    if (glm::length(posDelta) > 1e-4 && level.terrain->test_box_movement_intersection(box, posDelta, dis)) {
        box.a += glm::normalize(posDelta) * dis;
    } else {
        box.a += posDelta;
    }
    clipping = level.terrain->clip_box(box);
    pos = {box.a.x + box.s.x / 2, box.a.y, box.a.z + box.s.z / 2};
    clip_speed();
}

void MobEntity::clip_speed() {
    if ((bool)(clipping & BoxClipping::PosX) && speed.x > 0) {
        speed.x = 0;
    }
    if ((bool)(clipping & BoxClipping::NegX) && speed.x < 0) {
        speed.x = 0;
    }
    if ((bool)(clipping & BoxClipping::PosY) && speed.y > 0) {
        speed.y = 0;
    }
    if ((bool)(clipping & BoxClipping::NegY) && speed.y < 0) {
        speed.y = 0;
    }
    if ((bool)(clipping & BoxClipping::PosZ) && speed.z > 0) {
        speed.z = 0;
    }
    if ((bool)(clipping & BoxClipping::NegZ) && speed.z < 0) {
        speed.z = 0;
    }
}
