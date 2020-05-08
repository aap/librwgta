#define WITH_D3D
#include "euryopa.h"

using namespace rw;
using namespace RWDEVICE;

Texture *backBufferTex;
Raster *backBuffer;

static short indices[] = {
	2, 3, 1, 2, 1, 0
};
Im2DVertex verts[4];

static void
CreateImmediateData(void)
{
	Camera *cam = Scene.camera;
	int w = sk::globals.width;
	int h = sk::globals.height;
	float recipZ = 1.0f/cam->nearPlane;

	verts[0].setScreenX(0-0.5f);
	verts[0].setScreenY(0-0.5f);
	verts[0].setScreenZ(rw::im2d::GetNearZ());
	verts[0].setCameraZ(cam->nearPlane);
	verts[0].setRecipCameraZ(recipZ);
	verts[0].setColor(255, 255, 255, 255);
	verts[0].setU(0.0f, recipZ);
	verts[0].setV(0.0f, recipZ);

	verts[1].setScreenX(w-0.5f);
	verts[1].setScreenY(0-0.5f);
	verts[1].setScreenZ(rw::im2d::GetNearZ());
	verts[1].setCameraZ(cam->nearPlane);
	verts[1].setRecipCameraZ(recipZ);
	verts[1].setColor(255, 255, 255, 255);
	verts[1].setU(1.0f, recipZ);
	verts[1].setV(0.0f, recipZ);

	verts[2].setScreenX(0-0.5f);
	verts[2].setScreenY(h-0.5f);
	verts[2].setScreenZ(rw::im2d::GetNearZ());
	verts[2].setCameraZ(cam->nearPlane);
	verts[2].setRecipCameraZ(recipZ);
	verts[2].setColor(255, 255, 255, 255);
	verts[2].setU(0.0f, recipZ);
	verts[2].setV(1.0f, recipZ);

	verts[3].setScreenX(w-0.5f);
	verts[3].setScreenY(h-0.5f);
	verts[3].setScreenZ(rw::im2d::GetNearZ());
	verts[3].setCameraZ(cam->nearPlane);
	verts[3].setRecipCameraZ(recipZ);
	verts[3].setColor(255, 255, 255, 255);
	verts[3].setU(1.0f, recipZ);
	verts[3].setV(1.0f, recipZ);
}

static void
InitPostFX(void)
{
	if(backBuffer)
		backBuffer->destroy();
	backBuffer = Raster::create(sk::globals.width, sk::globals.height, 32, Raster::C888|Raster::CAMERATEXTURE);
	if(backBufferTex == nil)
		backBufferTex = Texture::create(backBuffer);
	else
		backBufferTex->raster = backBuffer;
}

#ifdef RW_D3D9

using namespace d3d;
using namespace d3d9;

static void
GetBackbuffer(Raster *raster)
{
	Raster::pushContext(raster);
	engine->currentCamera->frameBuffer->renderFast(0, 0);
	Raster::popContext();
}

static void *colourfilterIII_PS;
static void *colourfilterVC_PS;
static void *colourfilterLeedsPS2_PS;
static void *colourfilterLeedsPSP_PS;
static void *colourfilterSAPS2_PS;
static void *colourfilterSAPC_PS;
static void *radiosityPS;
static void *radiosityLeedsPS;
static bool shadersInitialized;

static void
CreateShaders(void)
{
#include "d3d_shaders/colourfilterIII_PS.inc"
	colourfilterIII_PS = createPixelShader(colourfilterIII_PS_cso);
#include "d3d_shaders/colourfilterVC_PS.inc"
	colourfilterVC_PS = createPixelShader(colourfilterVC_PS_cso);
#include "d3d_shaders/colourfilterLeedsPS2_PS.inc"
	colourfilterLeedsPS2_PS = createPixelShader(colourfilterLeedsPS2_PS_cso);
#include "d3d_shaders/colourfilterLeedsPSP_PS.inc"
	colourfilterLeedsPSP_PS = createPixelShader(colourfilterLeedsPSP_PS_cso);
#include "d3d_shaders/colourfilterSAPS2_PS.inc"
	colourfilterSAPS2_PS = createPixelShader(colourfilterSAPS2_PS_cso);
#include "d3d_shaders/colourfilterSAPC_PS.inc"
	colourfilterSAPC_PS = createPixelShader(colourfilterSAPC_PS_cso);
#include "d3d_shaders/radiosityPS.inc"
	radiosityPS = createPixelShader(radiosityPS_cso);
#include "d3d_shaders/radiosityLeedsPS.inc"
	radiosityLeedsPS = createPixelShader(radiosityLeedsPS_cso);

	shadersInitialized = true;
}

