#include <rw.h>
#include <skeleton.h>
#include <rwgta.h>
#include <string.h>
#include <assert.h>

#include "imgui/imgui_internal.h"
#include "sol/sol.hpp"
#include "sol/sol_ImGui.h"
#include "stuff.h"
#include "imgui/ImGuizmo.h"

sol::state lua;
rw::EngineOpenParams engineOpenParams;

void
luaError(const char *msg)
{
	fprintf(stderr, "Lua error: %s\n", msg);
	auto f = lua["conprint"];
	if(f.valid())
		f(std::string("Error: ") + msg);
}

void
Init(void)
{
	sk::globals.windowtitle = "Selanna";
	sk::globals.width = 1280;
	sk::globals.height = 800;
	sk::globals.quit = 0;

	initLua(lua);
	registerRW(lua);
	registerGTA(lua);
	registerSkeleton(lua);
	registerAlias(lua);
void registerDebugRender(sol::state &lua);
	registerDebugRender(lua);
	sol_ImGui::Init(lua);
}

static int32 txdStoreOffset;

void
TxdSetParent(rw::TexDictionary *child, rw::TexDictionary *parent)
{
	*PLUGINOFFSET(rw::TexDictionary*, child, txdStoreOffset) = parent;
}

rw::Texture*
TxdStoreFindCB(const char *name)
{
	rw::TexDictionary *txd = rw::TexDictionary::getCurrent();
	rw::Texture *tex;
	while(txd){
		tex = txd->find(name);
		if(tex) return tex;
		txd = *PLUGINOFFSET(rw::TexDictionary*, txd, txdStoreOffset);
	}
	return nil;
}

static void*
createTxdStore(void *object, int32 offset, int32)
{
	*PLUGINOFFSET(rw::TexDictionary*, object, offset) = nil;
	return object;
}

static void*
copyTxdStore(void *dst, void *src, int32 offset, int32)
{
	*PLUGINOFFSET(rw::TexDictionary*, dst, offset) = *PLUGINOFFSET(rw::TexDictionary*, src, offset);
	return dst;
}

static void*
destroyTxdStore(void *object, int32, int32)
{
	return object;
}

void
RegisterTexStorePlugin(void)
{
	txdStoreOffset = rw::TexDictionary::registerPlugin(sizeof(void*), gta::ID_TXDSTORE,
			createTxdStore,
			destroyTxdStore,
			copyTxdStore);
	rw::Texture::findCB = TxdStoreFindCB;
}


bool
attachPlugins(void)
{
	gta::attachPlugins();
	// probably should go into the thing above
	RegisterTexStorePlugin();
	return true;
}

rw::RGBA highlightColor;
rw::ObjPipeline *colourCodePipe;

void
myRenderCB(rw::Atomic *atomic)
{
	using namespace rw;
	if(gta::renderColourCoded)
		colourCodePipe->render(atomic);
	else if(highlightColor.red || highlightColor.green || highlightColor.blue){
		atomic->getPipeline()->render(atomic);
		gta::colourCode = highlightColor;
		gta::colourCode.alpha = 128;
		int32 zwrite, fog, aref;
		zwrite = GetRenderState(rw::ZWRITEENABLE);
		fog = rw::GetRenderState(rw::FOGENABLE);
		aref = rw::GetRenderState(rw::ALPHATESTREF);
		SetRenderState(rw::ZWRITEENABLE, 0);
		SetRenderState(rw::FOGENABLE, 0);
		SetRenderState(rw::ALPHATESTREF, 10);
		colourCodePipe->render(atomic);
		SetRenderState(rw::ZWRITEENABLE, zwrite);
		SetRenderState(rw::FOGENABLE, fog);
		SetRenderState(rw::ALPHATESTREF, aref);
	}else
		atomic->getPipeline()->render(atomic);
}

static rw::RawMatrix gizobj;

