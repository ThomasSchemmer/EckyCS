#include "main.h"

#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../ext/imgui/imgui.h"
#include "PlayerService.h"
#include "TestService.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"
#include "GameService/Game.h"
#include "Renderer/Renderer.h"

static std::shared_ptr<GLFWwindow> Window;
using namespace GameImports;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void InitRenderDoc()
{
	RendererPtr = make_unique<Renderer>();
	RendererPtr->InitRenderDoc();
}

static void InitWindow() {
	
	InitRenderDoc();
	if (!glfwInit())
		return;
	
	glfwSetErrorCallback(error_callback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* Raw = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	Window = std::shared_ptr<GLFWwindow>(Raw, [](GLFWwindow* w) {
		glfwDestroyWindow(w);
	});
	if (!Window)
		return;

	glfwMakeContextCurrent(Window.get());

	glewInit();
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BACK);
}

static void InitImGUI() {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(Window.get(), true);
	ImGui_ImplOpenGL3_Init();

	ImGui::StyleColorsDark();
}

static void Update() {
	Game::Instance->Update();
	RendererPtr->Update();
}

static void Render() {

	RendererPtr->HandleCaptureStart();
	
	glfwSwapBuffers(Window.get());
	
	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	RendererPtr->Render();
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	
	RendererPtr->HandleCaptureStop();
}

bool bIsToolActive = true;
static void RenderUI() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::Begin("My First Tool", &bIsToolActive, ImGuiWindowFlags_MenuBar);
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open..", "Ctrl+O")) { /* Do stuff */ }
			if (ImGui::MenuItem("Save", "Ctrl+S")) { /* Do stuff */ }
			if (ImGui::MenuItem("Close", "Ctrl+W")) { bIsToolActive = false; }
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	// Generate samples and plot them
	float samples[100];
	for (int n = 0; n < 100; n++)
		samples[n] = sinf(0.2f * n + ImGui::GetTime() * 1.5f);
	ImGui::PlotLines("Samples", samples, 100);

	// Display contents in a scrolling region
	ImGui::TextColored(ImVec4(1, 1, 0, 1), "Important Stuff");
	ImGui::BeginChild("Scrolling");
	for (int n = 0; n < 50; n++)
		ImGui::Text("%04d: Some text", n);
	ImGui::EndChild();
	ImGui::End();
}

static void InitWorld()
{
	Game::Instance = make_unique<Game>([]() -> float{return static_cast<float>(glfwGetTime());});
	Game::Instance->Services.push_back(make_shared<TestService>());
	Game::Instance->Services.push_back(make_shared<PlayerService>());

	Game::Instance->Init();
	RendererPtr->Init([](const int Key) -> int
	{
		return glfwGetKey(Window.get(), Key);
	});
}

static void DestroyWorld()
{
	Game::Instance.reset();
}

static void CleanUp() {
	DestroyWorld();
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

int main()
{
	
	cout << "Starting glfw";

	InitWindow();
	if (!Window) {
		glfwTerminate();
		return -1;
	}

	InitImGUI();
	InitWorld();
	
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

}