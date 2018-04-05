#include "storiesview.h"

static bool showTimeWeatherWindow;
static bool showViewWindow;
static bool showObjectWindow;

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

	ImGui::PopItemWidth();


	ImGui::InputInt("Weather", &currentWeather, 1);
	if(currentWeather >= 8) currentWeather = -1;
	if(currentWeather < -1) currentWeather = 7;

	ImGui::InputInt("Area", &currentArea, 1);
	if(currentArea >= gLevel->chunk->numInteriors) currentArea = -1;
	if(currentArea < -1) currentArea = gLevel->chunk->numInteriors-1;
}

static void
uiView(void)
{
	ImGui::Checkbox("Render World", &drawWorld);
	ImGui::Checkbox("Render LOD", &drawLOD);

	ImGui::NewLine();

	ImGui::Checkbox("Render Collision", &drawCol);
	ImGui::Checkbox("Render Solid Bounds", &drawBounds);

	ImGui::NewLine();

	ImGui::Checkbox("Render only Unmatched", &drawUnmatched);
}

static void
entityInfo(CEntity *e)
{
	static char tmp[64];
	int i;

	CBaseModelInfo *mi = CModelInfo::Get(e->modelIndex);
	EntityExt *ee = (EntityExt*)e->vtable;

	ImGui::Text("model: %s", mi->name);

	sprintf(tmp, "ipl: 0x%X", ee->GetIplID());
	ImGui::PushID(e);
	ImGui::Selectable(tmp);
	ImGui::PopID();
	if(ImGui::IsItemHovered())
		ee->highlight = 1;

	for(i = 0; i < ee->n; i++){
		sprintf(tmp, "  %p", ee->insts[i]);
		ImGui::PushID(ee->insts[i]);
		ImGui::Selectable(tmp);
		ImGui::PopID();
		if(ImGui::IsItemHovered())
			ee->insts[i]->highlight = 1;
	}
}

static void
uiObject(void)
{
	int i;
	char tmp[64];

	BuildingExt *b = BuildingExt::GetSelection();
	if(b && ImGui::TreeNode("Building")){
		ImGui::Text("%p", b);
		if(b->iplId >= 0){
			CEntity *e = GetEntityById(b->iplId);
			entityInfo(e);
		}
		ImGui::TreePop();
	}

	CEntity *e = EntityExt::GetSelection();
	if(e && ImGui::TreeNode("Entity")){
		entityInfo(e);
		ImGui::TreePop();
	}

	if(ImGui::Button("Write Links"))
		WriteLinks();

	if(b && ImGui::Button("Unlink"))
		b->SetEntity(-1);

	if(EntityExt::selection.count() != 1 ||
	   BuildingExt::selection.count() != 1)
		return;

	static char *failtext = "";	// not used after all....
	if(ImGui::Button("Link")){
		EntityExt *ee = (EntityExt*)e->vtable;
		printf("%p %x\n", b, ee->GetIplID());
		b->SetEntity(ee->GetIplID());
	}else if(!ImGui::IsItemHovered())
		failtext = "";
	ImGui::Text(failtext);
}

void
gui(void)
{
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

	if(CPad::IsKeyJustDown('O')) showObjectWindow ^= 1;
	if(showObjectWindow){
		ImGui::Begin("Object", &showObjectWindow);
		uiObject();
		ImGui::End();
	}

	if(CPad::IsKeyJustDown('H')){
		drawCol = !drawCol;
		drawWorld = !drawCol;
	}
	if(CPad::IsKeyJustDown('B'))
		drawBounds = !drawBounds;
	if(CPad::IsKeyJustDown('X') &&
	   EntityExt::selection.count() == 1 &&
	   BuildingExt::selection.count() == 1){
		BuildingExt *b = BuildingExt::GetSelection();
		CEntity *e = EntityExt::GetSelection();
		EntityExt *ee = (EntityExt*)e->vtable;
		printf("%p %x\n", b, ee->GetIplID());
		b->SetEntity(ee->GetIplID());
	}
}
