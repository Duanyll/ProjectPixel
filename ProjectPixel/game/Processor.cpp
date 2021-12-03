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
    auto duration = to_float_duration(calcTime - lastTime);
    lastTime = calcTime;

    step_motion(duration);
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

void LevelProcessor::handle_user_input(float duration) {
    std::unordered_map<std::string, float> o;
    std::queue<std::string> events;
    input.collect(o, events);
    auto player = std::dynamic_pointer_cast<Player>(level.entities["player1"]);
    if (player) {
        glm::vec3 control_speed{o["speed-x"], 0, o["speed-z"]};
        if (glm::length(control_speed) > 0.01) {
            control_speed = glm::normalize(control_speed) * Player::moveSpeed;
            // TODO: 更好的玩家速度操控
            player->speed = control_speed;
        } else {
            player->speed = {0, 0, 0};
        }
    }
}

void LevelProcessor::clip_speed() {
    for (auto& i : level.entities) {
        auto e = std::dynamic_pointer_cast<MobEntity>(i.second);
        if (e) e->clip_speed();
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
