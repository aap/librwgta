#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>
#include <skeleton.h>
#include <args.h>

#include <rwgta.h>
#include "imgui/ImGuizmo.h"
#include "sol/sol.hpp"

#include "stuff.h"

using namespace std;

static rw::Raster*
ConvertTexRaster(rw::Raster *ras)
{
	using namespace rw;

	if(ras->platform == rw::platform)
		return ras;
	// compatible platforms
	if(ras->platform == PLATFORM_D3D8 && rw::platform == PLATFORM_D3D9 ||
	   ras->platform == PLATFORM_D3D9 && rw::platform == PLATFORM_D3D8)
		return ras;

	Image *img = ras->toImage();
	ras->destroy();
	img->unpalettize();
	ras = Raster::createFromImage(img);
	img->destroy();
	return ras;
}

void
ConvertTxd(rw::TexDictionary *txd)
{
	rw::Texture *tex;
	FORLIST(lnk, txd->textures){
		tex = rw::Texture::fromDict(lnk);
		rw::Raster *ras = tex->raster;
		if(ras)
			tex->raster = ConvertTexRaster(ras);
		tex->setFilter(rw::Texture::LINEAR);
	}
}

void
ConvertClump(rw::Clump *clump)
{
	FORLIST(lnk, clump->atomics){
		rw::Atomic *atm = rw::Atomic::fromClump(lnk);
		gta::attachCustomPipelines(atm);
		int32 driver = rw::platform;
		int32 platform = rw::findPlatform(atm);
		if(platform){
			rw::platform = platform;
			rw::switchPipes(atm, rw::platform);
		}
		if(atm->geometry->flags & rw::Geometry::NATIVE)
			atm->uninstance();
		rw::ps2::unconvertADC(atm->geometry);
		rw::platform = driver;

void myRenderCB(rw::Atomic *atomic);
		atm->setRenderCB(myRenderCB);
	}
}

void
registerSkeleton(sol::state &lua)
{
	sol::table sktab = lua["sk"].get_or_create<sol::table>();

	lua.new_usertype<Camera>("Camera",
		"new", sol::constructors<Camera()>(),
		"update", &Camera::update,
		"turn", &Camera::turn,
		"orbit", &Camera::orbit,
		"zoom", &Camera::zoom,
		"dolly", &Camera::dolly,
		"pan", &Camera::pan,
		"fov", &Camera::m_fov,
		"aspectRatio", &Camera::m_aspectRatio,
		"near", &Camera::m_near,
		"far", &Camera::m_far,
		"position", &Camera::m_position,
		"target", &Camera::m_target,
		"rwCamera", &Camera::m_rwcam,
		"setDistance", [](Camera *cam, float d) {
			rw::V3d fwd = normalize(sub(cam->m_target, cam->m_position));
			cam->m_target = add(cam->m_position, scale(fwd, d));
		},
		"distanceTo", &Camera::distanceTo,
		"isSphereVisible", [](Camera *cam, const gta::CSphere &sph, rw::Matrix *xform) {
			rw::Sphere rwsph;
			rwsph.center = sph.center;
			rwsph.radius = sph.radius;
			return cam->isSphereVisible(&rwsph, xform);
		},
		"getHeading", &Camera::getHeading,
		"jumpTo", [](Camera *cam, rw::V3d pos) {
			rw::V3d dist = sub(pos, cam->m_target);
			cam->m_position = add(cam->m_position, dist);
			cam->m_target = add(cam->m_target, dist);
		}
	);
	sktab.set("KEY_ESC", sk::KEY_ESC);
	sktab.set("KEY_UP", sk::KEY_UP);
	sktab.set("KEY_DOWN", sk::KEY_DOWN);
	sktab.set("KEY_LEFT", sk::KEY_LEFT);
	sktab.set("KEY_RIGHT", sk::KEY_RIGHT);
	sktab.set("KEY_LSHIFT", sk::KEY_LSHIFT);
	sktab.set("KEY_RSHIFT", sk::KEY_RSHIFT);
	sktab.set("KEY_LCTRL", sk::KEY_LCTRL);
	sktab.set("KEY_RCTRL", sk::KEY_RCTRL);
	sktab.set("KEY_CAPSLK", sk::KEY_CAPSLK);
	sktab.set("KEY_LALT", sk::KEY_LALT);
	sktab.set("KEY_RALT", sk::KEY_RALT);
	char keystr[] = "KEY_.";
	for(int c = 'A'; c <= 'Z'; c++) {
		keystr[4] = c;
		sktab.set(keystr, c);
	}
	// ...

	sktab.set_function("CameraSize", [](rw::Camera *cam, int w, int h) {
		rw::Rect r;
		r.w = w;
		r.h = h;
		sk::CameraSize(cam, &r);
	});
	sktab.set_function("CameraCreate", &sk::CameraCreate);

	sktab.set_function("ImGuiBeginFrame", [](float timestep) {
		ImGui_ImplRW_NewFrame(timestep);
		ImGuizmo::BeginFrame();
	});
	sktab.set_function("ImGuiEndFrame", [](void) {
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplRW_RenderDrawLists(ImGui::GetDrawData());
	});
	sktab.set_function("ConvertTexDict", &ConvertTxd);
	sktab.set_function("ConvertClump", &ConvertClump);
}
