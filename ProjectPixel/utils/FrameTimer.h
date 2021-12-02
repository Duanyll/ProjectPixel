#pragma once

#include "Utils.h"

namespace FrameTimer {
void begin_frame_stats();
void tick_frame();
float get_last_frame_time();
float get_average_framerate();
float get_fps();
}  // namespace FrameTimer

class TickTimer {
   public:
    // 返回预期 tick 计算完成的时间
    TimeStamp start_tick();
    void end_tick();
    TimeDuration get_average_spt();

    const int statCycle = 20;

   protected:
    std::queue<TimeDuration> tickTimeHistory;
    TimeDuration totalTime = TimeDuration(0);
    TimeStamp startTime;
};