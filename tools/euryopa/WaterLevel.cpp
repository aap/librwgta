#include "euryopa.h"

namespace WaterLevel
{

#define WATERSTARTX params.waterStart.x
#define WATERENDX params.waterEnd.x
#define WATERSTARTY params.waterStart.y
#define WATERENDY params.waterEnd.y
#define WATERSMALLSECTORSZX ((WATERENDX - WATERSTARTX)/128)
#define WATERSMALLSECTORSZY ((WATERENDY - WATERSTARTY)/128)
#define WATERSECTORSZX ((WATERENDX - WATERSTARTX)/64)
#define WATERSECTORSZY ((WATERENDY - WATERSTARTY)/64)

static int ms_nNoOfWaterLevels;
static float ms_aWaterZs[48];
static CRect ms_aWaterRects[48];	// Seems to be unused
static uint8 aWaterBlockList[64][64];
static uint8 aWaterFineBlockList[128][128];

rw::Texture *gpWaterTex;
rw::Raster *gpWaterRaster;

// SA
struct WaterVertex
{
	rw::V3d pos;
	rw::V2d speed;
	float waveunk, waveheight;
};
struct WaterQuad
{
	int indices[4];
	int type;
};
struct WaterTri
{
	int indices[3];
	int type;
};
static WaterVertex waterVertices[NUMWATERVERTICES];
static int numWaterVertices;
static WaterQuad waterQuads[NUMWATERQUADS];
static int numWaterQuads;
static WaterTri waterTris[NUMWATERTRIS];
static int numWaterTris;

void
InitialiseWaterpro(void)
{
	FILE *file;

	ms_nNoOfWaterLevels = 0;
	if(file = fopen_ci("DATA\\waterpro.dat", "rb"), file == nil)
		return;
	fread(&ms_nNoOfWaterLevels, 1, sizeof(ms_nNoOfWaterLevels), file);
	fread(&ms_aWaterZs, 1, sizeof(ms_aWaterZs), file);
	fread(&ms_aWaterRects, 1, sizeof(ms_aWaterRects), file);
	fread(&aWaterBlockList, 1, sizeof(aWaterBlockList), file);
	fread(&aWaterFineBlockList, 1, sizeof(aWaterFineBlockList), file);
	fclose(file);
}

void
InitialiseWater(void)
{
	WaterVertex v[4];
	int type;
	int nfields;

	FILE *file;
	char *line;
	if(file = fopen_ci("data/water.dat", "rb"), file == nil)
		return;
	int h = 0, w = 0;
	while(line = FileLoader::LoadLine(file)){
		if(line[0] == ';' || line[0] == '*' || line[0] == 'p')
			continue;
		type = 1;
		nfields = sscanf(line,
			"%f %f %f %f %f %f %f "
			"%f %f %f %f %f %f %f "
			"%f %f %f %f %f %f %f "
			"%f %f %f %f %f %f %f "
			"%d",
			&v[0].pos.x, &v[0].pos.y, &v[0].pos.z, &v[0].speed.x, &v[0].speed.y, &v[0].waveunk, &v[0].waveheight,
			&v[1].pos.x, &v[1].pos.y, &v[1].pos.z, &v[1].speed.x, &v[1].speed.y, &v[1].waveunk, &v[1].waveheight,
			&v[2].pos.x, &v[2].pos.y, &v[2].pos.z, &v[2].speed.x, &v[2].speed.y, &v[2].waveunk, &v[2].waveheight,
			&v[3].pos.x, &v[3].pos.y, &v[3].pos.z, &v[3].speed.x, &v[3].speed.y, &v[3].waveunk, &v[3].waveheight,
			&type);
		if(nfields == 28 || nfields == 29){
			// quad
			if(numWaterVertices+4 > NUMWATERVERTICES ||
			   numWaterQuads+1 > NUMWATERQUADS){
				log("warning: too much water (%d vertices, %d quads, %d tris)\n",
					numWaterVertices, numWaterQuads, numWaterTris);
				continue;
			}
			WaterQuad *q = &waterQuads[numWaterQuads++];
			q->indices[0] = numWaterVertices++;
			q->indices[1] = numWaterVertices++;
			q->indices[2] = numWaterVertices++;
			q->indices[3] = numWaterVertices++;
			waterVertices[q->indices[0]] = v[0];
			waterVertices[q->indices[1]] = v[1];
			waterVertices[q->indices[2]] = v[2];
			waterVertices[q->indices[3]] = v[3];
			q->type = type;
		}else{
			if(numWaterVertices+3 > NUMWATERVERTICES ||
			   numWaterTris+1 > NUMWATERTRIS){
				log("warning: too much water (%d vertices, %d quads, %d tris)\n",
					numWaterVertices, numWaterQuads, numWaterTris);
				continue;
			}
			// triangle
			WaterTri *t = &waterTris[numWaterTris++];
			t->indices[0] = numWaterVertices++;
			t->indices[1] = numWaterVertices++;
			t->indices[2] = numWaterVertices++;
			waterVertices[t->indices[0]] = v[0];
			waterVertices[t->indices[1]] = v[1];
			waterVertices[t->indices[2]] = v[2];
			t->type = type;
		}
	}
	fclose(file);
}

void
Initialise(void)
{
	if(params.water == GAME_SA)
		InitialiseWater();
	else
		InitialiseWaterpro();

	TxdPush();
	int ptxd = FindTxdSlot("particle");
	TxdMakeCurrent(ptxd);
	if(gpWaterTex == nil)
		gpWaterTex = rw::Texture::read(params.waterTex, nil);
	gpWaterRaster = gpWaterTex->raster;
	TxdPop();
}

#define TEMPBUFFERVERTSIZE 256
#define TEMPBUFFERINDEXSIZE 1024
static int TempBufferIndicesStored;
static int TempBufferVerticesStored;
static rw::RWDEVICE::Im3DVertex TempVertexBuffer[TEMPBUFFERVERTSIZE];
static uint16 TempIndexBuffer[TEMPBUFFERINDEXSIZE];

float TEXTURE_ADDU, TEXTURE_ADDV;

static void
RenderAndEmptyRenderBuffer(void)
{
	assert(TempBufferVerticesStored <= TEMPBUFFERVERTSIZE);
	assert(TempBufferIndicesStored <= TEMPBUFFERINDEXSIZE);
	if(TempBufferVerticesStored){
		rw::im3d::Transform(TempVertexBuffer, TempBufferVerticesStored, nil);
		rw::im3d::RenderIndexed(rw::PRIMTYPETRILIST, TempIndexBuffer, TempBufferIndicesStored);
		rw::im3d::End();
	}
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}

void
RenderOneFlatSmallWaterPoly(float x, float y, float z, rw::RGBA const &color)
{
	if(TempBufferVerticesStored+4 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+6 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	int i = TempBufferVerticesStored;
	TempVertexBuffer[i + 0].setX(x);
	TempVertexBuffer[i + 0].setY(y);
	TempVertexBuffer[i + 0].setZ(z);
	TempVertexBuffer[i + 0].setU(TEXTURE_ADDU);
	TempVertexBuffer[i + 0].setV(TEXTURE_ADDV);
	TempVertexBuffer[i + 0].setColor(color.red, color.green, color.blue, color.alpha);

	TempVertexBuffer[i + 1].setX(x);
	TempVertexBuffer[i + 1].setY(y + WATERSMALLSECTORSZY);
	TempVertexBuffer[i + 1].setZ(z);
	TempVertexBuffer[i + 1].setU(TEXTURE_ADDU);
	TempVertexBuffer[i + 1].setV(TEXTURE_ADDV + 1.0f);
	TempVertexBuffer[i + 1].setColor(color.red, color.green, color.blue, color.alpha);

	TempVertexBuffer[i + 2].setX(x + WATERSMALLSECTORSZX);
	TempVertexBuffer[i + 2].setY(y + WATERSMALLSECTORSZY);
	TempVertexBuffer[i + 2].setZ(z);
	TempVertexBuffer[i + 2].setU(TEXTURE_ADDU + 1.0f);
	TempVertexBuffer[i + 2].setV(TEXTURE_ADDV + 1.0f);
	TempVertexBuffer[i + 2].setColor(color.red, color.green, color.blue, color.alpha);

	TempVertexBuffer[i + 3].setX(x + WATERSMALLSECTORSZX);
	TempVertexBuffer[i + 3].setY(y);
	TempVertexBuffer[i + 3].setZ(z);
	TempVertexBuffer[i + 3].setU(TEXTURE_ADDU + 1.0f);
	TempVertexBuffer[i + 3].setV(TEXTURE_ADDV);
	TempVertexBuffer[i + 3].setColor(color.red, color.green, color.blue, color.alpha);

	TempBufferVerticesStored += 4;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+3;
}

void
RenderOneFlatLargeWaterPoly(float x, float y, float z, rw::RGBA const &color)
{
	if(TempBufferVerticesStored+4 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+6 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	int i = TempBufferVerticesStored;
	TempVertexBuffer[i + 0].setX(x);
	TempVertexBuffer[i + 0].setY(y);
	TempVertexBuffer[i + 0].setZ(z);
	TempVertexBuffer[i + 0].setU(TEXTURE_ADDU);
	TempVertexBuffer[i + 0].setV(TEXTURE_ADDV);
	TempVertexBuffer[i + 0].setColor(color.red, color.green, color.blue, color.alpha);

	TempVertexBuffer[i + 1].setX(x);
	TempVertexBuffer[i + 1].setY(y + WATERSECTORSZY);
	TempVertexBuffer[i + 1].setZ(z);
	TempVertexBuffer[i + 1].setU(TEXTURE_ADDU);
	TempVertexBuffer[i + 1].setV(TEXTURE_ADDV + 2.0f);
	TempVertexBuffer[i + 1].setColor(color.red, color.green, color.blue, color.alpha);

	TempVertexBuffer[i + 2].setX(x + WATERSECTORSZX);
	TempVertexBuffer[i + 2].setY(y + WATERSECTORSZY);
	TempVertexBuffer[i + 2].setZ(z);
	TempVertexBuffer[i + 2].setU(TEXTURE_ADDU + 2.0f);
	TempVertexBuffer[i + 2].setV(TEXTURE_ADDV + 2.0f);
	TempVertexBuffer[i + 2].setColor(color.red, color.green, color.blue, color.alpha);

	TempVertexBuffer[i + 3].setX(x + WATERSECTORSZX);
	TempVertexBuffer[i + 3].setY(y);
	TempVertexBuffer[i + 3].setZ(z);
	TempVertexBuffer[i + 3].setU(TEXTURE_ADDU + 2.0f);
	TempVertexBuffer[i + 3].setV(TEXTURE_ADDV);
	TempVertexBuffer[i + 3].setColor(color.red, color.green, color.blue, color.alpha);

	TempBufferVerticesStored += 4;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+3;
}

void
RenderWaterpro(void)
{
	int i, j;

	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::FOGENABLE, gEnableFog);
	rw::engine->imtexture = gpWaterTex;

	rw::RGBA color = { 255, 255, 255, 255 };
	Timecycle::ColourSet *cs = &Timecycle::currentColours;
	if(params.water == GAME_III){
		color.red = (cs->amb.red + cs->dir.red)*255.0f;
		color.green = (cs->amb.green + cs->dir.green)*255.0f;
		color.blue = (cs->amb.blue + cs->dir.blue)*255.0f;
	}else
		rw::convColor(&color, &cs->water);
	TEXTURE_ADDU = 0.0f;
	TEXTURE_ADDV = 0.0f;
	float x, y, z;

	rw::Sphere sph;
	// Small polys
	sph.radius = sqrt(WATERSMALLSECTORSZX*WATERSMALLSECTORSZX*2.0f)/2.0f;
	for(i = 0; i < 128; i++)
		for(j = 0; j < 128; j++){
			if(aWaterFineBlockList[i][j] & 0x80)
				continue;
			x = WATERSMALLSECTORSZX*i + WATERSTARTX;
			y = WATERSMALLSECTORSZY*j + WATERSTARTY;
			z = ms_aWaterZs[aWaterFineBlockList[i][j]];
			sph.center.x = x + WATERSMALLSECTORSZX/2;
			sph.center.y = y + WATERSMALLSECTORSZY/2;
			sph.center.z = z;
			if(Scene.camera->frustumTestSphere(&sph) != rw::Camera::SPHEREOUTSIDE)
				RenderOneFlatSmallWaterPoly(x, y, z, color);
		}

/*
	// Large polys
	sph.radius = sqrt(WATERSECTORSZX*WATERSECTORSZX*2.0f)/2.0f;
	for(i = 0; i < 64; i++)
		for(j = 0; j < 64; j++){
			if(aWaterBlockList[i][j] & 0x80)
				continue;
			x = WATERSECTORSZX*i + WATERSTARTX;
			y = WATERSECTORSZY*j + WATERSTARTY;
			z = ms_aWaterZs[aWaterBlockList[i][j]];
			sph.center.x = x + WATERSECTORSZX/2;
			sph.center.y = y + WATERSECTORSZY/2;
			sph.center.z = z;
			if(Scene.camera->frustumTestSphere(&sph) != rw::Camera::SPHEREOUTSIDE)
				RenderOneFlatLargeWaterPoly(x, y, z, color);
		}
*/
	RenderAndEmptyRenderBuffer();
}

static const float uvscale = 1/32.0f;

static void
RenderWaterQuad(WaterQuad *q)
{
	if(TempBufferVerticesStored+4 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+6 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	rw::RGBA color;

	rw::convColor(&color, &Timecycle::currentColours.water);

	WaterVertex *v;
	int i = TempBufferVerticesStored;
	int j;
	for(j = 0; j < 4; j++){
		v = &waterVertices[q->indices[j]];
		TempVertexBuffer[i + j].setX(v->pos.x);
		TempVertexBuffer[i + j].setY(v->pos.y);
		TempVertexBuffer[i + j].setZ(v->pos.z);
		TempVertexBuffer[i + j].setU(v->pos.x*uvscale);
		TempVertexBuffer[i + j].setV(v->pos.y*uvscale);
		TempVertexBuffer[i + j].setColor(color.red, color.green, color.blue, color.alpha);
	}

	TempBufferVerticesStored += 4;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+3;
}

static void
RenderWaterTri(WaterTri *t)
{
	if(TempBufferVerticesStored+3 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+3 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	rw::RGBA color;

	rw::convColor(&color, &Timecycle::currentColours.water);

	WaterVertex *v;
	int i = TempBufferVerticesStored;
	int j;
	for(j = 0; j < 3; j++){
		v = &waterVertices[t->indices[j]];
		TempVertexBuffer[i + j].setX(v->pos.x);
		TempVertexBuffer[i + j].setY(v->pos.y);
		TempVertexBuffer[i + j].setZ(v->pos.z);
		TempVertexBuffer[i + j].setU(v->pos.x*uvscale);
		TempVertexBuffer[i + j].setV(v->pos.y*uvscale);
		TempVertexBuffer[i + j].setColor(color.red, color.green, color.blue, color.alpha);
	}

	TempBufferVerticesStored += 3;
	TempIndexBuffer[TempBufferIndicesStored++] = i+0;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
	TempIndexBuffer[TempBufferIndicesStored++] = i+2;
}

void
RenderWater(void)
{
	int i;

	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::FOGENABLE, gEnableFog);
	rw::engine->imtexture = gpWaterTex;

	for(i = 0; i < numWaterQuads; i++)
		RenderWaterQuad(&waterQuads[i]);
	for(i = 0; i < numWaterTris; i++)
		RenderWaterTri(&waterTris[i]);
	RenderAndEmptyRenderBuffer();
}

void
Render(void)
{
	SetRenderState(rw::CULLMODE, rw::CULLNONE);
	if(params.water == GAME_SA)
		RenderWater();
	else
		RenderWaterpro();
}

}