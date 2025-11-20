#include "main.h"

#include <iostream>
#include "GL/glew.h"
#include <GLFW/glfw3.h>

#include "HGame.h"
#include "../ext/imgui/imgui.h"
#include "PlayerService.h"
#include "TestService.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"
#include "GameService/Game.h"
#include "Renderer/Renderer.h"


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

	std::shared_ptr<GLFWwindow> Window;
		
	void error_callback(int error, const char* description)
	{
		fprintf(stderr, "Error: %s\n", description);
	}

	void InitRenderDoc(shared_ptr<Renderer>& Ptr)
	{
		Ptr = make_shared<Renderer>();
		Ptr->InitRenderDoc();
	}

	void InitWindow(shared_ptr<Renderer>& Ptr) {
		
		InitRenderDoc(Ptr);
		if (!glfwInit())
			return;
		
		glfwSetErrorCallback(error_callback);

		// configure borderless maximized window
		GLFWmonitor* Monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* Mode = glfwGetVideoMode(Monitor);
		glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); 

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);
		
		GLFWwindow* Raw = glfwCreateWindow(Mode->width, Mode->height, "My Title", nullptr, nullptr);
		glfwSetWindowPos(Raw, 0, 0);

		// TODO:remove:auto delete on releasing the ptr
		Window = std::shared_ptr<GLFWwindow>(Raw, [](GLFWwindow* w) {
			if (glfwGetCurrentContext() && w)
			{
				glfwDestroyWindow(w);
			}
		});
		if (!Window)
			return;

		glfwMakeContextCurrent(Window.get());
		glfwSwapInterval(0); 

		glewInit();
		
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_BACK);
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(GLDebugMessageCallback, nullptr);
	}

	void InitImGUI() {

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplGlfw_InitForOpenGL(Window.get(), true);
		ImGui_ImplOpenGL3_Init();

		ImGui::StyleColorsDark();
	}

	void Update() {
		Game::Instance->Update();
		Game::GetRenderer()->Update(Game::DeltaTime);
	}

	void Render() {

		auto Renderer = Game::GetRenderer();
		Renderer->HandleCaptureStart();
		
		glfwSwapBuffers(Window.get());
		
		glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Renderer->Render();
		
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		
		Renderer->HandleCaptureStop();
	}

	void RenderUI() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// rest of the ui is done in the different systems/services
		// see e.g. @Renderer
	}

	void InitWorld(shared_ptr<Renderer>& Ptr)
	{
		Game::Instance = make_unique<HGame>([]() -> float{return static_cast<float>(glfwGetTime());});
		Game::Instance->RendererPtr = Ptr;
		Game::Instance->Services.push_back(make_shared<PlayerService>());

		Game::Instance->Init(Window);
	}

	void DestroyWorld()
	{
		Game::Instance.reset();
	}

	void CleanUp() {
		DestroyWorld();
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		glfwTerminate();
	}

	
}

int main()
{
	cout << "Starting glfw";

	shared_ptr<Renderer> Ptr;
	InitWindow(Ptr);
	if (!Window) {
		glfwTerminate();
		return -1;
	}

	InitImGUI();
	InitWorld(Ptr);
		
	int display_w, display_h;
	glfwGetFramebufferSize(Window.get(), &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);

	while (!glfwWindowShouldClose(Window.get()))
	{
		glfwPollEvents();
		Update();

		RenderUI();
		Render();
	}

	CleanUp();
	return 0;
}