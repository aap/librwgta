#include "euryopa.h"

static bool show_demo_window;

static void
mainmenu(void)
{
	if(ImGui::BeginMainMenuBar()){
		if(ImGui::BeginMenu("File")){
			if(ImGui::MenuItem("Exit", "Alt+F4")) sk::globals.quit = 1;
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("View")){
			if(ImGui::MenuItem("Draw Collisions", NULL, gRenderCollision)) { gRenderCollision ^= 1; }
/*
			if(ImGui::MenuItem("Undo", "CTRL+Z")) {}
			if(ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
			ImGui::Separator();
			if(ImGui::MenuItem("Cut", "CTRL+X")) {}
			if(ImGui::MenuItem("Copy", "CTRL+C")) {}
			if(ImGui::MenuItem("Paste", "CTRL+V")) {}
*/
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Misc")){
			if(ImGui::MenuItem("Show Demo Window", NULL, show_demo_window)) { show_demo_window ^= 1; }
			ImGui::EndMenu();
		}
		ImGui::Separator();
		ImGui::Text("%.3f ms/frame %.1f FPS", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::EndMainMenuBar();
	}
}

void
gui(float timeDelta)
{
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui_ImplRW_NewFrame(timeDelta);

	mainmenu();

/*
	// 1. Show a simple window.
	// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");                           // Some text (you can use a format string too)
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float as a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats as a color
		if(ImGui::Button("Demo Window"))                       // Use buttons to toggle our bools. We could use Checkbox() as well.
			show_demo_window ^= 1;
		if(ImGui::Button("Another Window"))
			show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name the window.
	if(show_another_window){
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello from another window!");
		ImGui::End();
	}
*/

        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow().
	if(show_demo_window){
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowDemoWindow(&show_demo_window);
	}

	ImGui::EndFrame();
	ImGui::Render();
}
