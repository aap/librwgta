#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>

#include "rwgta.h"

using namespace std;

namespace rw {

int32
findPlatform(Atomic *a)
{
	Geometry *g = a->geometry;
	if(g->instData)
		return g->instData->platform;
	return 0;
}

int32
findPlatform(Clump *c)
{
	FORLIST(lnk, c->atomics){
		Geometry *g = Atomic::fromClump(lnk)->geometry;
		if(g->instData)
			return g->instData->platform;
	}
	return 0;
}

void
switchPipes(Atomic *a, int32 platform)
{
	if(a->pipeline && a->pipeline->platform != platform){
		uint32 plgid = a->pipeline->pluginID;
		switch(plgid){
		// assume default pipe won't be attached explicitly
		case ID_SKIN:
			a->pipeline = skinGlobals.pipelines[platform];
			break;
		case ID_MATFX:
			a->pipeline = matFXGlobals.pipelines[platform];
			break;
		}
	}
}

void
switchPipes(Clump *c, int32 platform)
{
	FORLIST(lnk, c->atomics){
		Atomic *a = Atomic::fromClump(lnk);
		switchPipes(a, platform);
	}
}

}

typedef rw::int32 int32;
typedef rw::int16 int16;
typedef rw::int8 int8;
typedef rw::uint32 uint32;
typedef rw::uint16 uint16;
typedef rw::uint8 uint8;
typedef rw::float32 float32;

/* broken NTL dragonball online plugin - we have no better place to do this than here */
static rw::Stream *ntlread(rw::Stream *stream, int32, void*, int32, int32) { return stream; }
static rw::Stream *ntlwrite(rw::Stream *stream, int32, void*, int32, int32) { return stream; }
static int32 ntlsize(void*, int32, int32) { return 0; }
static void registerNTLmaterialPlugin(void)
{
	rw::Material::registerPlugin(0x0, 0x177, nil, nil, nil);
	rw::Material::registerPluginStream(0x177, ntlread, ntlwrite, ntlsize);
}

