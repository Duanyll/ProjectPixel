#include "pch.h"
#include "HUD.h"

#include "../utils/Window.h"
#include "../driver/UI.h"

void GameHUD::update(SceneInstruction& i) {
    if (playerHP != i.playerHP) {
        playerHP = i.playerHP;
        doFlashHP = true;
    } else {
        doFlashHP = false;
    }
}

void GameHUD::render() {
    int sw = Window::width;
    int sh = Window::height;

    UI::print_text(std::to_string(playerHP), 25, sh - 75);
    print_hp_bar();
}

void GameHUD::print_hp_bar() {
    auto back = AssetsHub::get_texture_2d(doFlashHP ? "ui-heart-flashing"
                                                    : "ui-heart-empty");
    const int xbase = 25;
    const int ybase = 25;
    const int size = 45;
    const int padding = 5;
    const int perHeart = 5;
    const int heartCount = 10;

    int curx = xbase;
    int cury = ybase;
    for (int i = 1; i <= heartCount; i++) {
        UI::print_image2d(back, curx, cury, size, size);
        curx += size + padding;
    }

    auto full = AssetsHub::get_texture_2d("ui-heart-full");
    auto half = AssetsHub::get_texture_2d("ui-heart-half");
    curx = xbase;
    cury = ybase;
    int remain = playerHP;
    
    while (remain > 0) {
        if (remain > perHeart / 2) {
            UI::print_image2d(full, curx, cury, size, size);
        } else {
            UI::print_image2d(half, curx, cury, size, size);
        }
        curx += size + padding;
        remain -= perHeart;
    }
}