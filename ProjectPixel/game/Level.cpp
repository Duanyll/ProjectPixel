#include "pch.h"
#include "Level.h"
#include "Entity.h"

pTerrain LevelConfig::get_terrain() {
    std::filesystem::path fullPath("levels");
    fullPath /= terrainPath;
    if (terrainType == "boxStack") {
        std::ifstream fin(fullPath);
        std::stringstream ss;
        ss << fin.rdbuf();
        return std::make_shared<BoxStackTerrain>(xSize, zSize, ss.str());
    }
}

Level::Level(LevelConfig& config)
    : config(config), entityRegistry(4, config.xSize, config.zSize) {
    terrain = config.get_terrain();
    player = add_entity<Player>("player1");
    player->pos = config.playerSpawnPos;
    player->inventory[ItemType::Arrow] = 16;

    for (auto& pos : config.mobs["zombie"]) {
        auto zombie = add_entity<Zombie>();
        zombie->pos = pos;
    }

    for (auto& pos : config.mobs["skeleton"]) {
        auto skeleton = add_entity<Skeleton>();
        skeleton->pos = pos;
    }

    if (config.goal == "clear") {
        goal = std::make_shared<ClearGoal>(*this);
    } else if (config.goal == "speedRun") {
        goal = std::make_shared<SpeedRunGoal>(*this);
    }
}

bool SpeedRunGoal::should_game_stop(bool& isWin) {
    if (level.player->hp <= 0) {
        isWin = false;
        return true;
    }
    if (glm::length(level.player->pos - level.config.destinationPos) <
        level.config.destinationRange) {
        isWin = true;
        return true;
    }
    return false;
}

std::string SpeedRunGoal::get_goal_display() {
    return duration_to_msms(std::chrono::steady_clock::now() - startTime);
}

void SpeedRunGoal::on_game_start() {
    startTime = std::chrono::steady_clock::now();
}

void SpeedRunGoal::on_mob_die(std::string type) {}

bool ClearGoal::should_game_stop(bool& isWin) {
    if (level.player->hp <= 0) {
        isWin = false;
        return true;
    }
    if (remainEnemies == 0) {
        isWin = true;
        return true;
    }
    return false;
}

std::string ClearGoal::get_goal_display() {
    return std::format(
        "{} | {}", remainEnemies,
        duration_to_msms(std::chrono::steady_clock::now() - startTime));
}

void ClearGoal::on_game_start() {
    startTime = std::chrono::steady_clock::now();

    remainEnemies = 0;
    for (auto& [id, e] : level.entities) {
        if (std::dynamic_pointer_cast<MobEntity>(e)) {
            if (id != "player1") remainEnemies++;
        }
    }
}

void ClearGoal::on_mob_die(std::string type) {
    if (type == "zombie" || type == "skeleton") {
        remainEnemies--;
    }
}