namespace gta {

void
attachPlugins(void)
{
	rw::ps2::registerPDSPlugin(40);
	rw::ps2::registerPluginPDSPipes();
	gta::registerPDSPipes();

	rw::registerMeshPlugin();
	rw::registerNativeDataPlugin();
	rw::registerAtomicRightsPlugin();
	rw::registerMaterialRightsPlugin();
	rw::xbox::registerVertexFormatPlugin();
	rw::registerSkinPlugin();
	rw::registerUserDataPlugin();	// not GTA
	rw::registerHAnimPlugin();
	gta::registerNodeNamePlugin();
	rw::registerMatFXPlugin();
	rw::registerUVAnimPlugin();
	rw::ps2::registerADCPlugin();

	gta::registerExtraNormalsPlugin();
	gta::registerExtraVertColorPlugin();
	gta::registerEnvSpecPlugin();
	gta::registerBreakableModelPlugin();
	gta::registerCollisionPlugin();
	gta::register2dEffectPlugin();
	gta::registerPipelinePlugin();

	registerNTLmaterialPlugin();
}

//
// Frame
//

// Node Name

int32 nodeNameOffset;

static void*
createNodeName(void *object, int32 offset, int32)
{
	char *name = PLUGINOFFSET(char, object, offset);
	name[0] = '\0';
	return object;
}

static void*
copyNodeName(void *dst, void *src, int32 offset, int32)
{
	char *dstname = PLUGINOFFSET(char, dst, offset);
	char *srcname = PLUGINOFFSET(char, src, offset);
	strncpy(dstname, srcname, 23);
	return dst;
}

static void*
destroyNodeName(void *object, int32, int32)
{
	return object;
}

static rw::Stream*
readNodeName(rw::Stream *stream, int32 len, void *object, int32 offset, int32)
{
	char *name = PLUGINOFFSET(char, object, offset);
	stream->read(name, len);
	name[len] = '\0';
	//printf("%s\n", name);
	return stream;
}

static rw::Stream*
writeNodeName(rw::Stream *stream, int32 len, void *object, int32 offset, int32)
{
	char *name = PLUGINOFFSET(char, object, offset);
	stream->write(name, len);
	return stream;
}

static int32
getSizeNodeName(void *object, int32 offset, int32)
{
	char *name = PLUGINOFFSET(char, object, offset);
	int32 len = strlen(name);
	return len > 0 ? len : 0;
}


void
registerNodeNamePlugin(void)
{
	nodeNameOffset = rw::Frame::registerPlugin(24, ID_NODENAME,
	                                       createNodeName,
	                                       destroyNodeName,
	                                       copyNodeName);
	rw::Frame::registerPluginStream(ID_NODENAME,
	                            readNodeName,
	                            writeNodeName,
	                            getSizeNodeName);
}

char*
getNodeName(rw::Frame *f)
{
	return PLUGINOFFSET(char, f, nodeNameOffset);
}

//
// Geometry
//

// Breakable Model

int32 breakableOffset;

static void*
createBreakableModel(void *object, int32 offset, int32)
{
	*PLUGINOFFSET(uint8*, object, offset) = 0;
	return object;
}

static void*
destroyBreakableModel(void *object, int32 offset, int32)
{
	uint8 *p = *PLUGINOFFSET(uint8*, object, offset);
	delete[] p;
	return object;
}

static rw::Stream*
readBreakableModel(rw::Stream *stream, int32, void *object, int32 o, int32)
{
	uint32 header[13];
	uint32 hasBreakable = stream->readU32();
	if(hasBreakable == 0)
		return stream;
	stream->read(header, 13*4);
	uint32 size = header[1]*(12+8+4) + header[5]*(6+2) +
	              header[8]*(32+32+12);
	uint8 *p = new uint8[sizeof(Breakable)+size];
	Breakable *breakable = (Breakable*)p;
	*PLUGINOFFSET(Breakable*, object, o) = breakable;
	breakable->position     = header[0];
	breakable->numVertices  = header[1];
	breakable->numFaces     = header[5];
	breakable->numMaterials = header[8];
	p += sizeof(Breakable);
	stream->read(p, size);
	breakable->vertices = (float*)p;
	p += breakable->numVertices*12;
	breakable->texCoords = (float*)p;
	p += breakable->numVertices*8;
	breakable->colors = (uint8*)p;
	p += breakable->numVertices*4;
	breakable->faces = (uint16*)p;
	p += breakable->numFaces*6;
	breakable->matIDs = (uint16*)p;
	p += breakable->numFaces*2;
	breakable->texNames = (char(*)[32])p;
	p += breakable->numMaterials*32;
	breakable->maskNames = (char(*)[32])p;
	p += breakable->numMaterials*32;
	breakable->surfaceProps = (float32(*)[3])p;
	return stream;
}

static rw::Stream*
writeBreakableModel(rw::Stream *stream, int32, void *object, int32 o, int32)
{
	uint32 header[13];
	Breakable *breakable = *PLUGINOFFSET(Breakable*, object, o);
	uint8 *p = (uint8*)breakable;
	if(breakable == nil){
		stream->writeU32(0);
		return stream;
	}
	stream->writeU32(1);
	memset((char*)header, 0, 13*4);
	header[0] = breakable->position;
	header[1] = breakable->numVertices;
	header[5] = breakable->numFaces;
	header[8] = breakable->numMaterials;
	stream->write(header, 13*4);
	p += sizeof(Breakable);
	stream->write(p, breakable->numVertices*(12+8+4) +
	                       breakable->numFaces*(6+2) +
	                       breakable->numMaterials*(32+32+12));
	return stream;
}

static int32
getSizeBreakableModel(void *object, int32 offset, int32)
{
	Breakable *breakable = *PLUGINOFFSET(Breakable*, object, offset);
	if(breakable == nil)
		return 0; //4;
	return 56 + breakable->numVertices*(12+8+4) +
	            breakable->numFaces*(6+2) +
	            breakable->numMaterials*(32+32+12);
}

void
registerBreakableModelPlugin(void)
{
	breakableOffset = rw::Geometry::registerPlugin(sizeof(Breakable*),
	                                           ID_BREAKABLE,
	                                           createBreakableModel,
	                                           destroyBreakableModel, nil);
	rw::Geometry::registerPluginStream(ID_BREAKABLE,
	                               readBreakableModel,
	                               writeBreakableModel,
	                               getSizeBreakableModel);
}

// Extra normals

int32 extraNormalsOffset;

static void*
createExtraNormals(void *object, int32 offset, int32)
{
	*PLUGINOFFSET(float*, object, offset) = nil;
	return object;
}

static void*
destroyExtraNormals(void *object, int32 offset, int32)
{
	float *extranormals = *PLUGINOFFSET(float*, object, offset);
	delete[] extranormals;
	*PLUGINOFFSET(float*, object, offset) = nil;
	return object;
}

static rw::Stream*
readExtraNormals(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	rw::Geometry *geo = (rw::Geometry*)object;
	float **plgp = PLUGINOFFSET(float*, object, offset);
	if(*plgp)
		delete[] *plgp;
	float *extranormals = *plgp = new float[geo->numVertices*3];
	stream->read(extranormals, geo->numVertices*3*4);
//	printf("extra normals\n");

//	for(int i = 0; i < geo->numVertices; i++){
//		float *nx = extranormals+i*3;
//		float *n = geo->morphTargets[0].normals;
//		float len = n[0]*n[0] + n[1]*n[1] + n[2]*n[2];
//		printf("%f %f %f %f\n", n[0], n[1], n[2], len);
//		printf("%f %f %f\n", nx[0], nx[1], nx[2]);
//	}
	return stream;
}

static rw::Stream*
writeExtraNormals(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	rw::Geometry *geo = (rw::Geometry*)object;
	float *extranormals = *PLUGINOFFSET(float*, object, offset);
	assert(extranormals != nil);
	stream->write(extranormals, geo->numVertices*3*4);
	return stream;
}

static int32
getSizeExtraNormals(void *object, int32 offset, int32)
{
	rw::Geometry *geo = (rw::Geometry*)object;
	if(*PLUGINOFFSET(float*, object, offset))
		return geo->numVertices*3*4;
	return 0;
}

void
registerExtraNormalsPlugin(void)
{
	extraNormalsOffset = rw::Geometry::registerPlugin(sizeof(void*),
	                                              ID_EXTRANORMALS,
	                                              createExtraNormals,
	                                              destroyExtraNormals,
	                                              nil);
	rw::Geometry::registerPluginStream(ID_EXTRANORMALS,
	                               readExtraNormals,
	                               writeExtraNormals,
	                               getSizeExtraNormals);
}


// Extra colors

int32 extraVertColorOffset;

void
allocateExtraVertColors(rw::Geometry *g)
{
	ExtraVertColors *colordata =
		PLUGINOFFSET(ExtraVertColors, g, extraVertColorOffset);
	colordata->nightColors = new rw::RGBA[g->numVertices];
	colordata->dayColors = new rw::RGBA[g->numVertices];
	colordata->balance = 1.0f;
}

static void*
createExtraVertColors(void *object, int32 offset, int32)
{
	ExtraVertColors *colordata =
		PLUGINOFFSET(ExtraVertColors, object, offset);
	colordata->nightColors = nil;
	colordata->dayColors = nil;
	colordata->balance = 0.0f;
	return object;
}

static void*
destroyExtraVertColors(void *object, int32 offset, int32)
{
	ExtraVertColors *colordata =
		PLUGINOFFSET(ExtraVertColors, object, offset);
	delete[] colordata->nightColors;
	delete[] colordata->dayColors;
	return object;
}

static rw::Stream*
readExtraVertColors(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	uint32 hasData;
	ExtraVertColors *colordata =
		PLUGINOFFSET(ExtraVertColors, object, offset);
	hasData = stream->readU32();
	if(!hasData)
		return stream;
	rw::Geometry *geometry = (rw::Geometry*)object;
	colordata->nightColors = new rw::RGBA[geometry->numVertices];
	colordata->dayColors = new rw::RGBA[geometry->numVertices];
	colordata->balance = 1.0f;
	stream->read(colordata->nightColors, geometry->numVertices*4);
	if(geometry->colors)
		memcpy(colordata->dayColors, geometry->colors,
		       geometry->numVertices*4);
	return stream;
}

static rw::Stream*
writeExtraVertColors(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	ExtraVertColors *colordata =
		PLUGINOFFSET(ExtraVertColors, object, offset);
	stream->writeU32(colordata->nightColors != nil);
	if(colordata->nightColors){
		rw::Geometry *geometry = (rw::Geometry*)object;
		stream->write(colordata->nightColors, geometry->numVertices*4);
	}
	return stream;
}

static int32
getSizeExtraVertColors(void *object, int32 offset, int32)
{
	ExtraVertColors *colordata =
		PLUGINOFFSET(ExtraVertColors, object, offset);
	rw::Geometry *geometry = (rw::Geometry*)object;
	if(colordata->nightColors)
		return 4 + geometry->numVertices*4;
	return 0;
}

void
registerExtraVertColorPlugin(void)
{
	extraVertColorOffset = rw::Geometry::registerPlugin(sizeof(ExtraVertColors),
	                                                ID_EXTRAVERTCOLORS,
	                                                createExtraVertColors,
	                                                destroyExtraVertColors,
	                                                nil);
	rw::Geometry::registerPluginStream(ID_EXTRAVERTCOLORS,
	                               readExtraVertColors,
	                               writeExtraVertColors,
	                               getSizeExtraVertColors);
}

rw::RGBA*
getExtraVertColors(rw::Atomic *a)
{
	return PLUGINOFFSET(gta::ExtraVertColors, a->geometry, gta::extraVertColorOffset)->nightColors;
}

// Environment mat

int32 envMatOffset;

static void*
createEnvMat(void *object, int32 offset, int32)
{
	*PLUGINOFFSET(EnvMat*, object, offset) = nil;
	return object;
}

static void*
destroyEnvMat(void *object, int32 offset, int32)
{
	EnvMat **envmat = PLUGINOFFSET(EnvMat*, object, offset);
	delete *envmat;
	*envmat = nil;
	return object;
}

static void*
copyEnvMat(void *dst, void *src, int32 offset, int32)
{
	EnvMat *srcenv = *PLUGINOFFSET(EnvMat*, src, offset);
	if(srcenv == nil)
		return dst;
	EnvMat *dstenv = new EnvMat;
	dstenv->scaleX = srcenv->scaleX;
	dstenv->scaleY = srcenv->scaleY;
	dstenv->transScaleX = srcenv->transScaleX;
	dstenv->transScaleY = srcenv->transScaleY;
	dstenv->shininess = srcenv->shininess;
	dstenv->texture = nil;
	*PLUGINOFFSET(EnvMat*, dst, offset) = dstenv;
	return dst;
}

struct EnvStream {
	float scaleX, scaleY;
	float transScaleX, transScaleY;
	float shininess;
	int32 zero;
};

static rw::Stream*
readEnvMat(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	EnvStream buf;
	EnvMat *env = new EnvMat;
	*PLUGINOFFSET(EnvMat*, object, offset) = env;
	stream->read(&buf, sizeof(buf));
	env->scaleX = (int8)(buf.scaleX*8.0f);
	env->scaleY = (int8)(buf.scaleY*8.0f);
	env->transScaleX = (int8)(buf.transScaleX*8.0f);
	env->transScaleY = (int8)(buf.transScaleY*8.0f);
	env->shininess = (uint8)(buf.shininess*255.0f);
	env->texture = nil;
	return stream;
}

static rw::Stream*
writeEnvMat(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	EnvStream buf;
	EnvMat *env = *PLUGINOFFSET(EnvMat*, object, offset);
	buf.scaleX = env->scaleX/8.0f;
	buf.scaleY = env->scaleY/8.0f;
	buf.transScaleX = env->transScaleX/8.0f;
	buf.transScaleY = env->transScaleY/8.0f;
	buf.shininess = env->shininess/255.0f;
	buf.zero = 0;
	stream->write(&buf, sizeof(buf));
	return stream;
}

static int32
getSizeEnvMat(void *object, int32 offset, int32)
{
	EnvMat *env = *PLUGINOFFSET(EnvMat*, object, offset);
	return env ? (int)sizeof(EnvStream) : 0;
}

// Specular mat

int32 specMatOffset;

static void*
createSpecMat(void *object, int32 offset, int32)
{
	*PLUGINOFFSET(SpecMat*, object, offset) = nil;
	return object;
}

static void*
destroySpecMat(void *object, int32 offset, int32)
{
	SpecMat **specmat = PLUGINOFFSET(SpecMat*, object, offset);
	if(*specmat == nil)
		return object;
	if((*specmat)->texture)
		(*specmat)->texture->destroy();
	delete *specmat;
	*specmat = nil;
	return object;
}

static void*
copySpecMat(void *dst, void *src, int32 offset, int32)
{
	SpecMat *srcspec = *PLUGINOFFSET(SpecMat*, src, offset);
	if(srcspec == nil)
		return dst;
	SpecMat *dstspec = new SpecMat;
	*PLUGINOFFSET(SpecMat*, dst, offset) = dstspec;
	dstspec->specularity = srcspec->specularity;
	dstspec->texture = srcspec->texture;
	dstspec->texture->refCount++;
	return dst;
}

struct SpecStream {
	float specularity;
	char texname[24];
};

static rw::Stream*
readSpecMat(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	SpecStream buf;
	SpecMat *spec = new SpecMat;
	*PLUGINOFFSET(SpecMat*, object, offset) = spec;
	stream->read(&buf, sizeof(buf));
	spec->specularity = buf.specularity;
	spec->texture = rw::Texture::create(nil);
	strncpy(spec->texture->name, buf.texname, 24);
	return stream;
}

static rw::Stream*
writeSpecMat(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	SpecStream buf;
	SpecMat *spec = *PLUGINOFFSET(SpecMat*, object, offset);
	buf.specularity = spec->specularity;
	strncpy(buf.texname, spec->texture->name, 24);
	stream->write(&buf, sizeof(buf));
	return stream;
}

static int32
getSizeSpecMat(void *object, int32 offset, int32)
{
	SpecMat *spec = *PLUGINOFFSET(SpecMat*, object, offset);
	return spec ? (int)sizeof(SpecStream) : 0;
}

void
registerEnvSpecPlugin(void)
{
	envMatOffset = rw::Material::registerPlugin(sizeof(EnvMat*), ID_ENVMAT,
	                                        createEnvMat,
                                                destroyEnvMat,
                                                copyEnvMat);
	rw::Material::registerPluginStream(ID_ENVMAT, readEnvMat,
                                                  writeEnvMat,
                                                  getSizeEnvMat);
	specMatOffset = rw::Material::registerPlugin(sizeof(SpecMat*), ID_SPECMAT,
	                                         createSpecMat,
                                                 destroySpecMat,
                                                 copySpecMat);
	rw::Material::registerPluginStream(ID_SPECMAT, readSpecMat,
                                                   writeSpecMat,
                                                   getSizeSpecMat);
}

// Pipeline

int32 pipelineOffset;

static void*
createPipeline(void *object, int32 offset, int32)
{
	*PLUGINOFFSET(uint32, object, offset) = 0;
	return object;
}

static void*
copyPipeline(void *dst, void *src, int32 offset, int32)
{
	*PLUGINOFFSET(uint32, dst, offset) = *PLUGINOFFSET(uint32, src, offset);
	return dst;
}

static rw::Stream*
readPipeline(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	*PLUGINOFFSET(uint32, object, offset) = stream->readU32();
//	printf("%x\n", *PLUGINOFFSET(uint32, object, offset));
	return stream;
}

static rw::Stream*
writePipeline(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	stream->writeU32(*PLUGINOFFSET(uint32, object, offset));
	return stream;
}

static int32
getSizePipeline(void *object, int32 offset, int32)
{
	if(*PLUGINOFFSET(uint32, object, offset))
		return 4;
	return 0;
}

void
registerPipelinePlugin(void)
{
	pipelineOffset = rw::Atomic::registerPlugin(sizeof(uint32), ID_PIPELINE,
	                                        createPipeline,
                                                nil,
                                                copyPipeline);
	rw::Atomic::registerPluginStream(ID_PIPELINE, readPipeline,
	                             writePipeline, getSizePipeline);
}

uint32
getPipelineID(rw::Atomic *atomic)
{
	return *PLUGINOFFSET(uint32, atomic, pipelineOffset);
}

void
setPipelineID(rw::Atomic *atomic, uint32 id)
{
	*PLUGINOFFSET(uint32, atomic, pipelineOffset) = id;
}

// 2dEffect

struct SizedData
{
	uint32 size;
	uint8 *data;
};

int32 twodEffectOffset;

static void*
create2dEffect(void *object, int32 offset, int32)
{
	SizedData *data;
	data = PLUGINOFFSET(SizedData, object, offset);
	data->size = 0;
	data->data = nil;
	return object;
}

static void*
destroy2dEffect(void *object, int32 offset, int32)
{
	SizedData *data;
	data = PLUGINOFFSET(SizedData, object, offset);
	delete[] data->data;
	data->data = nil;
	data->size = 0;
	return object;
}

static void*
copy2dEffect(void *dst, void *src, int32 offset, int32)
{
	SizedData *srcdata, *dstdata;
	dstdata = PLUGINOFFSET(SizedData, dst, offset);
	srcdata = PLUGINOFFSET(SizedData, src, offset);
	dstdata->size = srcdata->size;
	if(dstdata->size != 0){
		dstdata->data = new uint8[dstdata->size];
		memcpy(dstdata->data, srcdata->data, dstdata->size);
	}
	return dst;
}

static rw::Stream*
read2dEffect(rw::Stream *stream, int32 size, void *object, int32 offset, int32)
{
	SizedData *data = PLUGINOFFSET(SizedData, object, offset);
	data->size = size;
	data->data = new uint8[data->size];
	stream->read(data->data, data->size);
	return stream;
}

static rw::Stream*
write2dEffect(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	SizedData *data = PLUGINOFFSET(SizedData, object, offset);
	stream->write(data->data, data->size);
	return stream;
}

static int32
getSize2dEffect(void *object, int32 offset, int32)
{
	SizedData *data = PLUGINOFFSET(SizedData, object, offset);
	return data->size;
}

void
register2dEffectPlugin(void)
{
	twodEffectOffset = rw::Geometry::registerPlugin(sizeof(SizedData), ID_2DEFFECT,
	                                            create2dEffect,
                                                    destroy2dEffect,
                                                    copy2dEffect);
	rw::Geometry::registerPluginStream(ID_2DEFFECT, read2dEffect,
	                               write2dEffect, getSize2dEffect);
}

// Collision

int32 collisionOffset;

static void*
createCollision(void *object, int32 offset, int32)
{
	SizedData *data;
	data = PLUGINOFFSET(SizedData, object, offset);
	data->size = 0;
	data->data = nil;
	return object;
}

static void*
destroyCollision(void *object, int32 offset, int32)
{
	SizedData *data;
	data = PLUGINOFFSET(SizedData, object, offset);
	delete[] data->data;
	data->data = nil;
	data->size = 0;
	return object;
}

static void*
copyCollision(void *dst, void *src, int32 offset, int32)
{
	SizedData *srcdata, *dstdata;
	dstdata = PLUGINOFFSET(SizedData, dst, offset);
	srcdata = PLUGINOFFSET(SizedData, src, offset);
	dstdata->size = srcdata->size;
	if(dstdata->size != 0){
		dstdata->data = new uint8[dstdata->size];
		memcpy(dstdata->data, srcdata->data, dstdata->size);
	}
	return dst;
}

static rw::Stream*
readCollision(rw::Stream *stream, int32 size, void *object, int32 offset, int32)
{
	SizedData *data = PLUGINOFFSET(SizedData, object, offset);
	data->size = size;
	data->data = new uint8[data->size];
	stream->read(data->data, data->size);
	return stream;
}

static rw::Stream*
writeCollision(rw::Stream *stream, int32, void *object, int32 offset, int32)
{
	SizedData *data = PLUGINOFFSET(SizedData, object, offset);
	stream->write(data->data, data->size);
	return stream;
}

static int32
getSizeCollision(void *object, int32 offset, int32)
{
	SizedData *data = PLUGINOFFSET(SizedData, object, offset);
	return data->size;
}

void
registerCollisionPlugin(void)
{
	collisionOffset = rw::Clump::registerPlugin(sizeof(SizedData), ID_COLLISION,
	                                        createCollision,
                                                destroyCollision,
                                                copyCollision);
	rw::Clump::registerPluginStream(ID_COLLISION, readCollision,
	                            writeCollision, getSizeCollision);
}

}
