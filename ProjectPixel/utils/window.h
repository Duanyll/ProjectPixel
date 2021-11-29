#pragma once

#include "../pch.h"

namespace Window {
void init_glfw();
extern GLFWwindow* handle;
extern int width, height;
void create(int width, int height, const std::string& title);
void stop_glfw();
enum class KeyMode { Continous, KeyDown, KeyUp, Toggle };

void register_command(const std::string& commandName,
                      std::function<void(float)> handler);
void execute_command(const std::string& commandName, float param = 0);
void remove_command(const std::string& commandName);
void bind_keys(const std::unordered_map<int, std::pair<std::string, KeyMode>>&
                   keybindings);
void update_key_mode(int key, KeyMode mode);
void rebind_key(int key, std::string& command);
void process_keys();
}  // namespace Window