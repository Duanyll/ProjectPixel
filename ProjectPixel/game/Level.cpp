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

Level::Level(LevelConfig& config) : config(config), entityRegistry(4, config.xSize, config.zSize) {
    terrain = config.get_terrain();
    auto player = add_entity<Player>("player1");
    player->pos = config.playerSpawnPos;

    for (auto& pos : config.mobs["zombie"]) {
        auto zombie = add_entity<Zombie>(generate_unique_id("zombie"));
        zombie->pos = pos;
    }

    for (auto& pos : config.mobs["skeleton"]) {
        auto skeleton = add_entity<Skeleton>(generate_unique_id("skeleton"));
        skeleton->pos = pos;
    }
}
