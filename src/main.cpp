#include "main.h"
#include "../ext/imgui/backends/imgui_impl_glfw.h"
#include "../ext/imgui/backends/imgui_impl_opengl3.h"
#include "Imports/Renderer/Renderer.h"

static GLFWwindow* window;

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

static void InitRenderDoc()
{
	RendererPtr = new Renderer();
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
	window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
	if (!window)
		return;

	glfwMakeContextCurrent(window);

	glewInit();
}

static void InitImGUI() {

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
}

static void Update() {

}

static void Render() {

	RendererPtr->HandleCaptureStart();
	
	glfwSwapBuffers(window);
	
	glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
	glClear(GL_COLOR_BUFFER_BIT);
	
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
	GamePtr = new Game();
	PlayerPtr = new PlayerService();
	GamePtr->Services.push_back(PlayerPtr);

	GamePtr->Init();
	RendererPtr->Init();
}

static void DestroyWorld()
{
	delete Game::Instance;
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
	if (!window) {
		glfwTerminate();
		return -1;
	}

	InitImGUI();
	InitWorld();
	
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
		Update();

		RenderUI();
		Render();
    }

	CleanUp();

}