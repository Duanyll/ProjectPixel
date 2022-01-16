#pragma once

#include "pch.h"
#include "../utils/Camera.h"

enum class Key {
    Unknown,
    MoveFront,
    MoveBack,
    MoveLeft,
    MoveRight,
    Run,
    Jump,
    Heal,
    Slot1,
    Slot2,
    Slot3,
    Diagnostics
};

class GamingInput {
   public:
    struct State {
        glm::vec3 cameraPos;
        glm::vec3 cursorDir;
        glm::vec3 movementDir;
    };

    struct Keys {
        std::atomic_bool attack;
        std::atomic_bool aim;
        std::atomic_bool run;
        std::atomic_bool jump;
        std::atomic_bool heal;
        std::atomic_bool slot1;
        std::atomic_bool slot2;
        std::atomic_bool slot3;
    } keys;

    void collect_state(ThirdPersonCamera& camera);
    State poll_state();

    void active();

   protected:
    mutable std::mutex mtx;
    State state;
};