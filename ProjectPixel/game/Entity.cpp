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

    if (ticksToHurt > 0) ticksToHurt--;
    if (ticksToJump > 0) ticksToJump--;
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
        rotationSpeed = glm::clamp(angle / time, -maxSpeed, maxSpeed);
    } else {
        rotationSpeed = 0;
    }
}

void MobEntity::hitback(glm::vec3 source, float strength) {
    auto dir = (pos - source);
    speed += strength * glm::normalize(glm::vec3(dir.x, 0, dir.z)) +
             glm::vec3(0, 2, 0);
}

bool MobEntity::hurt(int hits, HurtType type) {
    if (ticksToHurt > 0) return false;
    ticksToHurt = 5;
    return true;
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

void MobEntity::jump() {
    if (ticksToJump == 0 && (clipping & BoxClipping::NegY)) {
        speed.y = 6;
        ticksToJump = 5;
    }
}

void Player::tick(float time) {
    MobEntity::tick(time);
    if (ticksToAttack > 0) ticksToAttack--;
}

void Player::attack() {
    if (ticksToAttack == 0) {
        if (weapon == Item::DiamondAxe) {
            ticksToAttack = 12;
        } else {
            ticksToAttack = 6;
        }
        auto targets = level.entityRegistry.query_square_range(pos, 3);
        std::shared_ptr<MobEntity> target;
        float minAngle = 20.0f;
        for (auto& i : targets) {
            if (i->id == id) continue;
            auto e = std::dynamic_pointer_cast<MobEntity>(i);
            if (e) {
                if (glm::length(e->pos - pos) > 2) continue;
                auto cur = abs(horizonal_angle(get_front(), e->pos - pos));
                if (cur < minAngle) {
                    minAngle = cur;
                    target = e;
                }
            }

            auto arrow = std::dynamic_pointer_cast<Arrow>(i);
            if (arrow && arrow->clipping == BoxClipping::None) {
                if (glm::length(arrow->pos - pos) > 3) continue;
                auto cur = abs(horizonal_angle(get_front(), arrow->pos - pos));
                if (abs(cur) <= 30) {
                    auto newSpeed = glm::normalize(arrow->pos - pos) * 9.0f;
                    arrow->speed.x = newSpeed.x;
                    arrow->speed.z = newSpeed.z;
                }
            }
        }
        if (target) {
            float baseHurt, baseHitback = 5;
            if (weapon == Item::DiamondSword) {
                baseHurt = 5;
            } else if (weapon == Item::DiamondAxe) {
                baseHurt = 10;
            } else {
                baseHurt = 2;
            }
            if (!isAiming) {
                baseHurt *= 0.8;
                baseHitback *= 0.8;
            }
            if (speed.y < 0) {
                baseHurt *= 1.2;
                baseHitback *= 1.2;
            }
            if (target->hurt(baseHurt, HurtType::Melee)) {
                target->hitback(pos, baseHitback);
            }
        }
    }
}

void Player::sweep() {
    ticksToAttack = 0;
    auto targets = level.entityRegistry.query_square_range(pos, 2.5);
    float minAngle = 30.0f;
    for (auto& i : targets) {
        if (i->id == id) continue;
        auto e = std::dynamic_pointer_cast<MobEntity>(i);
        if (e) {
            if (glm::length(e->pos - pos) > 2.5) continue;
            auto cur = abs(horizonal_angle(get_front(), e->pos - pos));
            if (cur < minAngle) {
                if (e->hurt(3, HurtType::Melee)) {
                    e->hitback(pos, 5.5);
                }
            }
        }
    }
}

void Player::handle_attack_input(bool hold) {
    if (hold) {
        if (weapon != Item::Bow) {
            if (ticksAttackHold == 0) {
                attack();
            } else if (weapon == Item::DiamondSword && ticksAttackHold > 2 &&
                       ticksAttackHold < 20 &&
                       abs(rotationSpeed) >= 0.5 * Player::maxRotationSpeed) {
                isSweeping = true;
                sweep();
            } else {
                isSweeping = false;
            }
        }
        ticksAttackHold++;
    } else {
        if (weapon == Item::Bow) {
            if (ticksAttackHold > 3) {
                float arrowSpeed = std::clamp(ticksAttackHold, 3, 12) * 1.25;
                auto arrow =
                    level.add_entity<Arrow>(generate_unique_id("arrow"));
                arrow->pos = pos + glm::vec3{0, 1, 0} + get_front() * 0.5f;
                arrow->speed = get_front() * arrowSpeed + glm::vec3{0, 2, 0};
            }
        }
        ticksAttackHold = 0;
        isSweeping = false;
    }
}

EntityInstruction Player::get_instruction() {
    auto i = Entity::get_instruction();
    if (isSweeping) {
        i.state[1] = (char)HandAction::Sweeping;
    } else if (ticksToAttack > 0 ||
               (weapon == Item::Bow && ticksAttackHold > 0)) {
        i.state[1] = (char)HandAction::Attacking;
    } else if (isAiming) {
        i.state[1] = (char)HandAction::Holding;
    } else {
        i.state[1] = (char)HandAction::None;
    }
    i.state[2] = (char)weapon;
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
    if (ticksToAttack > 0) ticksToAttack--;
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

            if (ticksToAttack == 25) {
                if (dis < 1.5) {
                    if (player->hurt(5, HurtType::Melee)) {
                        player->hitback(pos, 4);
                    }
                } else {
                    ticksToAttack = 15;
                }
            }
            if (dis < 1.8 && ticksToAttack == 0) {
                ticksToAttack = 30;
            }
        } else {
            walk(time, {0, 0, 0}, moveSpeed, maxAcceleration);
            turn(time, 0, maxRotationSpeed);
        }
    }
}

