#pragma once

#include "../pch.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_move_callback(GLFWwindow* window, double xpos, double ypos);
void cursor_enter_callback(GLFWwindow* window, int in);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

namespace Window {
void init_glfw();
GLFWwindow* create(int width, int height, const std::string& title);
void stop_glfw();
enum class KeyMode {
	Continous,
	KeyDown,
	KeyUp,
	Toggle
};
void register_key(int key, KeyMode mode, std::function<void()> callback);
void process_keys(GLFWwindow* window);
}