#include "pch.h"
#include "Entity.h"

Entity::Entity(Level& level, const std::string& id) : level(level), id(id) {}

glm::vec3 Entity::get_front() {
    return {sin(-glm::radians(facing)), 0, cos(glm::radians(facing))};
}

void Entity::apply_gravity(float time, float g) { speed.y -= g * time; }

void Entity::apply_friction(float time, float acc) {
    if (clipping & BoxClipping::NegY) {
        glm::vec3 groundSpeed{speed.x, 0, speed.z};
        if (glm::length(groundSpeed) > acc * time) {
            groundSpeed -= glm::normalize(groundSpeed) * acc * time;
        } else {
            groundSpeed = {0, 0, 0};
        }
        speed.x = groundSpeed.x;
        speed.z = groundSpeed.z;
    }
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
    apply_gravity(time, GRAVITY);
    apply_friction(time, DEFAULT_FRICTION);

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

    if (hp <= 0) {
        if (ticksToRemove > 0) {
            ticksToRemove--;
        } else {
            destroyFlag = true;
            return;
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

bool MobEntity::hurt(int hits, HurtType type, std::string sender) {
    if (hp <= 0) return false;
    if (ticksToHurt > 0) return false;
    ticksToHurt = 5;

    level.goal->on_mob_hurt(sender, id, type, hits);
    hp -= hits;
    if (hp <= 0) {
        ticksToRemove = 20;
        hp = 0;
        on_die();
    }

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
    if (hp <= 0) return;
    if (ticksToAttack > 0) ticksToAttack--;
    if (ticksToRegenerate > 0) {
        ticksToRegenerate--;
    } else {
        if (hp < 20) {
            ticksToRegenerate = 60;
        } else {
            ticksToRegenerate = 100;
        }
        if (hp < 28) {
            hp += 2;
        }
    }
}

void Player::attack() {
    if (ticksToAttack == 0) {
        if (weapon == ItemType::DiamondAxe) {
            ticksToAttack = 7;
        } else {
            ticksToAttack = 4;
        }
        auto targets = level.entityRegistry.query_square_range(pos, 3);
        std::shared_ptr<MobEntity> target;
        float minAngle = 20.0f;
        if (isRunning) minAngle *= 1.5;
        for (auto& i : targets) {
            if (i->id == id) continue;
            auto e = std::dynamic_pointer_cast<MobEntity>(i);
            if (e) {
                if (glm::length(e->pos - pos) > 2.2) continue;
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
            if (weapon == ItemType::DiamondSword) {
                baseHurt = 4;
            } else if (weapon == ItemType::DiamondAxe) {
                baseHurt = 7;
            } else {
                baseHurt = 2;
            }
            if (!isAiming) {
                baseHurt *= 0.8;
                baseHitback *= 0.8;
            }
            if (speed.y < 0) {
                baseHurt *= 1.5;
                baseHitback *= 1.2;
            }
            if (target->hurt(baseHurt, HurtType::Melee, id)) {
                target->hitback(pos, baseHitback);
            }
        }
    }
}

void Player::sweep() {
    ticksToAttack = 0;
    auto targets = level.entityRegistry.query_square_range(pos, 2.8);
    float minAngle = 30.0f;
    for (auto& i : targets) {
        if (i->id == id) continue;
        auto e = std::dynamic_pointer_cast<MobEntity>(i);
        if (e) {
            if (glm::length(e->pos - pos) > 2.8) continue;
            auto cur = abs(horizonal_angle(get_front(), e->pos - pos));
            if (cur < minAngle) {
                if (e->hurt(3, HurtType::Sweep, id)) {
                    e->hitback(pos, 5.5);
                }
            }
        }
    }
}

void Player::walk(float time, glm::vec3 direction) {
    auto spd = moveSpeed;
    if (isHealing) {
        spd *= 0.6;
    } else if (isAiming) {
        spd *= 0.8;
    } else if (isRunning) {
        spd *= 1.5;
    }
    MobEntity::walk(time, direction, spd, maxAcceleration);
}

bool Player::hurt(int hits, HurtType type, std::string sender) {
    if (isHealing) {
        hits = hits * 3 / 2;
    } else if (isAiming) {
        hits = hits * 3 / 4;
    } else if (isRunning) {
        hits = hits * 4 / 3;
    }
    return MobEntity::hurt(hits, type, sender);
}

void Player::handle_heal_input(bool hold) {
    if (hold && inventory[ItemType::LifePotion] > 0 && hp < 50) {
        ticksHealHold++;
        if (ticksHealHold < 20) {
            isHealing = true;
        } else if (ticksHealHold == 20) {
            hp = std::min(hp + 10, 50);
            inventory[ItemType::LifePotion] -= 1;
            isHealing = false;
        }
    } else {
        isHealing = false;
        ticksHealHold = 0;
    }
}

void Player::handle_attack_input(bool hold) {
    if (!isHealing && hold &&
        (weapon != ItemType::Bow || inventory[ItemType::Arrow] > 0)) {
        if (weapon != ItemType::Bow) {
            if (ticksAttackHold == 0) {
                attack();
            } else if (weapon == ItemType::DiamondSword &&
                       ticksAttackHold > 2 && ticksAttackHold < 20 &&
                       abs(rotationSpeed) >= 0.3 * Player::maxRotationSpeed) {
                isSweeping = true;
                sweep();
            } else {
                isSweeping = false;
            }
        }
        ticksAttackHold++;
    } else {
        if (weapon == ItemType::Bow) {
            if (ticksAttackHold > 3 && inventory[ItemType::Arrow] > 0) {
                float arrowSpeed = std::clamp(ticksAttackHold, 3, 12) * 1.25;
                auto arrow = level.add_entity<Arrow>();
                arrow->pos = pos + glm::vec3{0, 1, 0} + get_front() * 0.5f;
                arrow->speed = get_front() * arrowSpeed + glm::vec3{0, 2, 0};
                arrow->canPickUp = true;
                arrow->sender = id;
                inventory[ItemType::Arrow] -= 1;
            }
        }
        ticksAttackHold = 0;
        isSweeping = false;
    }
}

EntityInstruction Player::get_instruction() {
    auto i = Entity::get_instruction();
    if (isHealing) {
        i.state[1] = (char)HandAction::Holding;
    } else if (isSweeping) {
        i.state[1] = (char)HandAction::Sweeping;
    } else if (ticksToAttack > 0 ||
               (weapon == ItemType::Bow && ticksAttackHold > 0)) {
        i.state[1] = (char)HandAction::Attacking;
    } else if (isAiming) {
        i.state[1] = (char)HandAction::Holding;
    } else {
        i.state[1] = (char)HandAction::None;
    }
    if (isHealing) {
        i.state[2] = (char)ItemType::LifePotion;
    } else {
        i.state[2] = (char)weapon;
    }
    if (hp <= 0) {
        i.state[0] = (char)LegAction::Lying;
    } else if (clipping & BoxClipping::NegY) {
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
    if (hp <= 0) return;
    if (ticksToAttack > 0) ticksToAttack--;
    const auto player = level.player;
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
                if (player->hurt(5, HurtType::Melee, id)) {
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

void Zombie::on_die() {
    MobEntity::on_die();
    std::uniform_int_distribution<int> chance(1, 5);
    if (chance(level.random) > 3) {
        auto item = level.add_entity<Item>(ItemType::LifePotion);
        item->pos = pos;
        std::uniform_real_distribution<float> angle(-180, 180);
        item->speed = angle_to_front(angle(level.random)) + glm::vec3{0, 2, 0};
    }
}

EntityInstruction Zombie::get_instruction() {
    auto i = Entity::get_instruction();
    if (ticksToAttack >= 20) {
        i.state[1] = (char)HandAction::ZombieAttacking;
    } else {
        i.state[1] = (char)HandAction::ZombieHanging;
    }
    i.state[2] = (char)ItemType::DiamondAxe;
    if (hp <= 0) {
        i.state[0] = (char)LegAction::Lying;
    } else if (glm::length(speed) > 1) {
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
            target->hurt(3 + 0.2 * glm::length(speed), HurtType::Arrow, sender);
            target->hitback(target->pos - speed, 0.4 * glm::length(speed));
            destroyFlag = true;
            return;
        }
    } else if (canPickUp) {
        const auto player = level.player;
        auto dis = glm::length(player->pos - pos);
        if (dis < 1) {
            player->inventory[ItemType::Arrow] += 1;
            destroyFlag = true;
            return;
        }
    }
}

void Skeleton::tick(float time) {
    MobEntity::tick(time);
    if (hp <= 0) return;
    if (ticksToChangeMovement > 0) ticksToChangeMovement--;
    if (ticksToShoot > 0) ticksToShoot--;
    const auto player = level.player;
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

        bool canSee = level.terrain->test_connectivity(get_head_pos(),
                                                       player->get_head_pos());
        if (canSee && ticksAttackHold < 15 && dis > 2 && ticksToShoot == 0) {
            isAiming = true;
            ticksAttackHold++;
        } else {
            if (ticksAttackHold > 3) {
                float arrowSpeed = std::clamp(ticksAttackHold, 3, 12) * 1.25;
                auto arrow = level.add_entity<Arrow>();
                arrow->pos = pos + glm::vec3{0, 1, 0} + get_front() * 0.5f;
                arrow->speed = glm::normalize(player->pos - pos) * arrowSpeed +
                               glm::vec3{0, 2, 0};
                arrow->ticksToDecay = 150;
                arrow->sender = id;
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

void Skeleton::on_die() {
    MobEntity::on_die();

    std::uniform_int_distribution<int> chance(1, 5);
    if (chance(level.random) > 2) {
        auto item = level.add_entity<Item>(ItemType::LifePotion);
        item->pos = pos;
        std::uniform_real_distribution<float> angle(-180, 180);
        item->speed = angle_to_front(angle(level.random)) + glm::vec3{0, 2, 0};
    }

    std::uniform_int_distribution<int> arrowDis(2, 5);
    int arrowCount = arrowDis(level.random);
    for (int i = 1; i <= arrowCount; i++) {
        auto item = level.add_entity<Item>(ItemType::Arrow);
        item->pos = pos;
        std::uniform_real_distribution<float> angle(-180, 180);
        item->speed = angle_to_front(angle(level.random)) + glm::vec3{0, 2, 0};
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
    i.state[2] = (char)ItemType::Bow;
    if (hp <= 0) {
        i.state[0] = (char)LegAction::Lying;
    } else if (clipping & BoxClipping::NegY) {
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

void Item::step_motion(float time) {
    facing += rotationSpeed * time;
    if (facing > 180) {
        facing -= 360;
    }
    if (facing <= -180) {
        facing += 360;
    }

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

void Item::tick(float time) {
    if (ticksToDecay > 0) {
        ticksToDecay--;
    } else {
        destroyFlag = true;
    }

    apply_gravity(time, GRAVITY);
    apply_friction(time, DEFAULT_FRICTION);

    const auto player = level.player;
    auto dis = glm::length(player->pos - pos);
    if (dis < 0.5) {
        player->inventory[type] += 1;
        destroyFlag = true;
        return;
    } else if (dis < 1.5) {
        speed += glm::normalize(player->pos - pos) * 2.0f;
    }
}

EntityInstruction Item::get_instruction() {
    auto i = Entity::get_instruction();
    i.state[0] = (char)type;
    return i;
}
