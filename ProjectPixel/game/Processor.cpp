#include "pch.h"
#include "Processor.h"

#include "../utils/Utils.h"

LevelProcessor::LevelProcessor(LevelConfig& config)
    : config(config), level(config) {}

void LevelProcessor::start() {
    lastTime = std::chrono::steady_clock::now();
    level.goal->on_game_start();
    WorkerThread::start();
}

void LevelProcessor::work() {
    auto calcTime = timer.start_tick();
    auto duration = std::min(to_float_duration(calcTime - lastTime), 0.05f);
    lastTime = calcTime;

    step_motion(duration);
    try_spawn_mobs();
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

void LevelProcessor::try_spawn_mobs() {
    if (!config.enableMobSpawning) return;
    std::uniform_int_distribution spawnChance(1, 25);
    if (spawnChance(level.random) > 24) {
        int mobCount = 0;
        for (auto& e : level.entities) {
            if (std::dynamic_pointer_cast<MobEntity>(e.second)) {
                mobCount++;
            }
        }

        if (mobCount < config.maxEnemies) {
            std::uniform_real_distribution<float> xCoord(
                0.0f, level.terrain->get_xsize());
            std::uniform_real_distribution<float> zCoord(
                0.0f, level.terrain->get_zsize());
            glm::vec3 center{xCoord(level.random), 0, zCoord(level.random)};
            if (!level.terrain->can_mob_spawn_on(center)) return;
            center.y = level.terrain->get_surface_height(center);
            if (glm::length(level.player->pos - center) < 8) return;

            std::uniform_real_distribution<float> dir(-180, 180);
            std::uniform_real_distribution<float> dis(1, 3);
            std::uniform_int_distribution spawnTries(2, 5);
            int count = spawnTries(level.random);
            for (int i = 0; i < count; i++) {
                auto pos = center + angle_to_front(dir(level.random)) *
                                        dis(level.random);
                if (!level.terrain->can_mob_spawn_on(pos)) continue;

                std::uniform_int_distribution spawnType(1, 4);
                std::shared_ptr<MobEntity> mob;
                if (spawnType(level.random) > 3) {
                    mob = level.add_entity<Skeleton>();
                } else {
                    mob = level.add_entity<Zombie>();
                }
                mob->pos = pos;
                mob->facing = dir(level.random);
            }
        }
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
    const auto player = level.player;
    if (player && player->hp > 0) {
        glm::vec3 controlSpeed{o["speed-x"], 0, o["speed-z"]};
        player->isAiming = flags.contains("aim");
        player->isRunning = flags.contains("run");
        player->walk(duration, controlSpeed);

        if (player->isAiming) {
            auto rotation = o["rotation"];
            player->turn(duration, rotation, Player::maxRotationSpeed);
        } else {
            player->turn(duration,
                         horizonal_angle(player->get_front(), controlSpeed),
                         Player::maxRotationSpeed);
        }

        player->handle_heal_input(flags.contains("heal"));
        player->handle_attack_input(flags.contains("attack"));

        while (!events.empty()) {
            auto i = events.front();
            events.pop();
            if (i == "jump") {
                player->jump();
            } else if (i == "slot-1") {
                player->weapon = ItemType::DiamondSword;
            } else if (i == "slot-2") {
                player->weapon = ItemType::DiamondAxe;
            } else if (i == "slot-3") {
                player->weapon = ItemType::Bow;
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
    auto ins = std::make_unique<SceneInstruction>();
    ins->creationTime = time;
    ins->entities.reserve(level.entities.size());
    for (auto& i : level.entities) {
        if (i.first != "player1" && i.second->destroyFlag) {
            ins->deletedEntities.push_back(i.second->id);
        } else {
            ins->entities.push_back(i.second->get_instruction());
        }
    }

    const auto player = level.player;
    ins->playerHP = player->hp;
    ins->playerWeapon = player->weapon;
    ins->playerLifePotion = player->inventory[ItemType::LifePotion];
    ins->playerArrow = player->inventory[ItemType::Arrow];

    ins->goalDisplay = level.goal->get_goal_display();

    for (auto& i : ins->deletedEntities) {
        level.entities.erase(i);
    }

    bool isWin = false;
    if (level.goal->should_game_stop(isWin)) {
        shouldStop = true;
        input.isEnabled = false;

        if (isWin) {
            ins->messages.push_back("Victory!");
        } else {
            ins->messages.push_back("Game over.");
        }
    }

    output.update(ins);
}
