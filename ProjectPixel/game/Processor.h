#pragma once

#include "pch.h"
#include "../utils/Threading.h"
#include "../utils/FrameTimer.h"
#include "../utils/Input.h"
#include "Level.h"
#include "Terrain.h"
#include "Instructions.h"
#include "Entity.h"

class LevelProcessor : public WorkerThread {
   public:
    LevelProcessor(LevelConfig& config);

    GamingInput input;
    OutputBuffer<SceneInstruction> output;

    inline std::chrono::milliseconds get_cycle_time() {
        return std::chrono::milliseconds(50);
    }

    void start();
    void work();

   protected:
    LevelConfig config;
    Level level;
    TickTimer timer;
    TimeStamp lastTime;

    bool isGameEnd = false;
    bool isWin = false;
    std::string finalScore;
    int ticksToStop = 0;

    void step_motion(float duration);
    void try_spawn_mobs();
    void tick_entities(float duration);
    void handle_user_input(float duration);
    void clip_speed();
    void emit_instructions(TimeStamp time);
};