#include "euryopa.h"
#include "imgui/imgui_internal.h"

static bool showDemoWindow;
static bool showEditorWindow;
static bool showInstanceWindow;
static bool showLogWindow;
static bool showHelpWindow;

static bool showTimeWeatherWindow;
static bool showViewWindow;
static bool showRenderingWindow;

// From the demo, slightly changed
struct ExampleAppLog
{
	ImGuiTextBuffer     Buf;
	ImGuiTextFilter     Filter;
	ImVector<int>       LineOffsets;        // Index to lines offset
	bool                ScrollToBottom;

	void Clear() { Buf.clear(); LineOffsets.clear(); }

	void
	AddLog(const char *fmt, va_list args)
	{
		int old_size = Buf.size();
		Buf.appendfv(fmt, args);
		for(int new_size = Buf.size(); old_size < new_size; old_size++)
			if(Buf[old_size] == '\n')
				LineOffsets.push_back(old_size);
		ScrollToBottom = true;
	}

	void
	AddLog(const char *fmt, ...) IM_FMTARGS(2)
	{
		va_list args;
		va_start(args, fmt);
		AddLog(fmt, args);
		va_end(args);
	}

	void
	Draw(const char *title, bool *p_open = nil)
	{
		ImGui::Begin(title, p_open);
		if(ImGui::Button("Clear")) Clear();
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -100.0f);
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0,0), false, ImGuiWindowFlags_HorizontalScrollbar);
		if(copy) ImGui::LogToClipboard();

		if(Filter.IsActive()){
			const char *buf_begin = Buf.begin();
			const char *line = buf_begin;
			for(int line_no = 0; line != nil; line_no++){
				const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : nil;
				if(Filter.PassFilter(line, line_end))
					ImGui::TextUnformatted(line, line_end);
				line = line_end && line_end[1] ? line_end + 1 : nil;
			}
		}else
			ImGui::TextUnformatted(Buf.begin());

		if(ScrollToBottom)
			ImGui::SetScrollHere(1.0f);
		ScrollToBottom = false;
		ImGui::EndChild();
		ImGui::End();
	}
};


