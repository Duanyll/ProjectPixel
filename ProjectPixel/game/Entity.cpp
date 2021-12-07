#include "pch.h"
#include "Entity.h"

Entity::Entity(Level& level, const std::string& id) : level(level), id(id) {}

glm::vec3 Entity::get_front() {
    return {sin(-glm::radians(facing)), 0, cos(glm::radians(facing))};
}

EntityInstruction Entity::get_instruction() {
    return {id, get_type(), {}, pos, speed, facing, rotationSpeed};
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

    auto nearby = level.entityRegistry.query_square_range(pos, 1);
    for (auto& i : nearby) {
        if (i.get() == this) continue;
        auto e = std::dynamic_pointer_cast<MobEntity>(i);
        if (e) {
            auto dis = glm::length(pos - e->pos);
            if (dis >= 0.01 && dis <= 1) {
                auto dir = glm::normalize(pos - e->pos);
                auto ratio = 0.5f / (dis * dis);
                ratio = std::min(ratio, 3.0f);
                speed += dir * ratio;
            }
        }
    }
}

void MobEntity::walk(float time, glm::vec3 dir, float walkSpeed,
                     float walkAcc) {
    glm::vec3 finalSpeed;
    if (glm::length(glm::vec3(dir.x, 0, dir.z)) > 0.01) {
        finalSpeed = glm::normalize(glm::vec3(dir.x, 0, dir.z)) * walkSpeed;
    } else {
        finalSpeed = {0, 0, 0};
    }
    auto deltaSpeed = finalSpeed - glm::vec3(speed.x, 0, speed.z);
    auto acc = walkAcc * time;
    if (!(clipping & BoxClipping::NegY)) {
        acc *= 0.2;
    }
    if (glm::length(deltaSpeed) <= acc) {
        speed.x = finalSpeed.x;
        speed.z = finalSpeed.z;
    } else {
        speed += glm::normalize(deltaSpeed) * acc;
    }
}

void MobEntity::turn(float time, float angle, float maxSpeed) {
    auto rotationAcc = maxSpeed * time;
    if (!std::isnan(angle) && std::fabsf(angle) > 1) {
        if (angle > rotationAcc) {
            rotationSpeed = maxSpeed;
        } else if (angle < -rotationAcc) {
            rotationSpeed = -maxSpeed;
        } else {
            rotationSpeed = 0;
            facing += angle;
        }
    } else {
        rotationSpeed = 0;
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

void Player::tick(float time) {
    MobEntity::tick(time);
    if (ticksToJump > 0) ticksToJump--;
}

void Player::jump() {
    if (ticksToJump == 0 && (clipping & BoxClipping::NegY)) {
        speed.y = jumpSpeed;
    }
}

EntityInstruction Player::get_instruction() {
    auto i = Entity::get_instruction();
    if (isAiming) {
        i.state[1] = (char)HandAction::Holding;
    } else {
        i.state[1] = (char)HandAction::None;
    }
    i.state[2] = (char)Item::DiamondSword;
    if (clipping & BoxClipping::NegY) {
        if (glm::length(speed) > 3) {
            i.state[0] = (char)LegAction::Running;
        } else if (glm::length(speed) > 1) {
            i.state[0] = (char)LegAction::Walking;
        } else {
            i.state[0] = (char)LegAction::Standing;
        }
    } else {
        if (speed.y > 4) {
            i.state[0] = (char)LegAction::Running;
        } else if (glm::length(glm::vec3(speed.x, 0, speed.z)) > 1) {
            i.state[0] = (char)LegAction::Walking;
        } else {
            i.state[0] = (char)LegAction::Standing;
        }
    }
    return i;
}

void Zombie::tick(float time) {
    MobEntity::tick(time);
    auto it = level.entities.find("player1");
    if (it != level.entities.end()) {
        auto player = std::dynamic_pointer_cast<Player>(it->second);
        auto dis = glm::length(player->pos - pos);
        if (dis > 0.5 && dis < 8 &&
            level.terrain->test_connectivity(get_head_pos(),
                                             player->get_head_pos())) {
            walk(time, player->pos - pos, moveSpeed, maxAcceleration);
            turn(time, horizonal_angle(get_front(), player->pos - pos),
                 maxRotationSpeed);
            if (player->pos.y > pos.y + 0.5 && (clipping & BoxClipping::NegY) &&
                (clipping & (BoxClipping::PosX | BoxClipping::PosZ |
                             BoxClipping::NegX | BoxClipping::NegZ))) {
                jump();
            }
        } else {
            walk(time, {0, 0, 0}, moveSpeed, maxAcceleration);
            turn(time, 0, maxRotationSpeed);
        }
    }
}

void Zombie::jump() {
    if (ticksToJump == 0 && (clipping & BoxClipping::NegY)) {
        speed.y = jumpSpeed;
    }
}

EntityInstruction Zombie::get_instruction() {
    auto i = Entity::get_instruction();
    i.state[1] = (char)HandAction::ZombieHanging;
    i.state[2] = (char)Item::DiamondAxe;
    if (glm::length(speed) > 1) {
        i.state[0] = (char)LegAction::Walking;
    } else {
        i.state[0] = (char)LegAction::Standing;
    }
    return i;
}
