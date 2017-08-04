#include "III.h"
#include <cstdarg>

CEntity *debugent;

rw::Camera *rwCamera;
rw::World  *rwWorld;
rw::Light  *pAmbient;
rw::Light  *pDirect;
rw::Light  *pExtraDirectionals[4];
bool isRunning;

uchar work_buff[55000];

char*
getPath(const char *path)
{
	static char cipath[1024];
	strncpy(cipath, path, 1024);
	rw::makePath(cipath);
	return cipath;
}

FILE*
fopen_ci(const char *path, const char *mode)
{
	char cipath[1024];
	strncpy(cipath, path, 1024);
	rw::makePath(cipath);
	return fopen(cipath, mode);
}

char*
skipWhite(char *s)
{
	while(isspace(*s))
		s++;
	return s;
}

int
StrAssoc::get(StrAssoc *desc, const char *key)
{
	for(; desc->key[0] != '\0'; desc++)
		if(strcmp(desc->key, key) == 0)
			return desc->val;
	return desc->val;
}

void*
DatDesc::get(DatDesc *desc, const char *name)
{
	for(; desc->name[0] != '\0'; desc++)
		if(strcmp(desc->name, name) == 0)
			return (void*)desc->handler;
	return (void*)desc->handler;
}

rw::Raster*
d3dToGl3(rw::Raster *raster)
{
	if(raster->platform != rw::PLATFORM_D3D8 &&
	   raster->platform != rw::PLATFORM_D3D9)
		return raster;
	rw::d3d::D3dRaster *natras = PLUGINOFFSET(rw::d3d::D3dRaster,
	                                      raster, rw::d3d::nativeRasterOffset);
	if(natras->format)
		assert(0 && "no custom d3d formats");

	rw::Image *image = raster->toImage();
	raster->destroy();
	raster = rw::Raster::createFromImage(image, rw::PLATFORM_GL3);
	image->destroy();
	return raster;
}

void
convertTxd(rw::TexDictionary *txd)
{
	FORLIST(lnk, txd->textures){
		rw::Texture *tex = rw::Texture::fromDict(lnk);
		//debug("converting %s\n", tex->name);
		tex->raster = d3dToGl3(tex->raster);
	}
}


CVector
FindPlayerCoors(void)
{
	return TheCamera.m_position;
}

float AmbientLightColourForFrame[3];
float DirectionalLightColourForFrame[3];

void
SetLightsWithTimeOfDayColour(rw::World*)
{
	// TODO: CCoronas::LightsMult
	AmbientLightColourForFrame[0] = CTimeCycle::m_fCurrentAmbientRed;
	AmbientLightColourForFrame[1] = CTimeCycle::m_fCurrentAmbientGreen;
	AmbientLightColourForFrame[2] = CTimeCycle::m_fCurrentAmbientBlue;
	// TODO: flash and rain etc.
	pAmbient->setColor(AmbientLightColourForFrame[0],
	                   AmbientLightColourForFrame[1],
	                   AmbientLightColourForFrame[2]);

	// TODO: CCoronas::LightsMult
	DirectionalLightColourForFrame[0] = CTimeCycle::m_fCurrentDirectionalRed;
	DirectionalLightColourForFrame[1] = CTimeCycle::m_fCurrentDirectionalGreen;
	DirectionalLightColourForFrame[2] = CTimeCycle::m_fCurrentDirectionalBlue;
	pDirect->setColor(DirectionalLightColourForFrame[0],
	                  DirectionalLightColourForFrame[1],
	                  DirectionalLightColourForFrame[2]);
	// TODO: transform
}

void
LightsCreate(rw::World *world)
{
	pAmbient = rw::Light::create(rw::Light::AMBIENT);
	pAmbient->setColor(0.25f, 0.2f, 0.25f);
	pAmbient->setFlags(rw::Light::LIGHTATOMICS);

	pDirect = rw::Light::create(rw::Light::DIRECTIONAL);
	pDirect->setFlags(rw::Light::LIGHTATOMICS);
	pDirect->setColor(1.0f, 0.45f, 0.85f);
	pDirect->radius = 2.0f;
	rw::Frame *frm = rw::Frame::create();
	pDirect->setFrame(frm);
	rw::V3d axis = { 1.0f, 1.0f, 0.0f };
	frm->rotate(&axis, 160.0f, rw::COMBINEPRECONCAT);

	world->addLight(pAmbient);
	world->addLight(pDirect);

	for(int i = 0; i < nelem(pExtraDirectionals); i++){
		rw::Light *l = rw::Light::create(rw::Light::DIRECTIONAL);
		pExtraDirectionals[i] = l;
		l->setFlags(0);
		l->setColor(1.0f, 0.5f, 0.0f);
		l->radius = 2.0f;
		l->setFrame(rw::Frame::create());
		world->addLight(l);
	}
}

void
DeActivateDirectional(void)
{
	pDirect->setFlags(0);
}

void
SetAmbientColours(void)
{
	pAmbient->setColor(AmbientLightColourForFrame[0],
	                   AmbientLightColourForFrame[1],
	                   AmbientLightColourForFrame[2]);
}

void
DefinedState(void)
{
	SetRenderState(rw::ZTESTENABLE, 1);
	SetRenderState(rw::ZWRITEENABLE, 1);
	SetRenderState(rw::VERTEXALPHA, 0);
	SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	SetRenderState(rw::FOGENABLE, 0);
	SetRenderState(rw::ALPHATESTREF, 10);
	SetRenderState(rw::ALPHATESTFUNC, rw::ALPHALESS);
	rw::RGBA c;
	c.red = CTimeCycle::m_nCurrentFogColourRed;
	c.green = CTimeCycle::m_nCurrentFogColourGreen;
	c.blue = CTimeCycle::m_nCurrentFogColourBlue;
	c.alpha = 0xFF;
	SetRenderState(rw::FOGCOLOR, *(uint32*)&c);
}

void
debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void
TheGame(void)
{
	static rw::RGBA clearcol = { 0x40, 0x40, 0x40, 0xFF };

	debug("Into TheGame!!!\n");

	isRunning = 1;
	CGame::InitialiseRW();
	CGame::InitialiseAfterRW();
	CGame::Initialise();

	while(isRunning && !plWindowclosed()){
		plHandleEvents();
		CTimer::Update();
		CGame::Process();

		SetLightsWithTimeOfDayColour(rwWorld);
		clearcol.red = CTimeCycle::m_nCurrentSkyTopRed;
		clearcol.green = CTimeCycle::m_nCurrentSkyTopGreen;
		clearcol.blue = CTimeCycle::m_nCurrentSkyTopBlue;

		CRenderer::ConstructRenderList();

		TheCamera.m_rwcam->clear(&clearcol,
		                         rw::Camera::CLEARIMAGE|rw::Camera::CLEARZ);
		DefinedState();
		rwCamera->setFarPlane(CTimeCycle::m_fCurrentFarClip);
		rwCamera->fogPlane = CTimeCycle::m_fCurrentFogStart;
		TheCamera.update();
		TheCamera.m_rwcam->beginUpdate();

//		debug("visible entities, alpha list: %d %d\n",
//			CRenderer::ms_nNoOfVisibleEntities,
//			CVisibilityPlugins::m_alphaEntityList.Count());

		CRenderer::RenderRoads();
		SetRenderState(rw::FOGENABLE, 1);
		CRenderer::RenderEverythingBarRoads();
		DefinedState();
		CRenderer::RenderFadingInEntities();

		TheCamera.m_rwcam->endUpdate();
		plPresent();
	}
}
