#include "pch.h"
#include "Window.h"

#include <map>

void Window::init_glfw() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

GLFWwindow* Window::create(int width, int height, const std::string& title) {
    GLFWwindow* window =
        glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;

    glViewport(0, 0, width, height);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_move_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    return window;
}

void Window::stop_glfw() { glfwTerminate(); }

struct KeyState {
    Window::KeyMode mode;
    std::function<void()> callback;
    bool lastStatus = false;
};
std::map<int, KeyState> keyState;

void Window::register_key(int key, KeyMode mode,
                          std::function<void()> callback) {
    keyState[key] = {
        mode, callback, false
    };
}

void Window::process_keys(GLFWwindow* window) { 
    for (auto& i : keyState) {
        if (glfwGetKey(window, i.first) == GLFW_PRESS) {
            if (i.second.mode == KeyMode::Continous) {
                i.second.callback();
            } else if (i.second.mode == KeyMode::KeyDown ||
                       i.second.mode == KeyMode::Toggle) {
                if (!i.second.lastStatus) {
                    i.second.callback();
                }
            }
            i.second.lastStatus = true;
        } else {
            if (i.second.mode == KeyMode::KeyUp ||
                i.second.mode == KeyMode::Toggle) {
                if (i.second.lastStatus) {
                    i.second.callback();
                }
            }
            i.second.lastStatus = false;
        }
    }
}
