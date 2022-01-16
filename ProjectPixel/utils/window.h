#pragma once

#include "../pch.h"

namespace Window {
void init_glfw();
extern GLFWwindow* handle;
extern int width, height;
void create(int width, int height, const std::string& title);
void stop_glfw();

class InputWatcher {
    friend void process_input();
   protected:
    virtual void tick() = 0;
};

class KeyWatcher : public InputWatcher {
   public:
    inline virtual void on_key_down() {}
    inline virtual void on_key_hold() {}
    inline virtual void on_key_up() {}

   protected:
    KeyWatcher(int key);
    void tick();

   private:
    int key;
    bool last = false;
};

class MouseButtonWatcher : public InputWatcher {
   public:
    inline virtual void on_button_down() {}
    inline virtual void on_button_hold() {}
    inline virtual void on_button_up() {}

   protected:
    MouseButtonWatcher(int button);
    void tick();

   private:
    int button;
    bool last = false;
};

class CursorMoveWatcher : public InputWatcher {
   public:
    inline virtual void on_cursor_move(float offsetX, float offsetY) {}

   protected:
    inline CursorMoveWatcher(){}
    void tick();
};

class ScrollWatcher : public InputWatcher {
   public:
    inline virtual void on_scroll(float offsetX, float offsetY) {}

   protected:
    inline ScrollWatcher() {}
    void tick();
};

class ResizeWatcher : public InputWatcher {
   public:
    inline virtual void on_resize(int width, int height) {}

   protected:
    inline ResizeWatcher() {}
    void tick();
};

void add_watcher(std::shared_ptr<InputWatcher> p);
void remove_watcher(std::shared_ptr<InputWatcher> p);
void process_input();

}  // namespace Window