// ProjectPixel.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

#include "utils/Window.h"
#include "utils/FrameTimer.h"

#include "driver/UI.h"
#include "driver/Flags.h"
#include "game/Level.h"
#include "game/Game.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "Please specify a level json file." << std::endl;
        return 1;
    }
    std::ifstream levelFile(argv[1]);
    json configJson;
    levelFile >> configJson;
    auto config = configJson.get<LevelConfig>();

    Window::init_glfw();
    Window::create(1920, 1080, "ProjectPixel");

    class WindowCloseWatcher : public Window::KeyWatcher {
       public:
        void on_key_down() { glfwSetWindowShouldClose(Window::handle, true); }
        WindowCloseWatcher() : Window::KeyWatcher(GLFW_KEY_ESCAPE) {}
    };
    Window::add_watcher(std::make_shared<WindowCloseWatcher>());

    AssetsHub::load_all();
    UI::init();
    auto game = std::make_unique<Game>(config);
    game->apply_to_window();

    auto screen = std::make_shared<FrameBufferTexture>(1920, 1080, false);
    FullScreenQuad quad(screen);

    game->start();
    FrameTimer::begin_frame_stats();
    while (!glfwWindowShouldClose(Window::handle)) {
        Window::process_input();

        screen->draw_inside([&]() -> void {
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            game->render();
        });
        Viewport v(Window::width, Window::height);
        quad.render();

        UI::print_logs();

        glfwSwapBuffers(Window::handle);
        FrameTimer::tick_frame();
    }
    game->stop();

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
