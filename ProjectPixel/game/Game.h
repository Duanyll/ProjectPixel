#pragma once

#include "pch.h"
#include "Level.h"
#include "Terrain.h"
#include "Processor.h"
#include "../renderer/Objects.h"
#include "../renderer/AssetsHub.h"
#include "../utils/Utils.h"
#include "../utils/Window.h"
#include "../renderer/Camera.h"
#include "../renderer/Light.h"

class Game {
   public:
    Game(LevelConfig& config);
    LevelConfig& config;

    void apply_to_window();
    void start();
    void render();
    void stop();

   protected:
    DirLight dirLight;
    SpotLight spotLight;

    ThirdPersonCamera camera;
    TerrainRenderer terrainRenderer;
    Skybox skybox;
    std::unordered_map<std::string, std::shared_ptr<EntityRenderer>> entities;
    LevelProcessor processor;

    TimeStamp updateTime;

    void update();

};