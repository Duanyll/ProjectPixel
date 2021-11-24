// ProjectPixel.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"

#include "utils/Window.h"
#include "utils/FrameTimer.h"

int main()
{ 
    Window::init_glfw();
    auto window = Window::create(1920, 1080, "ProjectPixel"); 

    FrameTimer::begin_frame_stats();
    while (!glfwWindowShouldClose(window)) {
        Window::process_keys(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();

        FrameTimer::tick_frame();
    }

    Window::stop_glfw();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void cursor_move_callback(GLFWwindow* window, double xpos, double ypos) {}
void cursor_enter_callback(GLFWwindow* window, int in) {}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
