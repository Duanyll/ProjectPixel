#include "pch.h"
#include "FrameTimer.h"

float lastFrame;
float deltaTime;

const int fps_stats_cycle = 50;
std::queue<float> frametime_history;
float total_time;
int frame_count;
float fps;

void FrameTimer::begin_frame_stats() { lastFrame = glfwGetTime(); }

void FrameTimer::tick_frame() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    total_time += deltaTime;
    frametime_history.push(deltaTime);
    if (frametime_history.size() > fps_stats_cycle) {
        total_time -= frametime_history.front();
        frametime_history.pop();
    }

    frame_count++;
    if (frame_count >= fps_stats_cycle) {
        frame_count = 0;
        fps = float(frametime_history.size()) / total_time;
    }
}

float FrameTimer::get_last_frame_time() { return deltaTime; }

float FrameTimer::get_average_framerate() {
    return frametime_history.size() / total_time;
}

float FrameTimer::get_fps() { return fps; }
