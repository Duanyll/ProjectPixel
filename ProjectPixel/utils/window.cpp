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

bool resized;
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Window::width = width;
    Window::height = height;
    resized = true;
}

double mousex, mousey;
void cursor_move_callback(GLFWwindow* window, double xpos, double ypos) {
    mousex = xpos;
    mousey = ypos;
}
void cursor_enter_callback(GLFWwindow* window, int in) {}
double scrollx, scrolly;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    scrollx = true;
    scrolly = true;
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

    // Turn on vsync
    glfwSwapInterval(1);
}

void Window::stop_glfw() { glfwTerminate(); }
Window::KeyWatcher::KeyWatcher(int key) : key(key) {}

void Window::KeyWatcher::tick() {
    bool current = glfwGetKey(handle, key) == GLFW_PRESS;
    if (!last && current) {
        on_key_down();
    } else if (last && current) {
        on_key_hold();
    } else if (last && !current) {
        on_key_up();
    }
    last = current;
}

Window::MouseButtonWatcher::MouseButtonWatcher(int button) : button(button) {}

void Window::MouseButtonWatcher::tick() {
    bool current = glfwGetMouseButton(handle, button) == GLFW_PRESS;
    if (!last && current) {
        on_button_down();
    } else if (last && current) {
        on_button_hold();
    } else if (last && !current) {
        on_button_up();
    }
    last = current;
}


void Window::CursorMoveWatcher::tick() {
    if (mousex != 0 || mousey != 0) on_cursor_move(mousex, mousey);
}

void Window::ScrollWatcher::tick() {
    if (scrollx != 0 && scrolly != 0) on_scroll(scrollx, scrolly);
}

void Window::ResizeWatcher::tick() {
    if (resized) on_resize(width, height);
}

std::unordered_set<std::shared_ptr<Window::InputWatcher>> activeKeyWatchers;

void Window::add_watcher(std::shared_ptr<InputWatcher> p) {
    activeKeyWatchers.insert(p);
}

void Window::remove_watcher(std::shared_ptr<InputWatcher> p) {
    activeKeyWatchers.erase(p);
}

void Window::process_input() {
    mousex = mousey = 0;
    scrollx = scrolly = 0;
    resized = false;
    glfwPollEvents();
    for (auto& i : activeKeyWatchers) i->tick();
}