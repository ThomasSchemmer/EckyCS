#pragma once
#include <map>
#include <sstream>
#include <vector>

#include "glm/vec2.hpp"
#include "imgui/imgui.h"

struct ImDrawList;

namespace legit
{
  struct GpuProfilerFrame;
  struct ProfilerTask;
}

namespace ImGuiUtils
{
    
  class ProfilerGraph
  {
  public:
    int frameWidth;
    int frameSpacing;
    bool useColoredLegendText;

    ProfilerGraph(size_t framesCount);

    void LoadFrameData(const legit::ProfilerTask *tasks, size_t count);
    
    float GetTotalTaskTime(int frameIndexOffset);
    void RenderTimings(int graphWidth, int legendWidth, int height, int frameIndexOffset, float maxFrameTime);
    void RenderStats(const legit::GpuProfilerFrame& TargetFrame) const;

  private:
    size_t GetCurrFrameIndex(size_t frameIndexOffset);
    void RebuildTaskStats(size_t endFrame, size_t framesCount);
    void RenderGraph(ImDrawList *drawList, glm::vec2 graphPos, glm::vec2 graphSize, size_t frameIndexOffset, float maxFrameTime);
    void RenderLegend(ImDrawList *drawList, glm::vec2 legendPos, glm::vec2 legendSize, size_t frameIndexOffset, float maxFrameTime);
    
    static void Triangle(ImDrawList *drawList, std::array<glm::vec2, 3> points, uint32_t col, bool filled = true);
    static void RenderTaskMarker(ImDrawList *drawList, glm::vec2 leftMinPoint, glm::vec2 leftMaxPoint, glm::vec2 rightMinPoint, glm::vec2 rightMaxPoint, uint32_t col);
    static void Text(ImDrawList *drawList, glm::vec2 point, uint32_t col, const char *text);
    static void Rect(ImDrawList *drawList, glm::vec2 minPoint, glm::vec2 maxPoint, uint32_t col, bool filled = true);
    
    struct FrameData
    {
      std::vector<legit::ProfilerTask> tasks;
      std::vector<size_t> taskStatsIndex;
      float totalTime;
    };

    struct TaskStats
    {
      double maxTime;
      size_t priorityOrder;
      size_t onScreenIndex;
    };
    std::vector<TaskStats> taskStats;
    std::map<std::string, size_t> taskNameToStatsIndex;

    std::vector<FrameData> frames;
    size_t currFrameIndex = 0;
  };
}
