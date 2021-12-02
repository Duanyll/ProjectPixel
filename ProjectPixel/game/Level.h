#pragma once

#include "pch.h"
#include "Terrain.h"

class LevelConfig {
   public:
    int version;
    int xSize, ySize, zSize;
    std::string terrainType;
    std::string terrainPath;

    glm::vec3 playerSpawnPos;

    pTerrain get_terrain();
};

class Entity;
class Level {
   public:
    pTerrain terrain;
    std::unordered_map<std::string, std::shared_ptr<Entity>> entities;
    LevelConfig& config;

    Level(LevelConfig& config);

    template<typename TEntity, typename ...TParams>
    std::shared_ptr<TEntity> add_entity(const std::string& id,
        TParams... params) {
        auto e = std::make_shared<TEntity>(*this, id, params...);
        entities[id] = e; 
        return e;
    }
};

namespace glm {
inline void to_json(json& j, const glm::vec2& P) { j = {{"x", P.x}, {"y", P.y}}; };

inline void from_json(const json& j, glm::vec2& P) {
    P.x = j.at("x").get<double>();
    P.y = j.at("y").get<double>();
}
inline void to_json(json& j, const glm::vec3& P) {
    j = {{"x", P.x}, {"y", P.y}, {"z", P.z}};
};

inline void from_json(const json& j, glm::vec3& P) {
    P.x = j.at("x").get<double>();
    P.y = j.at("y").get<double>();
    P.z = j.at("z").get<double>();
}
}  // namespace glm

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LevelConfig, version, xSize, ySize, zSize,
                                   terrainType, terrainPath, playerSpawnPos);