static void
uiMainmenu(void)
{
	if(ImGui::BeginMainMenuBar()){
		if(ImGui::BeginMenu("File")){
			if(ImGui::MenuItem("Exit", "Alt+F4")) sk::globals.quit = 1;
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Window")){
			if(ImGui::MenuItem("Time & Weather", "T", showTimeWeatherWindow)) { showTimeWeatherWindow ^= 1; }
			if(ImGui::MenuItem("View", "V", showViewWindow)) { showViewWindow ^= 1; }
			if(ImGui::MenuItem("Rendering", "R", showRenderingWindow)) { showRenderingWindow ^= 1; }
			if(ImGui::MenuItem("Object Info", "I", showInstanceWindow)) { showInstanceWindow ^= 1; }
			if(ImGui::MenuItem("Editor ", nil, showEditorWindow)) { showEditorWindow ^= 1; }
			if(ImGui::MenuItem("Log ", nil, showLogWindow)) { showLogWindow ^= 1; }
			if(ImGui::MenuItem("Demo ", nil, showDemoWindow)) { showDemoWindow ^= 1; }
			if(ImGui::MenuItem("Help", nil, showHelpWindow)) { showHelpWindow ^= 1; }
			ImGui::EndMenu();
		}

		if(params.numAreas){
			ImGui::PushItemWidth(100);
			if(ImGui::BeginCombo("Area", params.areaNames[currentArea])){
				for(int n = 0; n < params.numAreas; n++){
					bool is_selected = n == currentArea;
					static char str[100];
					sprintf(str, "%d - %s", n, params.areaNames[n]);
					if(ImGui::Selectable(str, is_selected))
						currentArea = n;
					if(is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
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
		"Ctrl+Alt+MMB: zoom into target\n"
		"C: toggle viewer camera (longer far clip)"
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
	}

	ImGui::End();
}

static void
uiWeatherBox(const char *id, int *weather)
{
	if(ImGui::BeginCombo(id, params.weatherNames[*weather])){
		for(int n = 0; n < params.numWeathers; n++){
			bool is_selected = n == *weather;
			static char str[100];
			sprintf(str, "%d - %s", n, params.weatherNames[n]);
			if(ImGui::Selectable(str, is_selected))
				*weather = n;
			if(is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
}

static void
advanceHour(int diff)
{
	currentHour += diff;
	if(currentHour >= 24)
		currentHour = 0;
	else if(currentHour < 0)
		currentHour = 23;
}

static void
advanceMinute(int diff)
{
	currentMinute += diff;
	if(currentMinute >= 60){
		currentMinute = 0;
		advanceHour(1);
	}else if(currentMinute < 0){
		currentMinute = 59;
		advanceHour(-1);
	}
}

static void
uiTimeWeather(void)
{
	static int weatherWidth;
	if(weatherWidth == 0){
		int i, w;
		for(i = 0; i < params.numWeathers; i++){
			w = ImGui::CalcTextSize(params.weatherNames[i]).x;
			if(w > weatherWidth)
				weatherWidth = w;
		}
		weatherWidth += 30;
	}


	ImGui::PushItemWidth(100);
	ImGui::BeginGroup();
	ImGui::Text("Hour");
	ImGui::InputInt("##Hour", &currentHour, 1);
	advanceHour(0);
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("Minute");
	ImGui::InputInt("##Minute", &currentMinute, 1);
	advanceMinute(0);
	ImGui::EndGroup();


	ImGui::PushItemWidth(0);
	int totalMinute = currentHour*60 + currentMinute;
	ImGui::SliderInt("##TotalMinute", &totalMinute, 0, 24*60-1);
	currentHour = totalMinute/60;
	currentMinute = totalMinute%60;
	ImGui::PopItemWidth();

	if(params.daynightPipe){
		ImGui::SliderFloat("Day/Night Balance", &gDayNightBalance, 0.0f, 1.0f, "%.2f");
		if(gameplatform != PLATFORM_XBOX)
			ImGui::SliderFloat("Wet Road Effect", &gWetRoadEffect, 0.0f, 1.0f, "%.2f");
	}


	ImGui::PushItemWidth(weatherWidth);
	ImGui::BeginGroup();
	ImGui::Text("Weather A");
	uiWeatherBox("##WeatherA", &oldWeather);
	ImGui::EndGroup();
	ImGui::PopItemWidth();

	ImGui::SameLine();

	ImGui::BeginGroup();
	ImGui::Text("");
	ImGui::SliderFloat("##Interpolation", &weatherInterpolation, 0.0f, 1.0f, "%.2f");
	ImGui::EndGroup();

	ImGui::SameLine();

	ImGui::PushItemWidth(weatherWidth);
	ImGui::BeginGroup();
	ImGui::Text("Weather B");
	uiWeatherBox("##WeatherB", &newWeather);
	ImGui::EndGroup();
	ImGui::PopItemWidth();
	ImGui::PopItemWidth();

	if(params.timecycle != GAME_III)
		ImGui::SliderInt("Extracolour", &extraColours, -1, params.numExtraColours*params.numHours - 1);

	if(params.neoWorldPipe)
		ImGui::SliderFloat("Neo Light map", &gNeoLightMapStrength, 0.0f, 1.0f, "%.2f");
}

static void
uiView(void)
{
	ImGui::Checkbox("Draw Collisions", &gRenderCollision);
	if(params.timecycle == GAME_SA)
		ImGui::Checkbox("Draw TimeCycle boxes", &gRenderTimecycleBoxes);
	ImGui::Checkbox("Draw Zones", &gRenderZones);
	if(gRenderZones){
		ImGui::Indent();
		ImGui::Checkbox("Map Zones", &gRenderMapZones);
		switch(gameversion){
		case GAME_III:
			ImGui::Checkbox("Zones", &gRenderNavigZones);
			ImGui::Checkbox("Cull Zones", &gRenderCullZones);
			break;
		case GAME_VC:
			ImGui::Checkbox("Navig Zones", &gRenderNavigZones);
			ImGui::Checkbox("Info Zones", &gRenderInfoZones);
			break;
		case GAME_SA:
			ImGui::Checkbox("Navig Zones", &gRenderNavigZones);
			break;
		}
		ImGui::Checkbox("Attrib Zones", &gRenderAttribZones);
		ImGui::Unindent();
	}


	ImGui::Checkbox("Draw Water", &gRenderWater);
	if(gameversion == GAME_SA)
		ImGui::Checkbox("Play Animations", &gPlayAnimations);

	static int render = 0;
	ImGui::RadioButton("Render Normal", &render, 0);
	ImGui::RadioButton("Render only HD", &render, 1);
	ImGui::RadioButton("Render only LOD", &render, 2);
	gRenderOnlyHD = !!(render&1);
	gRenderOnlyLod = !!(render&2);
	ImGui::SliderFloat("Draw Distance", &TheCamera.m_LODmult, 0.5f, 3.0f, "%.3f");
	ImGui::Checkbox("Render all Timed Objects", &gNoTimeCull);
	if(params.numAreas)
		ImGui::Checkbox("Render all Areas", &gNoAreaCull);
}

static void
uiRendering(void)
{
	ImGui::Checkbox("Draw PostFX", &gRenderPostFX);
	if(params.timecycle == GAME_VC){
		ImGui::Checkbox("Use Blur Ambient", &gUseBlurAmb); ImGui::SameLine();
		ImGui::Checkbox("Override", &gOverrideBlurAmb);
	}
	if(params.timecycle == GAME_SA){
		ImGui::Text("Colour filter"); ImGui::SameLine();
		ImGui::RadioButton("None##NOPOSTFX", &gColourFilter, 0); ImGui::SameLine();
		ImGui::RadioButton("PS2##PS2POSTFX", &gColourFilter, PLATFORM_PS2); ImGui::SameLine();
		ImGui::RadioButton("PC/Xbox##PCPOSTFX", &gColourFilter, PLATFORM_PC); ImGui::SameLine();
		ImGui::Checkbox("Radiosity", &gRadiosity);
	}
	if(params.daynightPipe){
		ImGui::Text("Building Pipe"); ImGui::SameLine();
		ImGui::RadioButton("PS2##PS2BUILD", &gBuildingPipeSwitch, PLATFORM_PS2); ImGui::SameLine();
		ImGui::RadioButton("PC##PCBUILD", &gBuildingPipeSwitch, PLATFORM_PC); ImGui::SameLine();
		ImGui::RadioButton("Xbox##XBOXBUILD", &gBuildingPipeSwitch, PLATFORM_XBOX);
	}
	ImGui::Checkbox("Backface Culling", &gDoBackfaceCulling);
	// TODO: not params
	ImGui::Checkbox("PS2 Alpha test", &params.ps2AlphaTest);
	ImGui::InputInt("Alpha Ref", &params.alphaRef, 1);
	if(params.alphaRef < 0) params.alphaRef = 0;
	if(params.alphaRef > 255) params.alphaRef = 255;

	ImGui::Checkbox("Draw Background", &gRenderBackground);
	ImGui::Checkbox("Enable Fog", &gEnableFog);
	if(params.timecycle == GAME_SA)
		ImGui::Checkbox("Enable TimeCycle boxes", &gEnableTimecycleBoxes);
}

static void
uiInstInfo(ObjectInst *inst)
{
	ObjectDef *obj;
	obj = GetObjectDef(inst->m_objectId);

	static char buf[MODELNAMELEN];
	strncpy(buf, obj->m_name, MODELNAMELEN);
	ImGui::InputText("Model", buf, MODELNAMELEN);

	ImGui::Text("IPL: %s", inst->m_file->name);

	ImGui::Text("Translation: %.3f %.3f %.3f",
		inst->m_translation.x,
		inst->m_translation.y,
		inst->m_translation.z);
	ImGui::Text("Rotation: %.3f %.3f %.3f %.3f",
		inst->m_rotation.x,
		inst->m_rotation.y,
		inst->m_rotation.x,
		inst->m_rotation.w);
	if(params.numAreas)
		ImGui::Text("Area: %d", inst->m_area);

	if(params.objFlagset == GAME_SA){
		ImGui::Checkbox("Unimportant", &inst->m_isUnimportant);
		ImGui::Checkbox("Underwater", &inst->m_isUnderWater);
		ImGui::Checkbox("Tunnel", &inst->m_isTunnel);
		ImGui::Checkbox("Tunnel Transition", &inst->m_isTunnelTransition);
	}
}

static void
uiObjInfo(ObjectDef *obj)
{
	int i;
	TxdDef *txd;

	txd = GetTxdDef(obj->m_txdSlot);
	static char buf[MODELNAMELEN];

	ImGui::Text("ID: %d\n", obj->m_id);
	strncpy(buf, obj->m_name, MODELNAMELEN);
	ImGui::InputText("Model", buf, MODELNAMELEN);
	strncpy(buf, txd->name, MODELNAMELEN);
	ImGui::InputText("TXD", buf, MODELNAMELEN);

	ImGui::Text("IDE: %s", obj->m_file->name);
	if(obj->m_colModel && !obj->m_gotChildCol)
		ImGui::Text("COL: %s", obj->m_colModel->file->name);

	ImGui::Text("Draw dist:");
	for(i = 0; i < obj->m_numAtomics; i++){
		ImGui::SameLine();
		ImGui::Text("%.0f", obj->m_drawDist[i]);
	}

	if(obj->m_isTimed){
		ImGui::Text("Time: %d %d (visible now: %s)",
			obj->m_timeOn, obj->m_timeOff,
			IsHourInRange(obj->m_timeOn, obj->m_timeOff) ? "yes" : "no");
	}

	switch(params.objFlagset){
	case GAME_III:
		ImGui::Checkbox("Normal cull", &obj->m_normalCull);
		ImGui::Checkbox("No Fade", &obj->m_noFade);
		ImGui::Checkbox("Draw Last", &obj->m_drawLast);
		ImGui::Checkbox("Additive Blend", &obj->m_additive);
		if(obj->m_additive) obj->m_drawLast = true;
		ImGui::Checkbox("Is Subway", &obj->m_isSubway);
		ImGui::Checkbox("Ignore Light", &obj->m_ignoreLight);
		ImGui::Checkbox("No Z-write", &obj->m_noZwrite);
		break;

	case GAME_VC:
		ImGui::Checkbox("Wet Road Effect", &obj->m_wetRoadReflection);
		ImGui::Checkbox("No Fade", &obj->m_noFade);
		ImGui::Checkbox("Draw Last", &obj->m_drawLast);
		ImGui::Checkbox("Additive Blend", &obj->m_additive);
		if(obj->m_additive) obj->m_drawLast = true;
//		ImGui::Checkbox("Is Subway", &obj->m_isSubway);
		ImGui::Checkbox("Ignore Light", &obj->m_ignoreLight);
		ImGui::Checkbox("No Z-write", &obj->m_noZwrite);
		ImGui::Checkbox("No shadows", &obj->m_noShadows);
		ImGui::Checkbox("Ignore Draw Dist", &obj->m_ignoreDrawDist);
		ImGui::Checkbox("Code Glass", &obj->m_isCodeGlass);
		ImGui::Checkbox("Artist Glass", &obj->m_isArtistGlass);
		break;

	case GAME_SA:
		ImGui::Checkbox("Draw Last", &obj->m_drawLast);
		ImGui::Checkbox("Additive Blend", &obj->m_additive);
		if(obj->m_additive) obj->m_drawLast = true;
		ImGui::Checkbox("No Z-write", &obj->m_noZwrite);
		ImGui::Checkbox("No shadows", &obj->m_noShadows);
		ImGui::Checkbox("No Backface Culling", &obj->m_noBackfaceCulling);
		if(obj->m_type == ObjectDef::ATOMIC){
			ImGui::Checkbox("Wet Road Effect", &obj->m_wetRoadReflection);
			ImGui::Checkbox("Don't collide with Flyer", &obj->m_dontCollideWithFlyer);

			static int flag = 0;
			flag = (int)obj->m_isCodeGlass |
				(int)obj->m_isArtistGlass<<1 |
				(int)obj->m_isGarageDoor<<2 |
				(int)obj->m_isDamageable<<3 |
				(int)obj->m_isTree<<4 |
				(int)obj->m_isPalmTree<<5 |
				(int)obj->m_isTag<<6 |
				(int)obj->m_noCover<<7 |
				(int)obj->m_wetOnly<<8;
			ImGui::RadioButton("None", &flag, 0);
			ImGui::RadioButton("Code Glass", &flag, 1);
			ImGui::RadioButton("Artist Glass", &flag, 2);
			ImGui::RadioButton("Garage Door", &flag, 4);
			if(!obj->m_isTimed)
				ImGui::RadioButton("Damageable", &flag, 8);
			ImGui::RadioButton("Tree", &flag, 0x10);
			ImGui::RadioButton("Palm Tree", &flag, 0x20);
			ImGui::RadioButton("Tag", &flag, 0x40);
			ImGui::RadioButton("No Cover", &flag, 0x80);
			ImGui::RadioButton("Wet Only", &flag, 0x100);
			obj->m_isCodeGlass = !!(flag & 1);
			obj->m_isArtistGlass = !!(flag & 2);
			obj->m_isGarageDoor = !!(flag & 4);
			obj->m_isDamageable = !!(flag & 8);
			obj->m_isTree = !!(flag & 0x10);
			obj->m_isPalmTree = !!(flag & 0x20);
			obj->m_isTag = !!(flag & 0x40);
			obj->m_noCover = !!(flag & 0x80);
			obj->m_wetOnly = !!(flag & 0x100);
		}else if(obj->m_type == ObjectDef::CLUMP){
			ImGui::Checkbox("Door", &obj->m_isDoor);
		}
		break;
	}
}

static void
uiEditorWindow(void)
{
	CPtrNode *p;
	ObjectInst *inst;
	ObjectDef *obj;

	ImGui::Begin("Editor Window", &showEditorWindow);

	if(ImGui::TreeNode("Camera")){
		ImGui::InputFloat3("Cam position", (float*)&TheCamera.m_position);
		ImGui::InputFloat3("Cam target", (float*)&TheCamera.m_target);
		ImGui::Text("Far: %f", Timecycle::currentColours.farClp);
		ImGui::TreePop();
	}

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

static void
uiInstWindow(void)
{
	ImGui::Begin("Object Info", &showInstanceWindow);
	if(selection.first){
		ObjectInst *inst = (ObjectInst*)selection.first->item;
		if(ImGui::CollapsingHeader("Instance"))
			uiInstInfo(inst);
		if(ImGui::CollapsingHeader("Object"))
			uiObjInfo(GetObjectDef(inst->m_objectId));
	}
	ImGui::End();
}

static void
uiTest(void)
{
	ImGuiContext &g = *GImGui;
	int y = g.FontBaseSize + g.Style.FramePadding.y * 2.0f;	// height of main menu
	ImGui::SetNextWindowPos(ImVec2(0, y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(200, sk::globals.height-y), ImGuiCond_Always);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::Begin("Dock", nil, ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize);
	ImGui::Text("hi there");
	if(ImGui::IsWindowFocused())
		ImGui::Text("focus");
	if(ImGui::IsMouseDragging())
		ImGui::Text("drag");
	if(ImGui::IsWindowHovered())
		ImGui::Text("hover");
	ImGui::End();
	ImGui::PopStyleVar();
}

static ExampleAppLog logwindow;
void addToLogWindow(const char *fmt, va_list args) { logwindow.AddLog(fmt, args); }

void
gui(void)
{
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	uiMainmenu();

	if(CPad::IsKeyJustDown('C')) gUseViewerCam = !gUseViewerCam;

	if(CPad::IsKeyJustDown('T')) showTimeWeatherWindow ^= 1;
	if(showTimeWeatherWindow){
		ImGui::Begin("Time & Weather", &showTimeWeatherWindow);
		uiTimeWeather();
		ImGui::End();
	}

	if(CPad::IsKeyJustDown('V')) showViewWindow ^= 1;
	if(showViewWindow){
		ImGui::Begin("View", &showViewWindow);
		uiView();
		ImGui::End();
	}

	if(CPad::IsKeyJustDown('R')) showRenderingWindow ^= 1;
	if(showRenderingWindow){
		ImGui::Begin("Rendering", &showRenderingWindow);
		uiRendering();
		ImGui::End();
	}

	if(CPad::IsKeyJustDown('I')) showInstanceWindow ^= 1;
	if(showInstanceWindow) uiInstWindow();

	if(showEditorWindow) uiEditorWindow();
	if(showHelpWindow) uiHelpWindow();
	if(showDemoWindow){
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
		ImGui::ShowDemoWindow(&showDemoWindow);
	}

	if(showLogWindow) logwindow.Draw("Log", &showLogWindow);

//	uiTest();
}
