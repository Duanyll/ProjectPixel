// ProjectPixel.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

#include "utils/Window.h"
#include "utils/FrameTimer.h"

#include "renderer/AssetsHub.h"
#include "renderer/Camera.h"
#include "renderer/Uniform.h"
#include "renderer/Light.h"
#include "renderer/Objects.h"
#include "utils/Text.h"

FreeCamera camera(1920, 1080);

int main() {
    Window::init_glfw();
    auto window = Window::create(1920, 1080, "ProjectPixel");

    Window::register_key(
        GLFW_KEY_ESCAPE, Window::KeyMode::KeyDown,
        [&]() -> void { glfwSetWindowShouldClose(window, true); });
    Window::register_key(GLFW_KEY_W, Window::KeyMode::Continous, [&]() -> void {
        camera.move_pos(FreeCamera::Direction::Front);
    });
    Window::register_key(GLFW_KEY_A, Window::KeyMode::Continous, [&]() -> void {
        camera.move_pos(FreeCamera::Direction::Left);
    });
    Window::register_key(GLFW_KEY_S, Window::KeyMode::Continous, [&]() -> void {
        camera.move_pos(FreeCamera::Direction::Back);
    });
    Window::register_key(GLFW_KEY_D, Window::KeyMode::Continous, [&]() -> void {
        camera.move_pos(FreeCamera::Direction::Right);
    });
    Window::register_key(
        GLFW_KEY_LEFT_SHIFT, Window::KeyMode::Continous,
        [&]() -> void { camera.move_pos(FreeCamera::Direction::Down); });
    Window::register_key(
        GLFW_KEY_SPACE, Window::KeyMode::Continous,
        [&]() -> void { camera.move_pos(FreeCamera::Direction::Up); });

    AssetsHub::load_all();
    Logger::init();

    Logger::info("hello");

    FrameTimer::begin_frame_stats();

    DirLight dirLight;
    dirLight.apply();
    DirLight::set_ambient({0.1, 0.1, 0.1});
    PointLight::set_active_count(0);
    SpotLight spotLight;

    Paperman paperman;
    paperman.material = {AssetsHub::get_texture_2d("paperman-droid-diffuse"),
                         AssetsHub::get_texture_2d("paperman-droid-specular"),
                         AssetsHub::get_texture_2d("paperman-droid-emission"), 64, true};
    paperman.position = {10, 0, 5};
    paperman.animationType = Paperman::AnimationType::ZombieWalking;
    Skybox skybox;

    while (!glfwWindowShouldClose(window)) {
        Window::process_keys(window);

        glEnable(GL_DEPTH_TEST);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        camera.apply_uniform();
        spotLight.position = camera.pos;
        spotLight.direction = camera.front;
        spotLight.apply();

        paperman.step(FrameTimer::get_last_frame_time());
        paperman.render();
        skybox.render();

        Logger::flush();

        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameTimer::tick_frame();
    }

    Window::stop_glfw();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    camera.change_screen_size(width, height);
}

void cursor_move_callback(GLFWwindow* window, double xpos, double ypos) {
    camera.on_cursor_move(xpos, ypos);
}
void cursor_enter_callback(GLFWwindow* window, int in) {
    camera.on_cursor_enter(in);
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.on_scroll(yoffset);
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
