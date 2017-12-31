#include "euryopa.h"

static bool showDemoWindow;
static bool showEditorWindow;
static bool showHelpWindow;

static void
uiMainmenu(void)
{
	if(ImGui::BeginMainMenuBar()){
		if(ImGui::BeginMenu("File")){
			if(ImGui::MenuItem("Exit", "Alt+F4")) sk::globals.quit = 1;
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("View")){
			if(ImGui::MenuItem("Draw Collisions", nil, gRenderCollision)) { gRenderCollision ^= 1; }
			ImGui::Separator();
			static int render = 0;
			ImGui::RadioButton("Render Normal", &render, 0);
			ImGui::RadioButton("Render only HD", &render, 1);
			ImGui::RadioButton("Render only LOD", &render, 2);
			gRenderOnlyHD = !!(render&1);
			gRenderOnlyLod = !!(render&2);
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Misc")){
			if(ImGui::MenuItem("Show Demo Window", nil, showDemoWindow)) { showDemoWindow ^= 1; }
			if(ImGui::MenuItem("Show Editor Window", nil, showEditorWindow)) { showEditorWindow ^= 1; }
			if(ImGui::MenuItem("Help", nil, showHelpWindow)) { showHelpWindow ^= 1; }
			ImGui::EndMenu();
		}

		ImGui::Separator();
		ImGui::Text("%.3f ms/frame %.1f FPS", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::EndMainMenuBar();
	}
}

static void
uiHelpWindow(void)
{
	ImGui::Begin("Help", &showHelpWindow);

	ImGui::BulletText("Camera controls:\n"
		"LMB: first person look around\n"
		"Ctrl+Alt+LMB; W/S: move forward/backward\n"
		"MMB: pan\n"
		"Alt+MMB: arc rotate around target\n"
		"Ctrl+Alt+MMB: zoom into target"
		);
	ImGui::Separator();
	ImGui::BulletText("Selection: click on an object to select it,\n"
		"Shift+click to add to the selection,\n"
		"Alt+click to remove from the selection,\n"
		"Ctrl+click to toggle selection.");
	ImGui::BulletText("In the editor window, double click an instance to jump there,\n"
		"Right click a selection to deselect it.");
	ImGui::BulletText("Use the filter in the instance list to find instances by name.");

	if(ImGui::CollapsingHeader("Dear ImGUI help")){
		ImGui::ShowUserGuide();
		ImGui::TreePop();
	}

	ImGui::End();
}

static void
uiEditorWindow(void)
{
	CPtrNode *p;
	ObjectInst *inst;
	ObjectDef *obj;

	ImGui::Begin("Editor Window", &showEditorWindow);

	if(ImGui::TreeNode("CD images")){
		uiShowCdImages();
		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Selection")){
		for(p = selection.first; p; p = p->next){
			inst = (ObjectInst*)p->item;
			obj = GetObjectDef(inst->m_objectId);
			ImGui::PushID(inst);
			ImGui::Selectable(obj->m_name);
			ImGui::PopID();
			if(ImGui::IsItemHovered()){
				inst->m_highlight = HIGHLIGHT_HOVER;
				if(ImGui::IsMouseClicked(1))
					inst->Deselect();
				if(ImGui::IsMouseDoubleClicked(0))
					inst->JumpTo();
			}
		}
		ImGui::TreePop();
	}

	if(ImGui::TreeNode("Instances")){
		static ImGuiTextFilter filter;
		filter.Draw();
		static bool highlight;
		ImGui::Checkbox("Highlight matches", &highlight);
		for(p = instances.first; p; p = p->next){
			inst = (ObjectInst*)p->item;
			obj = GetObjectDef(inst->m_objectId);
			if(filter.PassFilter(obj->m_name)){
				bool pop = false;
				if(inst->m_selected){
					ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(255, 0, 0));
					pop = true;
				}
				ImGui::PushID(inst);
				ImGui::Selectable(obj->m_name);
				ImGui::PopID();
				if(ImGui::IsItemHovered()){
					if(ImGui::IsMouseClicked(1))
						inst->Select();
					if(ImGui::IsMouseDoubleClicked(0))
						inst->JumpTo();
				}
				if(pop)
					ImGui::PopStyleColor();
				if(highlight)
					inst->m_highlight = HIGHLIGHT_FILTER;
				if(ImGui::IsItemHovered())
					inst->m_highlight = HIGHLIGHT_HOVER;
			}
		}
		ImGui::TreePop();
	}

	ImGui::End();
}

void
gui(float timeDelta)
{
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	uiMainmenu();

	if(showEditorWindow)
		uiEditorWindow();

	if(showHelpWindow)
		uiHelpWindow();

	if(showDemoWindow){
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&showDemoWindow);
	}
}
