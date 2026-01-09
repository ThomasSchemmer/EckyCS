#include "main.h"

#include <iostream>

#include "HGame.h"
#include "../ext/imgui/imgui.h"
#include "PlayerService.h"
#include "Util/RenderTestService.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"
#include "GameService/Game.h"
#include "Renderer/Renderer.h"

#define WIN32_LEAN_AND_MEAN
// mask windows byte def, otherwise it overrides std::byte
#define byte win_byte_override
#include <Windows.h>

#include "GL/wglew.h"
#include "renderdoc/renderdoc_app.h"
#undef byte


extern "C" {
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
	__declspec(dllexport) int NvOptimusEnablement = 1;
}

using namespace GameImports;
namespace
{
	
    void APIENTRY GLDebugMessageCallback(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam)
    {
        std::cerr << "GL DEBUG MESSAGE (" << id << "): " << message << std::endl;

        std::cerr << "  Source: ";
        switch (source)
        {
        case GL_DEBUG_SOURCE_API:             std::cerr << "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Other"; break;
        }
        std::cerr << std::endl;

        std::cerr << "  Type: ";
        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:               std::cerr << "Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cerr << "Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cerr << "Other"; break;
        }
        std::cerr << std::endl;

        std::cerr << "  Severity: ";
        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "HIGH"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cerr << "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "NOTIFICATION"; break;
        }
        std::cerr << std::endl;
    }
		
	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}
	

	void HandleCaptureStart();
	void HandleCaptureStop();
	void LoadRenderDocWindows();
	void UnloadRenderDocWindows();

	void InitWindow() {
#ifdef _WIN32
		LoadRenderDocWindows();
#endif
		if (!glfwInit())
			return;
		
		glfwSetErrorCallback(error_callback);

		// configure borderless maximized window
		GLFWmonitor* Monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* Mode = glfwGetVideoMode(Monitor);
		glfwDefaultWindowHints();
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		
		Window = glfwCreateWindow(Mode->width, Mode->height, "My Title", nullptr, nullptr);
		if (!Window)
			return;
		
		
		glfwSetWindowPos(Window, 0, 0);
		glfwMakeContextCurrent(Window);
		glfwSwapInterval(0); 
		
		glewInit();
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BACK);
		//glDisable(GL_DEBUG_OUTPUT);
		//glDebugMessageCallback(GLDebugMessageCallback, nullptr);
	}

	void InitImGUI() {

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForOpenGL(Window, true);
		ImGui_ImplOpenGL3_Init();

		ImGui::StyleColorsDark();
	}

	void Update() {
		auto& Instance = Game::Instance;
		{
			CPU_PROFILE(Game::CpuProfilerFrame, "Update::Game", legit::Colors::emerald);
			Instance->Update();
		}
		{
			CPU_PROFILE(Game::CpuProfilerFrame, "Update::Renderer", legit::Colors::nephritis);
			Instance->RendererPtr->Update(Game::DeltaTime);
		}
		{
			CPU_PROFILE(Game::CpuProfilerFrame, "Update::Terrain", legit::Colors::orange);
			Instance->TerrainPtr->Update(Game::DeltaTime);
		}
	}

	void ResolveGpuQueries(legit::GpuProfilerFrame& frame)
	{
		for (auto& task : frame.tasks)
		{
			GLuint64 startTime = 0, endTime = 0;
			glGetQueryObjectui64v(GLuint(task.startTime), GL_QUERY_RESULT, &startTime);
			glGetQueryObjectui64v(GLuint(task.endTime),   GL_QUERY_RESULT, &endTime);

			task.startTime = double(startTime) * 1e-9f; // convert ns → seconds
			task.endTime   = double(endTime)   * 1e-9f;
		}
	}

	void Render() {
		{
			CPU_PROFILE(Game::CpuProfilerFrame, "Render", legit::Colors::alizarin);
			auto Renderer = Game::GetRenderer();
			HandleCaptureStart();
		
			glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
			Renderer->Render();
		}

		ResolveGpuQueries(Game::GetLastGpuFrame());
		Game::ProfilerWindow.gpuGraph.LoadFrameData(
		  Game::GetLastGpuFrame().tasks.data(),
		  Game::GetLastGpuFrame().tasks.size()
		);
		Game::GpuFrameNext();

		Game::ProfilerWindow.cpuGraph.LoadFrameData(
		  Game::CpuProfilerFrame.tasks.data(),
		  Game::CpuProfilerFrame.tasks.size()
		);

		Game::ProfilerWindow.Render();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		HandleCaptureStop();
		glfwSwapBuffers(Window);
	}

	void RenderUI() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// rest of the ui is done in the different systems/services
		// see e.g. @Renderer
	}

	void InitWorld()
	{
		Game::Instance = make_unique<HGame>([]() -> float{return static_cast<float>(glfwGetTime());});

		Game::Instance->Init(Window);
	}

	void DestroyWorld()
	{
		Game::Instance->TerrainPtr->CleanUp();
		Game::Instance->RendererPtr->CleanUp();
		Game::Instance->RendererPtr.reset();
		Game::Instance.reset();

		for (int i = 0; i < Game::GpuFrameCount; i++)
		{
			Game::GetGpuFrame(i).Invalidate();
		}
	}

	void CleanUp() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		
		glfwDestroyWindow(Window);
		DestroyWorld();
		
