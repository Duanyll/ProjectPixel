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

Level::Level(LevelConfig& config) : config(config) {
    terrain = config.get_terrain();
    add_entity<Player>("player1");
}
