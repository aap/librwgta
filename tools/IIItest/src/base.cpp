#include "III.h"
#include <cstdarg>

rw::Camera *rwCamera;
rw::World  *rwWorld;
rw::Light  *ambient;
rw::Light  *direct;
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
        using namespace rw;
	if(raster->platform != PLATFORM_D3D8 &&
	   raster->platform != PLATFORM_D3D9)
		return raster;
	d3d::D3dRaster *natras = PLUGINOFFSET(d3d::D3dRaster,
	                                      raster, d3d::nativeRasterOffset);
	if(natras->format)
		assert(0 && "no custom d3d formats");

	Image *image = raster->toImage();
	raster->destroy();
	raster = Raster::createFromImage(image, PLATFORM_GL3);
	image->destroy();
	return raster;
}

void
convertTxd(rw::TexDictionary *txd)
{
	using namespace rw;
	FORLIST(lnk, txd->textures){
		Texture *tex = Texture::fromDict(lnk);
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
	ambient->setColor(AmbientLightColourForFrame[0],
	                  AmbientLightColourForFrame[1],
	                  AmbientLightColourForFrame[2]);

	// TODO: CCoronas::LightsMult
	DirectionalLightColourForFrame[0] = CTimeCycle::m_fCurrentDirectionalRed;
	DirectionalLightColourForFrame[1] = CTimeCycle::m_fCurrentDirectionalGreen;
	DirectionalLightColourForFrame[2] = CTimeCycle::m_fCurrentDirectionalBlue;
	direct->setColor(DirectionalLightColourForFrame[0],
	                 DirectionalLightColourForFrame[1],
	                 DirectionalLightColourForFrame[2]);
	// TODO: transform
}

void
DefinedState(void)
{
	using namespace rw;
	engine->setRenderState(ZTESTENABLE, 1);
	engine->setRenderState(ZWRITEENABLE, 1);
	engine->setRenderState(VERTEXALPHA, 0);
	engine->setRenderState(SRCBLEND, BLENDSRCALPHA);
	engine->setRenderState(DESTBLEND, BLENDINVSRCALPHA);
	engine->setRenderState(FOGENABLE, 0);
	RGBA c;
	c.red = CTimeCycle::m_nCurrentFogColourRed;
	c.green = CTimeCycle::m_nCurrentFogColourGreen;
	c.blue = CTimeCycle::m_nCurrentFogColourBlue;
	c.alpha = 0xFF;
	engine->setRenderState(FOGCOLOR, *(uint32*)&c);
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
	using namespace rw;
	static RGBA clearcol = { 0x40, 0x40, 0x40, 0xFF };

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
		                         Camera::CLEARIMAGE|Camera::CLEARZ);
		DefinedState();
		rwCamera->setFarPlane(CTimeCycle::m_fCurrentFarClip);
		rwCamera->fogPlane = CTimeCycle::m_fCurrentFogStart;
		TheCamera.update();
		TheCamera.m_rwcam->beginUpdate();

		CRenderer::RenderEverything();
		CRenderer::RenderFadingInEntities();

		TheCamera.m_rwcam->endUpdate();
		plPresent();
	}
}