#ifdef _WIN32
		UnloadRenderDocWindows();
#endif
		glfwTerminate();
	}
	
	/***************** RENDERDOC INCLUDE *******************************/
#ifdef _WIN32

	char RenderDocPath[42] = "./ImportsLib/ext/renderdoc/renderdoc.dll";
	void LoadRenderDocWindows()
	{
		if(HMODULE RDC = LoadLibraryA(RenderDocPath))
		{
			auto RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(RDC, "RENDERDOC_GetAPI");
			int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void **)&RDocAPI);
			if (ret != 1)
			{
				throw std::runtime_error("ERROR::RENDERER::FAILED_LOADING_RENDERDOC\n");
			}
		}
	}

	void UnloadRenderDocWindows()
	{
		if (RDocAPI == nullptr)
			return;

		if (HMODULE RDC = GetModuleHandleA(RenderDocPath)) {
			RDocAPI->Shutdown();
			if (FreeLibrary(RDC)) {
				RDC = nullptr;
			} 
		}
	}
#endif


	void HandleCaptureStart() 
	{
		if(!RDocAPI)
			return;
    
		ImGui::Begin("RenderDoc");
		if (ImGui::Button("Capture") || glfwGetKey(Window, GLFW_KEY_F11) == GLFW_PRESS)
		{
			RDocAPI->StartFrameCapture(nullptr, nullptr);
		}
		ImGui::End();
	}

	void HandleCaptureStop()
	{
		if (!RDocAPI || !RDocAPI->IsFrameCapturing())
			return;

		RDocAPI->EndFrameCapture(nullptr, nullptr);

		char pathBuffer[4096];
		auto Num = RDocAPI->GetNumCaptures();
		RDocAPI->GetCapture(Num - 1, pathBuffer, nullptr, nullptr);
    
		RDocAPI->LaunchReplayUI(1, pathBuffer);
	}
	
	
}

int main()
{
	cout << "Starting glfw" << "\n";

	InitWindow();
	if (!Window) {
		glfwTerminate();
		return -1;
	}

	InitImGUI();
	InitWorld();
		

	while (!glfwWindowShouldClose(Window))
	{
		Game::CpuProfilerFrame.BeginFrame();
		Game::GetGpuFrame().BeginFrame();
		{
			CPU_PROFILE(Game::CpuProfilerFrame, "PollEvents", legit::Colors::amethyst);
			glfwPollEvents();
		}
		{
			CPU_PROFILE(Game::CpuProfilerFrame, "Update", legit::Colors::carrot);
			Update();
		}

		{
			CPU_PROFILE(Game::CpuProfilerFrame, "RenderUI", legit::Colors::clouds);
			RenderUI();
		}
		Render();
	}
	CleanUp();
	return 0;
}
