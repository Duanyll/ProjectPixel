#include "pch.h"
#include "Processor.h"

#include "../utils/Utils.h"

LevelProcessor::LevelProcessor(LevelConfig& config)
    : config(config), level(config) {}

void LevelProcessor::start() {
    lastTime = std::chrono::steady_clock::now();
    WorkerThread::start();
}

void LevelProcessor::work() {
    auto calcTime = timer.start_tick();
    auto duration = std::min(to_float_duration(calcTime - lastTime), 0.05f);
    lastTime = calcTime;

    step_motion(duration);
    tick_entities(duration);
    handle_user_input(duration);
    clip_speed();
    emit_instructions(calcTime);

    timer.end_tick();
}

void LevelProcessor::step_motion(float duration) {
    level.entityRegistry.clear();
    for (auto& e : level.entities) {
        e.second->step_motion(duration);
        level.entityRegistry.add(e.second);
    }
}

void LevelProcessor::tick_entities(float duration) {
    for (auto& e : level.entities) {
        e.second->tick(duration);
    }
}

void LevelProcessor::handle_user_input(float duration) {
    std::unordered_map<std::string, float> o;
    std::queue<std::string> events;
    std::unordered_set<std::string> flags;
    input.poll(o, events, flags);
    auto player = std::dynamic_pointer_cast<Player>(level.entities["player1"]);
    if (player) {
        glm::vec3 controlSpeed{o["speed-x"], 0, o["speed-z"]};
        if (flags.contains("aim")) {
            player->walk(duration, controlSpeed, Player::moveSpeed * 0.6,
                         Player::maxAcceleration);
        } else if (flags.contains("run")) {
            player->walk(duration, controlSpeed, Player::moveSpeed * 1.5,
                         Player::maxAcceleration);
        } else {
            player->walk(duration, controlSpeed, Player::moveSpeed,
                         Player::maxAcceleration);
        }

        if (flags.contains("aim")) {
            player->isAiming = true;
            auto rotation = o["rotation"];
            player->turn(duration, rotation, Player::maxRotationSpeed);
        } else {
            player->isAiming = false;
            player->turn(duration,
                         horizonal_angle(player->get_front(), controlSpeed),
                         Player::maxRotationSpeed);
        }

        if (flags.contains("attack")) {
            if (player->ticksAttackHold == 0) {
                if (player->ticksToAttack == 0) {
                    player->ticksToAttack = 5;
                    auto targets =
                        level.entityRegistry.query_square_range(player->pos, 2);
                    std::shared_ptr<MobEntity> target;
                    float minAngle = 20.0f;
                    for (auto& i : targets) {
                        auto e = std::dynamic_pointer_cast<MobEntity>(i);
                        if (e) {
                            auto cur =
                                abs(horizonal_angle(player->get_front(),
                                                    e->pos - player->pos));
                            if (cur < minAngle) {
                                minAngle = cur;
                                target = e;
                            }
                        }
                    }
                    if (target) {
                        if (target->hurt(5, HurtType::Melee)) {
                            target->hitback(player->pos, 5);
                        }
                    }
                }
            }
            player->ticksAttackHold++;
        } else {
            player->ticksAttackHold = 0;
        }

        while (!events.empty()) {
            auto i = events.front();
            events.pop();
            if (i == "jump") {
                player->jump();
            }
        }
    }
}

void LevelProcessor::clip_speed() {
    for (auto& e : level.entities) {
        e.second->clip_speed();
    }
}

void LevelProcessor::emit_instructions(TimeStamp time) {
    auto instructions = std::make_unique<SceneInstruction>();
    instructions->creationTime = time;
    instructions->entities.reserve(level.entities.size());
    for (auto& i : level.entities) {
        instructions->entities.push_back(i.second->get_instruction());
    }
    output.update(instructions);
}
