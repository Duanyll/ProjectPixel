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
    for (auto& e : level.entities) {
        e.second->step_motion(duration);
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
    input.collect(o, events);
    auto player = std::dynamic_pointer_cast<Player>(level.entities["player1"]);
    if (player) {
        glm::vec3 controlSpeed{o["speed-x"], 0, o["speed-z"]};
        glm::vec3 finalSpeed;
        if (glm::length(controlSpeed) > 0.01) {
            finalSpeed = glm::normalize(controlSpeed) * Player::moveSpeed;
        } else {
            finalSpeed = {0, 0, 0};
        }
        auto deltaSpeed =
            finalSpeed - glm::vec3(player->speed.x, 0, player->speed.z);
        auto acc = Player::maxAcceleration * duration;
        if (glm::length(deltaSpeed) <= acc) {
            player->speed.x = finalSpeed.x;
            player->speed.z = finalSpeed.z;
        } else {
            player->speed += glm::normalize(deltaSpeed) * acc;
        }

        auto rotation = o["rotation"];
        auto rotationAcc = Player::maxRotationSpeed * duration;
        if (!std::isnan(rotation) && std::fabsf(rotation) > 1) {
            if (rotation > rotationAcc) {
                player->rotationSpeed = Player::maxRotationSpeed;
            } else if (rotation < -rotationAcc) {
                player->rotationSpeed = -Player::maxRotationSpeed;
            } else {
                player->rotationSpeed = 0;
                player->facing += rotation;
            }
        } else {
            player->rotationSpeed = 0;
        }

        while (!events.empty()) {
            auto i = events.front();
            events.pop();
            if (i == "jump") {
                if (player->ticksToJump == 0 &&
                    (player->clipping & BoxClipping::NegY)) {
                    player->speed.y = Player::jumpSpeed;
                }
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
