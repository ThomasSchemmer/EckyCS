#pragma once

#include <chrono>
#include <iostream>

#include "ProfilerTask.h"
#include "GL/glew.h"

namespace legit
{
    struct GpuProfilerFrame
    {
        std::vector<ProfilerTask> tasks;
        
        GLuint VertexShaderInvocationsQuery;
        GLuint FragmentShaderInvocationsQuery;
        GLuint PrimitivesGeneratedQuery;
        GLuint SamplesPassedQuery;
        GLuint TimeElapsedQuery;
        GLuint FrameStartQuery;

        GLuint64 VertexShaderInvocations = 0;
        GLuint64 FragmentShaderInvocations = 0;
        GLuint64 PrimitivesGenerated = 0;
        GLuint64 SamplesPassed = 0;
        GLuint64 TimeElapsed = 0;

        bool bIsValid = false;

        void BeginFrame()
        {
            Invalidate();
            StartQueries();
        }

        void Invalidate()
        {
            tasks.clear();
        }

        void InvalidateQueries()
        {
            if (VertexShaderInvocationsQuery != 0)   glDeleteQueries(1, &VertexShaderInvocationsQuery);
            if (FragmentShaderInvocationsQuery != 0) glDeleteQueries(1, &FragmentShaderInvocationsQuery);
            if (PrimitivesGeneratedQuery != 0)       glDeleteQueries(1, &PrimitivesGeneratedQuery);
            if (SamplesPassedQuery != 0)             glDeleteQueries(1, &SamplesPassedQuery);
            if (TimeElapsedQuery != 0)               glDeleteQueries(1, &TimeElapsedQuery);
            if (FrameStartQuery != 0)                glDeleteQueries(1, &FrameStartQuery);
        }

        void PushTask(const char* name, uint32_t color, GLuint startQuery, GLuint endQuery)
        {
            // We'll resolve the queries a few frames later
            ProfilerTask t{};
            t.name = name;
            t.startTime = float(startQuery); // temporarily store query ID
            t.endTime   = float(endQuery);
            t.color     = color;
            tasks.push_back(t);
        }

        void StartQueries()
        {
            glBeginQuery(GL_VERTEX_SHADER_INVOCATIONS, VertexShaderInvocationsQuery);
            glBeginQuery(GL_FRAGMENT_SHADER_INVOCATIONS, FragmentShaderInvocationsQuery);
            glBeginQuery(GL_PRIMITIVES_GENERATED, PrimitivesGeneratedQuery);
            glBeginQuery(GL_SAMPLES_PASSED, SamplesPassedQuery);
            glBeginQuery(GL_TIME_ELAPSED, TimeElapsedQuery);
            glQueryCounter(FrameStartQuery, GL_TIMESTAMP);
            bIsValid = true;
        }

        void EndQueries()
        {
            glEndQuery(GL_VERTEX_SHADER_INVOCATIONS);
            glEndQuery(GL_FRAGMENT_SHADER_INVOCATIONS);
            glEndQuery(GL_PRIMITIVES_GENERATED);
            glEndQuery(GL_SAMPLES_PASSED);
            glEndQuery(GL_TIME_ELAPSED);
        }

        void ResolveQueries()
        {
            if (!bIsValid)
                return;
            
            EndQueries();
            glGetQueryObjectui64v(VertexShaderInvocationsQuery, GL_QUERY_RESULT, &VertexShaderInvocations);
            glGetQueryObjectui64v(FragmentShaderInvocationsQuery, GL_QUERY_RESULT, &FragmentShaderInvocations);
            glGetQueryObjectui64v(PrimitivesGeneratedQuery, GL_QUERY_RESULT, &PrimitivesGenerated);
            glGetQueryObjectui64v(SamplesPassedQuery, GL_QUERY_RESULT, &SamplesPassed);
            glGetQueryObjectui64v(TimeElapsedQuery, GL_QUERY_RESULT, &TimeElapsed);
        }
        
        void ResolveTasks()
        {
            GLuint64 FrameStart;
            glGetQueryObjectui64v(FrameStartQuery, GL_QUERY_RESULT, &FrameStart);
            
            for (auto& task : tasks)
            {
                GLuint64 startTime = 0, endTime = 0;
                GLuint StartQuery = GLuint(task.startTime);
                GLuint EndQuery = GLuint(task.endTime);
                glGetQueryObjectui64v(StartQuery, GL_QUERY_RESULT, &startTime);
                glGetQueryObjectui64v(EndQuery,   GL_QUERY_RESULT, &endTime);
                glDeleteQueries(1, &StartQuery);
                glDeleteQueries(1, &EndQuery);
                
                task.startTime = static_cast<double>(startTime - FrameStart) * 1e-9f; // convert ns → seconds
                task.endTime   = static_cast<double>(endTime - FrameStart)   * 1e-9f;
            }
        }

        GpuProfilerFrame()
        {
            glGenQueries(1, &VertexShaderInvocationsQuery);
            glGenQueries(1, &FragmentShaderInvocationsQuery);
            glGenQueries(1, &PrimitivesGeneratedQuery);
            glGenQueries(1, &SamplesPassedQuery);
            glGenQueries(1, &TimeElapsedQuery);
            glGenQueries(1, &FrameStartQuery);
        }
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