#include "ProfilersWindow.h"

#include "imgui/imgui.h"

namespace ImGuiUtils
{
    
    ProfilersWindow::ProfilersWindow(float maxFrameTime):
      cpuGraph(300),
      gpuGraph(300),
      maxFrameTime(maxFrameTime)
    {
      stopProfiling = false;
      frameOffset = 0;
      frameWidth = 3;
      frameSpacing = 1;
      useColoredLegendText = true;
      prevFpsFrameTime = std::chrono::system_clock::now();
      fpsFramesCount = 0;
      avgFrameTime = 1.0f;
    }
    
    void ProfilersWindow::Render(const legit::GpuProfilerFrame& TargetFrame)
    {
      fpsFramesCount++;
      auto currFrameTime = std::chrono::system_clock::now();
      {
        float fpsDeltaTime = std::chrono::duration<float>(currFrameTime - prevFpsFrameTime).count();
        if (fpsDeltaTime > 0.5f)
        {
          this->avgFrameTime = fpsDeltaTime / float(fpsFramesCount);
          fpsFramesCount = 0;
          prevFpsFrameTime = currFrameTime;
        }
      }

      std::stringstream title;
      title.precision(2);
      title << std::fixed << "Legit profiler [" << 1.0f / avgFrameTime << "fps\t" << " cpu: " << cpuGraph.GetTotalTaskTime(frameOffset) * 1000.0f << "ms gpu: " << gpuGraph.GetTotalTaskTime(frameOffset) * 1000.0f << "ms]###ProfilerWindow";
      //###AnimatedTitle
      ImGui::Begin(title.str().c_str(), 0, ImGuiWindowFlags_NoScrollbar);
      ImVec2 canvasSize = ImGui::GetContentRegionAvail();

      int sizeMargin = int(ImGui::GetStyle().ItemSpacing.y);
      int maxGraphHeight = 300;
      int statsHeight = 5 * ImGui::GetTextLineHeightWithSpacing();
      int availableGraphHeight = (int(canvasSize.y) - sizeMargin - statsHeight) / 2;
      int graphHeight = std::min(maxGraphHeight, availableGraphHeight);
      int legendWidth = 200;
      int graphWidth = int(canvasSize.x) - legendWidth;
      gpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset, maxFrameTime);
      cpuGraph.RenderTimings(graphWidth, legendWidth, graphHeight, frameOffset, maxFrameTime);
      gpuGraph.RenderStats(TargetFrame);
      if (graphHeight * 2 + sizeMargin + sizeMargin < canvasSize.y)
      {
        ImGui::Columns(2);
        size_t textSize = 50;
        ImGui::Checkbox("Stop profiling", &stopProfiling);
        //ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - textSize);
        ImGui::Checkbox("Colored legend text", &useColoredLegendText);
        ImGui::DragInt("Frame offset", &frameOffset, 1.0f, 0, 400);
        ImGui::NextColumn();

        ImGui::SliderInt("Frame width", &frameWidth, 1, 4);
        ImGui::SliderInt("Frame spacing", &frameSpacing, 0, 2);
        ImGui::SliderFloat("Transparency", &ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w, 0.0f, 1.0f);
        ImGui::Columns(1);
      }
      
      if (!stopProfiling)
        frameOffset = 0;
      gpuGraph.frameWidth = frameWidth;
      gpuGraph.frameSpacing = frameSpacing;
      gpuGraph.useColoredLegendText = useColoredLegendText;
      cpuGraph.frameWidth = frameWidth;
      cpuGraph.frameSpacing = frameSpacing;
      cpuGraph.useColoredLegendText = useColoredLegendText;


      ImGui::End();
    }
}
