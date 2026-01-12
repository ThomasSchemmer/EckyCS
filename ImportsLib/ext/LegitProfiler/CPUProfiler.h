#pragma once
#include <chrono>
#include <vector>

#include "ProfilerTask.h"

namespace legit
{
    inline double NowSeconds()
    {
        using clock = std::chrono::high_resolution_clock;
        return std::chrono::duration<double>(clock::now().time_since_epoch()).count();
    }

    struct CpuProfilerFrame
    {
        double frameStart = 0.0;
        std::vector<legit::ProfilerTask> tasks;

        void BeginFrame()
        {
            tasks.clear();
            frameStart = NowSeconds();
        }

        void PushTask(const char* name, uint32_t color, double start, double end)
        {
            legit::ProfilerTask t{};
            t.name = name;
            t.startTime = float(start - frameStart);
            t.endTime   = float(end - frameStart);
            t.color = color;
            tasks.push_back(t);
        }
    };

    struct CpuProfileScope
    {
        CpuProfilerFrame& frame;
        const char* name;
        uint32_t color;
        double start;

        CpuProfileScope(CpuProfilerFrame& f, const char* n, uint32_t c)
          : frame(f), name(n), color(c), start(NowSeconds())
        {}

        ~CpuProfileScope()
        {
            frame.PushTask(name, color, start, NowSeconds());
        }
    };
}


#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define CPU_PROFILE(frame, name, color)                   \
    do                                                     \
    {                                                      \
    [[maybe_unused]] legit::CpuProfileScope                  \
    CONCAT(_cpuScope_, __COUNTER__) { frame, name, color }; \
    } while (0)