static void
RenderColourFilterGeneric(void *ps)
{
	d3d::im2dOverridePS = ps;
	im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&verts, 4, &indices, 6);
	d3d::im2dOverridePS = nil;
}

static void
RenderColourFilterIII(void)
{
	RenderColourFilterGeneric(colourfilterIII_PS);
}

static void
RenderColourFilterVC(void)
{
	RenderColourFilterGeneric(colourfilterVC_PS);
}

static void
RenderColourFilterLeedsPS2(void)
{
	RenderColourFilterGeneric(colourfilterLeedsPS2_PS);
}

static void
RenderColourFilterLeedsPSP(void)
{
	RenderColourFilterGeneric(colourfilterLeedsPSP_PS);
}

static void
RenderColourFilterSAPS2(void)
{
	RenderColourFilterGeneric(colourfilterSAPS2_PS);
}

static void
RenderColourFilterSAPC(void)
{
	RenderColourFilterGeneric(colourfilterSAPC_PS);
}

static void
RenderRadiosity(void)
{
	RenderColourFilterGeneric(radiosityPS);
}

static void
RenderRadiosityLeeds(void)
{
	RenderColourFilterGeneric(radiosityLeedsPS);
}

void
RenderPostFX(void)
{
	bool32 changedBackbuf;
	if(backBuffer == nil ||
	   backBuffer->width != sk::globals.width ||
	   backBuffer->height != sk::globals.height)
		InitPostFX();
	GetBackbuffer(backBuffer);
	changedBackbuf = 0;

	CreateImmediateData();

	if(!shadersInitialized)
		CreateShaders();

	float postfxvars[4];
	d3ddevice->SetPixelShaderConstantF(0, (float*)&Timecycle::currentColours.postfx1, 1);
	d3ddevice->SetPixelShaderConstantF(1, (float*)&Timecycle::currentColours.postfx2, 1);
	postfxvars[0] = Timecycle::currentColours.radiosityLimit/255.0f;
	postfxvars[1] = Timecycle::currentColours.radiosityIntensity/255.0f;
	postfxvars[2] = 1.0f;		// render passes
	d3ddevice->SetPixelShaderConstantF(2, (float*)postfxvars, 1);

	rw::SetRenderStatePtr(TEXTURERASTER, backBufferTex->raster);
	rw::SetRenderState(rw::TEXTUREFILTER, Texture::LINEAR);
	// we may want to blur later on
	SetRenderState(TEXTUREADDRESS, Texture::CLAMP);

	SetRenderState(VERTEXALPHA, 0);
	SetRenderState(ZTESTENABLE, 0);
	SetRenderState(ZWRITEENABLE, 0);


	switch(params.timecycle){
	case GAME_III:
		RenderColourFilterIII();
		changedBackbuf = 1;
		break;
	case GAME_VC:
		RenderColourFilterVC();
		changedBackbuf = 1;
		break;
	case GAME_SA:
		if(gColourFilter == PLATFORM_PS2){
			RenderColourFilterSAPS2();
			changedBackbuf = 1;
		}else if(gColourFilter == PLATFORM_PC || gColourFilter == PLATFORM_XBOX){
			RenderColourFilterSAPC();
			changedBackbuf = 1;
		}
		break;
	}

	if(gRadiosity){
		if(changedBackbuf)
			GetBackbuffer(backBuffer);
		switch(params.timecycle){
		case GAME_SA:
			RenderRadiosity();
			changedBackbuf = 1;
			break;
		case GAME_VCS:
			RenderRadiosityLeeds();
			changedBackbuf = 1;
			break;
		}
	}

	switch(params.timecycle)
	case GAME_LCS:
	case GAME_VCS:
		if(gColourFilter == PLATFORM_PS2){
			if(changedBackbuf)
				GetBackbuffer(backBuffer);
			RenderColourFilterLeedsPS2();
			changedBackbuf = 1;
		}else if(gColourFilter == PLATFORM_PSP){
			if(changedBackbuf)
				GetBackbuffer(backBuffer);
			RenderColourFilterLeedsPSP();
			changedBackbuf = 1;
		}


	d3ddevice->SetPixelShader(nil);
}

#else
void RenderPostFX(void) {}
#endif