EntityInstruction Zombie::get_instruction() {
    auto i = Entity::get_instruction();
    if (ticksToAttack >= 20) {
        i.state[1] = (char)HandAction::ZombieAttacking;
    } else {
        i.state[1] = (char)HandAction::ZombieHanging;
    }
    i.state[2] = (char)Item::DiamondAxe;
    if (glm::length(speed) > 1) {
        i.state[0] = (char)LegAction::Walking;
    } else {
        i.state[0] = (char)LegAction::Standing;
    }
    return i;
}

void Arrow::step_motion(float time) {
    if (glm::length(speed) == 0) return;
    facing = horizonal_angle({0, 0, 1}, speed);
    rotationSpeed = 0;

    auto posDelta = speed * time;
    float dis = -1;
    if (glm::length(posDelta) > 1e-4 &&
        level.terrain->test_line_intersection(pos, posDelta, dis)) {
        pos += glm::normalize(posDelta) * dis;
    } else {
        pos += posDelta;
    }
    clipping = level.terrain->clip_point(pos);
    clip_speed();
}

void Arrow::tick(float time) {
    if (clipping != BoxClipping::None) {
        speed = {0, 0, 0};
    } else {
        speed.y -= GRAVITY * time * 0.4;
    }

    if (ticksToDecay > 0) {
        ticksToDecay--;
    } else {
        destroyFlag = true;
        return;
    }

    if (glm::length(speed) != 0) {
        auto posDelta = speed * time;
        auto targets = level.entityRegistry.query_square_range(pos, 4);
        std::shared_ptr<MobEntity> target;
        float minDis = glm::length(posDelta);
        for (auto& i : targets) {
            if (i->id == id) continue;
            auto e = std::dynamic_pointer_cast<MobEntity>(i);
            if (e) {
                auto box = e->get_bounding_box();
                if (box.test_point_inside(pos)) {
                    target = e;
                    break;
                }
                float d = -1;
                if (box.test_line_intersection(pos, glm::normalize(posDelta),
                                               d)) {
                    if (d < minDis) {
                        minDis = d;
                        target = e;
                    }
                }
            }
        }

        if (target) {
            target->hurt(3 + 0.2 * glm::length(speed), HurtType::Arrow);
            target->hitback(target->pos - speed, 0.4 * glm::length(speed));
            destroyFlag = true;
            return;
        }
    }
}

void Skeleton::tick(float time) {
    MobEntity::tick(time);
    if (ticksToChangeMovement > 0) ticksToChangeMovement--;
    if (ticksToShoot > 0) ticksToShoot--;
    auto it = level.entities.find("player1");
    if (it != level.entities.end()) {
        auto player = std::dynamic_pointer_cast<Player>(it->second);
        auto dis = glm::length(player->pos - pos);
        if (dis > 0.5 && dis < 12) {
            auto spd = moveSpeed;
            if (isAiming) {
                spd *= 0.8;
            }
            if (dis < 4) {
                walk(time, pos - player->pos, spd, maxAcceleration);
            } else if (dis > 8) {
                walk(time, player->pos - pos, spd, maxAcceleration);
            } else {
                if (ticksToChangeMovement == 0) {
                    std::uniform_real_distribution<float> angle(-180, 180);
                    randomMovement = angle_to_front(angle(level.random));
                    std::uniform_int_distribution<int> ticks(5, 20);
                    ticksToChangeMovement = ticks(level.random);
                }
                walk(time, randomMovement, spd, maxAcceleration);
            }
            turn(time, horizonal_angle(get_front(), player->pos - pos),
                 maxRotationSpeed);
            if (player->pos.y > pos.y + 0.5 && (clipping & BoxClipping::NegY) &&
                (clipping & (BoxClipping::PosX | BoxClipping::PosZ |
                             BoxClipping::NegX | BoxClipping::NegZ))) {
                jump();
            }

            bool canSee = level.terrain->test_connectivity(
                get_head_pos(), player->get_head_pos());
            if (canSee && ticksAttackHold < 15 && dis > 2 &&
                ticksToShoot == 0) {
                isAiming = true;
                ticksAttackHold++;
            } else {
                if (ticksAttackHold > 3) {
                    float arrowSpeed =
                        std::clamp(ticksAttackHold, 3, 12) * 1.25;
                    auto arrow =
                        level.add_entity<Arrow>(generate_unique_id("arrow"));
                    arrow->pos = pos + glm::vec3{0, 1, 0} + get_front() * 0.5f;
                    arrow->speed =
                        glm::normalize(player->pos - pos) * arrowSpeed +
                        glm::vec3{0, 2, 0};
                    arrow->ticksToDecay = 150;
                    ticksToShoot = 20;
                }
                ticksAttackHold = 0;
                isAiming = false;
            }
        } else {
            walk(time, {0, 0, 0}, moveSpeed, maxAcceleration);
            turn(time, 0, maxRotationSpeed);
        }
    }
}

EntityInstruction Skeleton::get_instruction() {
    auto i = Entity::get_instruction();
    if (ticksAttackHold) {
        i.state[1] = (char)HandAction::Attacking;
    } else if (isAiming) {
        i.state[1] = (char)HandAction::Holding;
    } else {
        i.state[1] = (char)HandAction::None;
    }
    i.state[2] = (char)Item::Bow;
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
