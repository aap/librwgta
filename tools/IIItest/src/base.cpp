#include "III.h"
#include <cstdarg>

rw::Camera *rwCamera;
rw::World  *rwWorld;
rw::Light  *ambient;
rw::Light  *direct;

uchar work_buff[55000];

CVector
FindPlayerCoors(void)
{
	return TheCamera.m_position;
}

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

void
debug(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

void
update(double t)
{
}

void
display(void)
{
	using namespace rw;
	static RGBA clearcol = { 0x40, 0x40, 0x40, 0xFF };

	CTimer::Update();
	CGame::Process();

	CRenderer::ConstructRenderList();

	TheCamera.update();
	TheCamera.m_rwcam->clear(&clearcol, Camera::CLEARIMAGE|Camera::CLEARZ);
	TheCamera.m_rwcam->beginUpdate();

	CRenderer::RenderEverything();
	CRenderer::RenderFadingInEntities();

	TheCamera.m_rwcam->endUpdate();
}

void
shutdown(void)
{
}

int
init(void)
{
	CGame::InitialiseRW();
	CGame::InitialiseAfterRW();
	CGame::Initialise();

	return 1;
}
