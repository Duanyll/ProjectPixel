#pragma once

namespace FrameTimer {
void begin_frame_stats();
void tick_frame();
float get_last_frame_time();
float get_average_framerate();
float get_fps();
}  // namespace frame_timer