void
gizmoUse(ImGuizmo::OPERATION operation, ImGuizmo::MODE mode, float snap,
         rw::Camera *cam, float rx, float ry, float rw_, float rh)
{
	rw::Matrix view;
	float *fview, *fproj, *fobj;

	rw::Matrix::invert(&view, cam->getFrame()->getLTM());
	fview = (float*)&cam->devView;
	fproj = (float*)&cam->devProj;
	fobj  = (float*)&gizobj;

	float snap3[3] = { snap, snap, snap };
	ImGuizmo::SetRect(rx, ry, rw_, rh);
	ImGuizmo::Manipulate(fview, fproj, operation, mode, fobj, nil, snap3);
}

bool
InitRW(void)
{
//	rw::platform = rw::PLATFORM_D3D8;
	if(!sk::InitRW())
		return false;
	rw::Texture::setCreateDummies(1);
	rw::d3d::isP8supported = 0;
	rw::Image::setSearchPath("textures/");

	colourCodePipe = gta::makeColourCodePipeline();
	gta::makeCustomBuildingPipelines();
	gta::makeCustomCarPipeline();

	lua["gWidth"] = sk::globals.width;
	lua["gHeight"] = sk::globals.height;

	sol::table gizmotab = lua["gizmo"].get_or_create<sol::table>();
	gizmotab.set_function("Use", [](ImGuizmo::OPERATION op, ImGuizmo::MODE mode, float snap,
	                                    rw::Camera *cam, float rx, float ry, float rw_, float rh) {
		gizmoUse(op, mode, snap, cam, rx, ry, rw_, rh);
	});
	gizmotab.set_function("Init", [](rw::V3d pos, rw::Quat rot) {
		rw::Matrix m;
		rw::Matrix::makeRotation(&m, rot);
		rw::convMatrix(&gizobj, &m);
		gizobj.pos = pos;
	});
	gizmotab.set_function("GetXform", []() -> std::tuple<rw::V3d, rw::Quat> {
		rw::Matrix m;
		rw::convMatrix(&m, &gizobj);
		return { gizobj.pos, m.getRotation() };
	});
	gizmotab.set_function("InitMatrix", [](rw::Matrix *m) {
		rw::convMatrix(&gizobj, m);
	});
	gizmotab.set_function("GetMatrix", []() -> rw::Matrix {
		rw::Matrix m;
		rw::convMatrix(&m, &gizobj);
		return m;
	});
	gizmotab.set_function("IsUsing", []() { return ImGuizmo::IsUsing(); });
	gizmotab.set("ROTATE", ImGuizmo::ROTATE);
	gizmotab.set("TRANSLATE", ImGuizmo::TRANSLATE);
	gizmotab.set("SCALE", ImGuizmo::SCALE);
	gizmotab.set("UNIVERSAL", ImGuizmo::UNIVERSAL);
	gizmotab.set("LOCAL", ImGuizmo::LOCAL);
	gizmotab.set("WORLD", ImGuizmo::WORLD);

	rw::Matrix tmp;
	tmp.setIdentity();
	convMatrix(&gizobj, &tmp);
	gizobj.pos = rw::makeV3d(0.0f, 0.0f, 0.0f);

	sol::table rwtab = lua["rw"].get_or_create<sol::table>();
	rwtab.set_function("imageTexture", [](rw::Texture *tex, float w, float h) {
		ImGui::Image((ImTextureID)(void*)tex, ImVec2(w, h));
	});

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.05f, 0.05f, 0.94f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.10f, 0.07f, 0.07f, 0.94f);
	colors[ImGuiCol_Border]                 = ImVec4(0.50f, 0.30f, 0.30f, 0.50f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.30f, 0.10f, 0.10f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.70f, 0.18f, 0.18f, 0.40f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.80f, 0.18f, 0.18f, 0.67f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.04f, 0.03f, 0.03f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.35f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.55f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.70f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.35f, 0.35f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.72f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.90f, 0.30f, 0.30f, 1.00f);
	colors[ImGuiCol_Button]                 = ImVec4(0.80f, 0.18f, 0.18f, 0.40f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.80f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.65f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.80f, 0.18f, 0.18f, 0.31f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.80f, 0.18f, 0.18f, 0.80f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.80f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.65f, 0.18f, 0.18f, 0.78f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.75f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.80f, 0.18f, 0.18f, 0.20f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.80f, 0.18f, 0.18f, 0.67f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.80f, 0.18f, 0.18f, 0.95f);
	colors[ImGuiCol_InputTextCursor]        = colors[ImGuiCol_Text];
	colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered];
	colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header],       colors[ImGuiCol_TitleBgActive], 0.80f);
	colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
	colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab],          colors[ImGuiCol_TitleBg], 0.80f);
	colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected],  colors[ImGuiCol_TitleBg], 0.40f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.30f, 0.30f, 0.00f);
	colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.20f, 0.12f, 0.12f, 1.00f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.35f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.25f, 0.15f, 0.15f, 1.00f);
	colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive];
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.80f, 0.18f, 0.18f, 0.35f);
	colors[ImGuiCol_TreeLines]              = colors[ImGuiCol_Border];
	colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 0.80f, 0.00f, 0.90f);
	colors[ImGuiCol_NavCursor]              = ImVec4(0.80f, 0.18f, 0.18f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

	if(sk::args.argc > 1)
		execLua(sk::args.argv[1]);
	else
		execLua("init.lua");
	luaCall("Init");
	return true;
}



