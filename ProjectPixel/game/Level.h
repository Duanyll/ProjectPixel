#pragma once

#include "pch.h"
#include "Terrain.h"

class LevelConfig {
   public:
    int version;
    int xSize, ySize, zSize;
    std::string terrainType;
    std::string terrainPath;

    pTerrain get_terrain();
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelConfig, version, xSize, ySize, zSize,
                                   terrainType, terrainPath);