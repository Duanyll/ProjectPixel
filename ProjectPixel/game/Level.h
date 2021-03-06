#pragma once

#include "pch.h"
#include "Terrain.h"

#include "Instructions.h"

class LevelConfig {
   public:
    int version;
    int xSize, ySize, zSize;
    std::string terrainType;
    std::string terrainPath;

    glm::vec3 playerSpawnPos;

    std::unordered_map<std::string, std::vector<glm::vec3>> mobs;

    pTerrain get_terrain();

    std::string goal;
    glm::vec3 destinationPos;
    float destinationRange;

    bool enableMobSpawning;
    int maxEnemies;
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
class Player;
class Goal;
class Level {
   public:
    pTerrain terrain;
    std::unordered_map<std::string, std::shared_ptr<Entity>> entities;
    LocationBuffer<Entity> entityRegistry;
    std::shared_ptr<Player> player;
    LevelConfig& config;
    std::vector<ParticleInstruction> particles;

    std::shared_ptr<Goal> goal;

    std::mt19937 random{
        std::chrono::steady_clock::now().time_since_epoch().count()};

    Level(LevelConfig& config);

    template <typename TEntity, typename... TParams>
    std::shared_ptr<TEntity> add_entity(TParams... params) {
        auto e = std::make_shared<TEntity>(*this, params...);
        entities[e->id] = e;
        return e;
    }
};

enum class HurtType;
class Goal {
   public:
    Level& level;
    inline Goal(Level& level) : level(level) {}
    virtual bool should_game_stop(bool& isWin) = 0;
    virtual std::string get_goal_display() = 0;

    inline virtual void on_game_start() {}
    inline virtual void on_mob_die(std::string type) {}
    inline virtual void on_mob_hurt(std::string sender, std::string target,
                                    HurtType type, int hits) {}
    inline virtual void on_intercept_arrow() {}
};

class SpeedRunGoal : public Goal {
   public:
    inline SpeedRunGoal(Level& level) : Goal(level) {}
    bool should_game_stop(bool& isWin);
    std::string get_goal_display();

    void on_game_start();
    void on_mob_die(std::string type);

   protected:
    TimeStamp startTime;
};

class ClearGoal : public Goal {
   public:
    inline ClearGoal(Level& level) : Goal(level) {}
    bool should_game_stop(bool& isWin);
    std::string get_goal_display();

    void on_game_start();
    void on_mob_die(std::string type);

   protected:
    int remainEnemies = 0;
    TimeStamp startTime;
};

class ArcadeGoal : public Goal {
   public:
    inline ArcadeGoal(Level& level) : Goal(level) {}
    bool should_game_stop(bool& isWin);
    std::string get_goal_display();

    void on_mob_die(std::string type);
    void on_mob_hurt(std::string sender, std::string target, HurtType type,
                     int hits);
    void on_intercept_arrow();

   protected:
    int score = 0;
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
                                   mobs, goal, destinationPos, destinationRange,
                                   enableMobSpawning, maxEnemies);