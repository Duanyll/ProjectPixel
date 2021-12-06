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

    std::unordered_map<std::string, std::vector<glm::vec3>> mobs;

    pTerrain get_terrain();
};

template <typename T>
class LocationBuffer {
   public:
    int gridSize, xSize, zSize, xGridCount, zGridCount;
    LocationBuffer(int gridSize, int xSize, int zSize)
        : gridSize(gridSize), xSize(xSize), zSize(zSize) {
        xGridCount = (xSize + gridSize - 1) / gridSize;
        zGridCount = (zSize + gridSize - 1) / gridSize;
    }
    std::vector<std::shared_ptr<T>> query_square_range(glm::vec3 center,
        float range) {
        int xGridMin = (center.x - range) / gridSize;
        int xGridMax = (center.x + range) / gridSize;
        int zGridMin = (center.z - range) / gridSize;
        int zGridMax = (center.z + range) / gridSize;
        std::vector<std::shared_ptr<T>> res;
        for (int x = xGridMin; x <= xGridMax; x++) {
            for (int z = zGridMin; z <= zGridMax; z++) {
                for (auto& i : store[x + z * xGridCount]) {
                    auto& pos = i->pos;
                    if (pos.x >= center.x - range &&
                        pos.x <= center.x + range &&
                        pos.z >= center.z - range &&
                        pos.z <= center.z + range) {
                        res.push_back(i);
                    }
                }
            }
        }
        return res;
    }

    void add(std::shared_ptr<T> e) {
        store[get_grid_id(e->pos.x, e->pos.z)].push_back(e);
    }
    void clear() { store.clear(); }

   private:
    std::unordered_map<int, std::vector<std::shared_ptr<T>>> store;
    int get_grid_id(int x, int z) {
        return (x / gridSize) + (z / gridSize) * xGridCount;
    }

};

class Entity;
class Level {
   public:
    pTerrain terrain;
    std::unordered_map<std::string, std::shared_ptr<Entity>> entities;
    LocationBuffer<Entity> entityRegistry;
    LevelConfig& config;

    Level(LevelConfig& config);

    template <typename TEntity, typename... TParams>
    std::shared_ptr<TEntity> add_entity(const std::string& id,
                                        TParams... params) {
        auto e = std::make_shared<TEntity>(*this, id, params...);
        entities[id] = e;
        return e;
    }
};

namespace glm {
inline void to_json(json& j, const glm::vec2& P) {
    j = {{"x", P.x}, {"y", P.y}};
};

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
                                   terrainType, terrainPath, playerSpawnPos,
                                   mobs);