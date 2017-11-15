#include <rw.h>
#include <skeleton.h>
#include <assert.h>

#include <rwgta.h>
#define PS2

using namespace std;
using rw::int8;
using rw::uint8;
using rw::int16;
using rw::uint16;
using rw::int32;
using rw::uint32;
using rw::float32;
using rw::bool32;

typedef uint16 float16;
#include <rsl.h>

#include "templates.h"

#include <relocchunk.h>

#include <leedsgta.h>
#include <streamworld.h>

#include "Pad.h"
#include "Camera.h"
#include "TexListStore.h"

void panic(const char *fmt, ...);
void debug(const char *fmt, ...);

void plCapturePad(int arg);
void plUpdatePad(CControllerState *state);

struct Zfile;
Zfile *zopen(const char *path, const char *mode);
void zclose(Zfile *zf);
int zread(Zfile *zf, void *dst, int n);
int ztell(Zfile *zf);

struct SceneGlobals {
	rw::World *world;
	rw::Camera *camera;
};
extern SceneGlobals Scene;

enum eLevel
{
#ifdef LCS
	LEVEL_INDUST = 1,
	LEVEL_COMMER = 2,
	LEVEL_SUBURB = 3,
	LEVEL_UNDERG = 4,
#else
	LEVEL_BEACH = 1,
	LEVEL_MAINLAND = 2,
	LEVEL_MALL = 3,
#endif
};

extern int frameCounter;
extern CCamera TheCamera;
extern int drawLOD;

// helpers
extern rw::Geometry *cubeGeo;
extern rw::Material *cubeMat;
extern rw::Light *pAmbient;

bool GetIsTimeInRange(uint8 h1, uint8 h2);

rw::Raster *convertRasterPS2(RslRasterPS2 *ras);


enum SectorType
{
	SECTOR_NA,
	SECTOR_WORLD,
	SECTOR_INTERIOR,
	SECTOR_TRIG,	// only resources
};

// streamed world
struct BuildingExt
{
	bool isTimed;
	uint8 timeOn, timeOff;
	bool hidden;
	bool isTransparent;

	struct Model {
		int lastFrame;
		int resId;
		Model *next;
	};
	Model *resources;

	Model *GetResourceInfo(int id);
};
struct SectorExt
{
	Sector *sect;
	rw::V3d origin;
	SectorType type;
	int secx, secy;	// world sector indices
	// for triggered sectors
	bool isTimed;
	uint8 timeOn, timeOff;
	bool hidden;

	int numInstances;
	rw::Atomic **instances;
	rw::Atomic **dummies;
};
struct ResourceExt
{
	SectorExt *sector;
};
struct LevelExt
{
	sLevelChunk *chunk;
	FILE *imgfile;
	int numWorldSectors;
	int numSectors;
	SectorExt *sectors;
#ifdef VCS
	Area **areas;
#endif
	BuildingExt *buildings[0x8000];
	ResourceExt *res;
};
extern LevelExt *gLevel;
extern SectorExt *worldSectors[NUMSECTORSX][NUMSECTORSY];
void LoadLevel(eLevel lev);
void LoadSector(int n);
void LoadArea(int n);
void renderCubesSector(SectorExt*);
void renderSector(SectorExt*);

namespace Renderer
{
extern rw::ObjPipeline *buildingPipe;
void renderCubesIPL(void);
void renderPathNodes(void);
void reset(void);
void addToOpaqueRenderList(rw::Atomic *a);
void addToTransparentRenderList(rw::Atomic *a);
void renderOpaque(void);
void renderTransparent(void);
};
rw::ObjPipeline *makeBuildingPipe(void);

typedef CPool<CBuilding, CBuilding> BuildingPool;
typedef CPool<CTreadable, CTreadable> TreadablePool;
typedef CPool<CDummy, CDummy> DummyPool;
extern BuildingPool *pBuildingPool;
extern TreadablePool *pTreadablePool;
extern DummyPool *pDummyPool;
extern CTimeCycle *pTimecycle;
extern rw::RGBA currentAmbient;
extern rw::RGBA currentEmissive;
extern CPathFind *gpThePaths;

struct CModelInfo
{
	static int msNumModelInfos;
	static CBaseModelInfo **ms_modelInfoPtrs;
	static void Load(int n, CBaseModelInfo **mi);
	static CBaseModelInfo *Get(int n);
};

struct CWaterLevel_ : CWaterLevel
{
	static CWaterLevel_ *mspInst;
	static void Initialize(CWaterLevel *wl);
	static void RenderAndEmptyRenderBuffer(void);
	void RenderWater(void);
	static void RenderOneFlatSmallWaterPoly(float x, float y, float z, rw::RGBA const &color);
	static void RenderOneFlatLargeWaterPoly(float x, float y, float z, rw::RGBA const &color);
};
