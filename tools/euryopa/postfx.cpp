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

	verts[0].setScreenX(0-0.5f);
	verts[0].setScreenY(0-0.5f);
	verts[0].setScreenZ(rw::im2d::GetNearZ());
	verts[0].setCameraZ(cam->nearPlane);
	verts[0].setRecipCameraZ(1.0f/cam->nearPlane);
	verts[0].setColor(255, 255, 255, 255);
	verts[0].setU(0.0f);
	verts[0].setV(0.0f);

	verts[1].setScreenX(w-0.5f);
	verts[1].setScreenY(0-0.5f);
	verts[1].setScreenZ(rw::im2d::GetNearZ());
	verts[1].setCameraZ(cam->nearPlane);
	verts[1].setRecipCameraZ(1.0f/cam->nearPlane);
	verts[1].setColor(255, 255, 255, 255);
	verts[1].setU(1.0f);
	verts[1].setV(0.0f);

	verts[2].setScreenX(0-0.5f);
	verts[2].setScreenY(h-0.5f);
	verts[2].setScreenZ(rw::im2d::GetNearZ());
	verts[2].setCameraZ(cam->nearPlane);
	verts[2].setRecipCameraZ(1.0f/cam->nearPlane);
	verts[2].setColor(255, 255, 255, 255);
	verts[2].setU(0.0f);
	verts[2].setV(1.0f);

	verts[3].setScreenX(w-0.5f);
	verts[3].setScreenY(h-0.5f);
	verts[3].setScreenZ(rw::im2d::GetNearZ());
	verts[3].setCameraZ(cam->nearPlane);
	verts[3].setRecipCameraZ(1.0f/cam->nearPlane);
	verts[3].setColor(255, 255, 255, 255);
	verts[3].setU(1.0f);
	verts[3].setV(1.0f);
}

static void
InitPostFX(void)
{
	if(backBuffer)
		backBuffer->destroy();
	backBuffer = Raster::create(sk::globals.width, sk::globals.height, 32, Raster::C888|Raster::CAMERATEXTURE);
	if(backBufferTex == nil){
		backBufferTex = Texture::create(backBuffer);
		backBufferTex->setFilter(Texture::FilterMode::LINEAR);
		// we may want to blur later on
		backBufferTex->setAddressU(Texture::Addressing::CLAMP);
		backBufferTex->setAddressV(Texture::Addressing::CLAMP);
	}else
		backBufferTex->raster = backBuffer;
}

#ifdef RW_D3D9

using namespace d3d;
using namespace d3d9;

static void
GetBackbuffer(Raster *raster)
{
	assert(raster->type == Raster::CAMERATEXTURE);
	D3dRaster *natras = PLUGINOFFSET(D3dRaster, raster, nativeRasterOffset);

	IDirect3DTexture9 *tex = (IDirect3DTexture9*)natras->texture;
	IDirect3DSurface9 *surf = nil;
	IDirect3DSurface9 *backbuf = nil;
 	tex->GetSurfaceLevel(0, &surf);
	assert(surf);
	d3ddevice->GetRenderTarget(0, &backbuf);
	assert(backbuf);
	d3ddevice->StretchRect(backbuf, nil, surf, nil, D3DTEXF_NONE);
	backbuf->Release();
	surf->Release();
}

static void *colourfilterIII_PS;
static void *colourfilterVC_PS;
static void *colourfilterSAPS2_PS;
static void *colourfilterSAPC_PS;
static void *radiosityPS;
static bool shadersInitialized;

static void
CreateShaders(void)
{
#include "d3d_shaders/colourfilterIII_PS.inc"
	colourfilterIII_PS = createPixelShader(colourfilterIII_PS_cso);
#include "d3d_shaders/colourfilterVC_PS.inc"
	colourfilterVC_PS = createPixelShader(colourfilterVC_PS_cso);
#include "d3d_shaders/colourfilterSAPS2_PS.inc"
	colourfilterSAPS2_PS = createPixelShader(colourfilterSAPS2_PS_cso);
#include "d3d_shaders/colourfilterSAPC_PS.inc"
	colourfilterSAPC_PS = createPixelShader(colourfilterSAPC_PS_cso);
#include "d3d_shaders/radiosityPS.inc"
	radiosityPS = createPixelShader(radiosityPS_cso);

	shadersInitialized = true;
}

static void
RenderColourFilterIII(void)
{
	setPixelShader(colourfilterIII_PS);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&verts, 4, &indices, 6);
}

static void
RenderColourFilterVC(void)
{
	setPixelShader(colourfilterVC_PS);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&verts, 4, &indices, 6);
}

static void
RenderColourFilterSAPS2(void)
{
	setPixelShader(colourfilterSAPS2_PS);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&verts, 4, &indices, 6);
}

static void
RenderColourFilterSAPC(void)
{
	setPixelShader(colourfilterSAPC_PS);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&verts, 4, &indices, 6);
}

static void
RenderRadiosity(void)
{
	setPixelShader(radiosityPS);
	rw::im2d::RenderIndexedPrimitive(rw::PRIMTYPETRILIST,
		&verts, 4, &indices, 6);
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
	postfxvars[0] = Timecycle::currentColours.intensityLimit/255.0f;
	postfxvars[1] = 35/255.0f;	// intensity
	postfxvars[2] = 1.0f;		// render passes
	d3ddevice->SetPixelShaderConstantF(2, (float*)postfxvars, 1);

	rw::engine->imtexture = backBufferTex;
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
		RenderRadiosity();
	}

	d3ddevice->SetPixelShader(nil);
}

#endif
