#include "pch.h"
#include "Input.h"

#include "Window.h"

std::unordered_map<Key, int> keyBindings{
    {Key::MoveFront, GLFW_KEY_W},    {Key::MoveBack, GLFW_KEY_S},
    {Key::MoveLeft, GLFW_KEY_A},     {Key::MoveRight, GLFW_KEY_D},
    {Key::Run, GLFW_KEY_LEFT_SHIFT}, {Key::Jump, GLFW_KEY_SPACE},
    {Key::Heal, GLFW_KEY_X},         {Key::Slot1, GLFW_KEY_1},
    {Key::Slot2, GLFW_KEY_2},        {Key::Slot3, GLFW_KEY_3},
    {Key::Diagnostics, GLFW_KEY_F3}};

void GamingInput::collect_state(ThirdPersonCamera& camera) {
    std::lock_guard<std::mutex> lg(mtx);
    state.cameraPos = camera.pos;
    state.cursorDir = camera.resolve_cursor_dir();
    auto check_key = [&](Key key, glm::vec3 dir) -> void {
        if (glfwGetKey(Window::handle, keyBindings.at(key)) == GLFW_PRESS) {
            state.movementDir += glm::normalize(dir);
        }
    };
    check_key(Key::MoveFront, camera.groundFront);
    check_key(Key::MoveBack, -camera.groundFront);
    check_key(Key::MoveRight, glm::cross(camera.groundFront, camera.up));
    check_key(Key::MoveLeft, -glm::cross(camera.groundFront, camera.up));
}

GamingInput::State GamingInput::poll_state() {
    std::lock_guard<std::mutex> lg(mtx);
    auto res = state;
    state.movementDir = {0, 0, 0};
    return res;
}

void GamingInput::active() {
    class GameKeyWatcher : public Window::KeyWatcher {
        std::atomic_bool& out;

       public:
        GameKeyWatcher(Key key, std::atomic_bool& out)
            : Window::KeyWatcher(keyBindings.at(key)), out(out) {}

        void on_key_down() { out = true; }
        void on_key_up() { out = false; }
    };

    class GameMouseWatcher : public Window::MouseButtonWatcher {
        std::atomic_bool& out;

       public:
        GameMouseWatcher(int button, std::atomic_bool& out)
            : Window::MouseButtonWatcher(button), out(out) {}

        void on_button_down() { out = true; }
        void on_button_up() { out = false; }
    };

    std::vector<std::shared_ptr<Window::InputWatcher>> watchers{
        std::make_shared<GameKeyWatcher>(Key::Run, keys.run),
        std::make_shared<GameKeyWatcher>(Key::Jump, keys.jump),
        std::make_shared<GameKeyWatcher>(Key::Heal, keys.heal),
        std::make_shared<GameKeyWatcher>(Key::Slot1, keys.slot1),
        std::make_shared<GameKeyWatcher>(Key::Slot2, keys.slot2),
        std::make_shared<GameKeyWatcher>(Key::Slot3, keys.slot3),
        std::make_shared<GameMouseWatcher>(GLFW_MOUSE_BUTTON_LEFT, keys.attack),
        std::make_shared<GameMouseWatcher>(GLFW_MOUSE_BUTTON_RIGHT, keys.aim),
    };

    for (auto& i : watchers) {
        Window::add_watcher(i);
    }
}
