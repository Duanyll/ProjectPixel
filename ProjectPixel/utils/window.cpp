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

GLFWwindow* Window::handle = nullptr;
int Window::width, Window::height;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    Window::execute_command("frame-resize");
}

void cursor_move_callback(GLFWwindow* window, double xpos, double ypos) {
    Window::execute_command("mouse");
    Window::execute_command("mouse-x", xpos);
    Window::execute_command("mouse-y", ypos);
}
void cursor_enter_callback(GLFWwindow* window, int in) {
    Window::execute_command("mouse-enter");
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    Window::execute_command("scroll-x", xoffset);
    Window::execute_command("scroll-y", yoffset);
}
void mouse_button_callback(GLFWwindow* window, int button, int action,
                           int mods) {
    Window::execute_command("mouse-button");
}

void Window::create(int width, int height, const std::string& title) {
    Window::width = width;
    Window::height = height;
    handle = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (handle == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(handle);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) << std::endl;

    glViewport(0, 0, width, height);
    glfwSetCursorPosCallback(handle, cursor_move_callback);
    glfwSetCursorEnterCallback(handle, cursor_enter_callback);
    glfwSetScrollCallback(handle, scroll_callback);
    glfwSetFramebufferSizeCallback(handle, framebuffer_size_callback);
    glfwSetMouseButtonCallback(handle, mouse_button_callback);
    
    // Turn on vsync
    glfwSwapInterval(1);
}

void Window::stop_glfw() { glfwTerminate(); }

std::unordered_map<std::string, std::function<void(float)>> commandHandlers;

void Window::register_command(const std::string& commandName,
                              std::function<void(float)> handler) {
    commandHandlers[commandName] = handler;
}

void Window::execute_command(const std::string& commandName, float param) {
    auto it = commandHandlers.find(commandName);
    if (it != commandHandlers.end()) {
        it->second(param);
    }
}

void Window::remove_command(const std::string& commandName) {
    commandHandlers.erase(commandName);
}

struct KeyState {
    Window::KeyMode mode;
    std::string command;
    bool lastStatus = false;
};
std::unordered_map<int, KeyState> keyState;

void Window::bind_keys(
    const std::unordered_map<int, std::pair<std::string, KeyMode>>&
        keybindings) {
    keyState.clear();
    for (auto& i : keybindings) {
        keyState[i.first] = {i.second.second, i.second.first, false};
    }
}

void Window::update_key_mode(int key, KeyMode mode) {
    keyState[key].mode = mode;
    keyState[key].lastStatus = false;
}

void Window::rebind_key(int key, std::string& command) {
    keyState[key].command = command;
}

void Window::process_keys() {
    for (auto& i : keyState) {
        if (glfwGetKey(handle, i.first) == GLFW_PRESS) {
            if (i.second.mode == KeyMode::Continous) {
                execute_command(i.second.command, 1);
            } else if (i.second.mode == KeyMode::KeyDown ||
                       i.second.mode == KeyMode::Toggle) {
                if (!i.second.lastStatus) {
                    execute_command(i.second.command, 1);
                }
            }
            i.second.lastStatus = true;
        } else {
            if (i.second.mode == KeyMode::KeyUp ||
                i.second.mode == KeyMode::Toggle) {
                if (i.second.lastStatus) {
                    execute_command(i.second.command, 0);
                }
            }
            i.second.lastStatus = false;
        }
    }
}