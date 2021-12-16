#pragma once

#include "pch.h"
#include "../driver/AssetsHub.h"
#include "Instructions.h"
#include "Objects.h"

class GameHUD : public RenderObject {
   public:
    void update(SceneInstruction& i);
    void render();

    bool doFlashHP = false;
    int playerHP;

    ItemType weapon = ItemType::None;
    int lifePotionCount = 0;
    int arrowCount = 0;
    std::string goalDisplay;
    std::string centerTitle;

   protected:
    void print_hp_bar();
    void print_inventory();
};