#pragma once
#include <chrono>

#include "ProfilerGraph.h"
#include "ProfilerTask.h"

namespace ImGuiUtils
{
  class ProfilersWindow
  {
  public:
    
    ProfilersWindow(float maxFrameTime = 1.0f / 60.0f);
    void Render(const legit::GpuProfilerFrame& TargetFrame);

    bool stopProfiling;
    int frameOffset;
    ProfilerGraph cpuGraph;
    ProfilerGraph gpuGraph;
    int frameWidth;
    int frameSpacing;
    bool useColoredLegendText;
    using TimePoint = std::chrono::time_point<std::chrono::system_clock>;
    TimePoint prevFpsFrameTime;
    size_t fpsFramesCount;
    float avgFrameTime;
    float maxFrameTime;
  };
}
