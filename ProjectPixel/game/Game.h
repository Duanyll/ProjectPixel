#pragma once

#include "pch.h"
#include "Level.h"
#include "Terrain.h"
#include "Processor.h"
#include "Objects.h"
#include "HUD.h"
#include "Particle.h"
#include "../driver/AssetsHub.h"
#include "../utils/Utils.h"
#include "../utils/Window.h"
#include "../utils/Camera.h"
#include "../driver/Light.h"

class Game {
   public:
    Game(LevelConfig& config);
    LevelConfig& config;

    void apply_to_window();
    void start();
    void render();
    void stop();

   protected:
    ThirdPersonCamera camera;
    TerrainRenderer terrainRenderer;
    Skybox skybox;
    std::unordered_map<std::string, std::shared_ptr<EntityRenderer>> entities;
    ParticleManager particles;
    LevelProcessor processor;

    TimeStamp updateTime;

    GameHUD hud;

    std::shared_ptr<OutlineFrameBufferTexture> outlineBuffer;

    void update();
};