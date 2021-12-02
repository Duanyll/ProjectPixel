#include "pch.h"
#include "FrameTimer.h"

#include "Window.h"
#include "Text.h";

TimeStamp lastFrame;
TimeDuration deltaTime;
float deltaTimeFloat;

const int fps_stats_cycle = 50;
std::queue<float> frametime_history;
float total_time;
int frame_count;
float fps;

void FrameTimer::begin_frame_stats() {
    lastFrame = std::chrono::steady_clock::now();
    Window::register_command("framerate", [](float _) {
        Logger::info(std::format("Framerate: {:.2f}FPS", get_average_framerate()));
    });
}

void FrameTimer::tick_frame() {
    auto currentFrame = std::chrono::steady_clock::now();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    deltaTimeFloat = to_float_timestamp(deltaTime);
    total_time += deltaTimeFloat;
    frametime_history.push(deltaTimeFloat);
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

float FrameTimer::get_last_frame_time() { return deltaTimeFloat; }

float FrameTimer::get_average_framerate() {
    if (total_time == 0) return 165;
    return frametime_history.size() / total_time;
}

float FrameTimer::get_fps() { return fps; }

TimeStamp TickTimer::start_tick() { 
    startTime = std::chrono::steady_clock::now();
    return startTime + get_average_spt();
}

void TickTimer::end_tick() {
    auto current = std::chrono::steady_clock::now() - startTime;
    tickTimeHistory.push(current);
    totalTime += current;
    if (tickTimeHistory.size() > statCycle) {
        auto front = tickTimeHistory.front();
        tickTimeHistory.pop();
        totalTime -= front;
    }
}

TimeDuration TickTimer::get_average_spt() { 
    if (tickTimeHistory.size() < statCycle) return TimeDuration(0);
    return totalTime / tickTimeHistory.size(); 
}
