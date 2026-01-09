#pragma once

#include <chrono>

#include "ProfilerTask.h"
#include "GL/glew.h"

namespace legit
{
    struct GpuProfilerFrame
    {
        std::vector<ProfilerTask> tasks;

        void BeginFrame()
        {
            Invalidate();
        }

        void Invalidate()
        {
            tasks.clear();
        }

        void PushTask(const char* name, uint32_t color, GLuint startQuery, GLuint endQuery)
        {
            // We'll resolve the queries a few frames later
            legit::ProfilerTask t{};
            t.name = name;
            t.startTime = float(startQuery); // temporarily store query ID
            t.endTime   = float(endQuery);
            t.color     = color;
            tasks.push_back(t);
        }

        ~GpuProfilerFrame() = default;
    };

    struct GpuProfileScope
    {
        GpuProfilerFrame& frame;
        const char* name;
        uint32_t color;
        GLuint startQuery;
        GLuint endQuery;

        GpuProfileScope(GpuProfilerFrame& f, const char* n, uint32_t c)
            : frame(f), name(n), color(c)
        {
            glGenQueries(1, &startQuery);
            glGenQueries(1, &endQuery);
            glQueryCounter(startQuery, GL_TIMESTAMP);
        }

        ~GpuProfileScope()
        {
            glQueryCounter(endQuery, GL_TIMESTAMP);
            frame.PushTask(name, color, startQuery, endQuery);
        }
    };

}
#define GPU_CONCAT_IMPL(x, y) x##y
#define GPU_CONCAT(x, y) GPU_CONCAT_IMPL(x, y)

#define GPU_PROFILE(frame, name, color) \
    legit::GpuProfileScope GPU_CONCAT(_gpuScope_, __COUNTER__){frame, name, color}