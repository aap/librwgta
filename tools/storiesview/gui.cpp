#include "storiesview.h"

static bool showTimeWeatherWindow;
static bool showViewWindow;

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
	ImGui::RadioButton("Render HD", &drawLOD, 0);
	ImGui::RadioButton("Render LOD", &drawLOD, 1);

	ImGui::Checkbox("Render Cubes", &drawCubes);
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
}
