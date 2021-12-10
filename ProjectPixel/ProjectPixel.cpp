// ProjectPixel.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

#include "utils/Window.h"
#include "utils/FrameTimer.h"

#include "driver/UI.h"
#include "game/Level.h"
#include "game/Game.h"

int main() {
    Window::init_glfw();
    Window::create(1920, 1080, "ProjectPixel");
    {
        using Window::KeyMode;
        Window::bind_keys({
            {GLFW_KEY_W, {"move-front", KeyMode::Continous}},
            {GLFW_KEY_A, {"move-left", KeyMode::Continous}},
            {GLFW_KEY_S, {"move-back", KeyMode::Continous}},
            {GLFW_KEY_D, {"move-right", KeyMode::Continous}},
            {GLFW_KEY_LEFT_SHIFT, {"move-down", KeyMode::Continous}},
            {GLFW_KEY_SPACE, {"move-up", KeyMode::KeyDown}},
            {GLFW_KEY_F3, {"diagnostics", KeyMode::KeyDown}},
            {GLFW_KEY_F, {"framerate", KeyMode::KeyDown}},
            {GLFW_KEY_ESCAPE, {"exit", KeyMode::KeyUp}},
            {GLFW_KEY_LEFT_CONTROL, {"run", KeyMode::Toggle}},
            {GLFW_KEY_1, {"slot-1", KeyMode::KeyDown}},
            {GLFW_KEY_2, {"slot-2", KeyMode::KeyDown}},
            {GLFW_KEY_3, {"slot-3", KeyMode::KeyDown}},
        });
    }
    Window::register_command("exit", [](float _) {
        glfwSetWindowShouldClose(Window::handle, true);
    });

    AssetsHub::load_all();
    UI::init();

    std::ifstream levelFile("levels/default.json");
    json configJson;
    levelFile >> configJson;
    auto config = configJson.get<LevelConfig>();
    Game game(config);
    game.apply_to_window();

    auto screen = std::make_shared<FrameBufferTexture>(1920, 1080, false);
    FullScreenQuad quad(screen);

    game.start();
    FrameTimer::begin_frame_stats();
    while (!glfwWindowShouldClose(Window::handle)) {
        Window::process_keys();

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        screen->draw_inside([&]() -> void { game.render(); });

        glViewport(0, 0, Window::width, Window::height);
        quad.render();

        UI::print_logs();

        glfwSwapBuffers(Window::handle);
        glfwPollEvents();

        FrameTimer::tick_frame();
    }
    game.stop();

    Window::stop_glfw();
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧:
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5.
//   转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