sk::EventStatus
AppEventHandler(sk::Event e, void *param)
{
	using namespace sk;
	Rect *r;
	MouseState *ms;

	ImGuiEventHandler(e, param);
	ImGuiIO &io = ImGui::GetIO();
	switch(e){
	case INITIALIZE:
		Init();
		return EVENTPROCESSED;
	case RWINITIALIZE:
		return ::InitRW() ? EVENTPROCESSED : EVENTERROR;
	case PLUGINATTACH:
		return attachPlugins() ? EVENTPROCESSED : EVENTERROR;
	case KEYDOWN:
//		if(!io.WantCaptureKeyboard && !io.WantTextInput && !ImGuizmo::IsOver())
		if(!io.WantCaptureKeyboard && !io.WantTextInput)
			luaCall("KeyDown", *(int*)param);
		return EVENTPROCESSED;
	case KEYUP:
		luaCall("KeyUp", *(int*)param);
		return EVENTPROCESSED;

	case MOUSEBTN:
		if(!io.WantCaptureMouse && !ImGuizmo::IsOver()) {
			ms = (MouseState*)param;
			luaCall("MouseBtn", ms->buttons);
		}else
			luaCall("MouseBtn", 0);
		return EVENTPROCESSED;
	case MOUSEMOVE:
		ms = (MouseState*)param;
		luaCall("MouseMotion", ms->posx, ms->posy);
		return EVENTPROCESSED;

	case MOUSEWHEEL:
		if(!io.WantCaptureMouse) {
			ms = (MouseState*)param;
			luaCall("MouseWheel", ms->wheelDelta);
		}
		return EVENTPROCESSED;


	case RESIZE:
		r = (Rect*)param;
		if(r->w == 0) r->w = 1;
		if(r->h == 0) r->h = 1;
		lua["gWidth"] = sk::globals.width = r->w;
		lua["gHeight"] = sk::globals.height = r->h;
		luaCall("Resize", r->w, r->h);
		break;
	case IDLE:
		lua["gFramerate"] = ImGui::GetIO().Framerate;
		lua["gDeltaTime"] = ImGui::GetIO().DeltaTime;
		luaCall("Draw", *(float*)param);
		return EVENTPROCESSED;
	}
	return sk::EVENTNOTPROCESSED;
}
