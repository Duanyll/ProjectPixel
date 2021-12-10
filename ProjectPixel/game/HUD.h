#pragma once

#include "pch.h"
#include "../driver/AssetsHub.h"
#include "Instructions.h"

class GameHUD {
   public:
    void update(SceneInstruction& i);
    void render();

    bool doFlashHP = false;
    int playerHP;

    protected:
    void print_hp_bar();
};