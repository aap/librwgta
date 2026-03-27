#ifdef _WIN32
#include <Windows.h>	// necessary for the moment
#endif

#include <rw.h>
#include <skeleton.h>
#include "imgui/ImGuizmo.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "ini_parser.hpp"
#include "Rect.h"
#include <rwgta.h>
#define PS2

//using namespace std;
using rw::int8;
using rw::uint8;
using rw::int16;
using rw::uint16;
using rw::int32;
using rw::uint32;
using rw::int64;
using rw::uint64;
using rw::float32;
using rw::bool32;
using rw::uintptr;
typedef unsigned int uint;

struct ObjectInst;

#ifdef RWHALFPIXEL
#define HALFPX (0.5f)
#else
#define HALFPX (0.0f)
#endif

#include "Pad.h"
#include "camera.h"
#include "collision.h"

void panic(const char *fmt, ...);
void debug(const char *fmt, ...);
void log(const char *fmt, ...);
void addToLogWindow(const char *fmt, va_list args);

char *getPath(const char *path);
FILE *fopen_ci(const char *path, const char *mode);
bool doesFileExist(const char *path);
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
template <typename T> T min(T a, T b) { return a < b ? a : b; }
template <typename T> T max(T a, T b) { return a > b ? a : b; }
template <typename T> T clamp(T v, T min, T max) { return v<min ? min : v>max ? max : v; }
template <typename T> T sq(T a) { return a*a; }

void plCapturePad(int arg);
void plUpdatePad(CControllerState *state);

void ConvertTxd(rw::TexDictionary *txd);

extern float timeStep;
extern float avgTimeStep;

#define DEGTORAD(d) (d/180.0f*PI)

struct Ray {
	rw::V3d start;
	rw::V3d dir;
};
inline bool
SphereIntersect(const CSphere &sph, const Ray &ray)
{
	rw::V3d diff = sub(ray.start, sph.center);
	float a = dot(ray.dir,ray.dir);
	float b = 2*dot(ray.dir, diff);
	float c = dot(diff,diff) - sq(sph.radius);

	float discr = sq(b) - 4*a*c;
	return discr > 0.0f;
}

//
// Options
//

extern bool gRenderOnlyLod;
extern bool gRenderOnlyHD;
extern bool gRenderBackground;
extern bool gRenderWater;
extern bool gRenderPostFX;
extern bool gEnableFog;
extern bool gEnableTimecycleBoxes;
extern bool gUseBlurAmb;
extern bool gOverrideBlurAmb;
extern bool gNoTimeCull;
extern bool gNoAreaCull;
extern bool gDoBackfaceCulling;
extern bool gPlayAnimations;
extern bool gUseViewerCam;
extern bool gDrawTarget;

// non-rendering things
extern bool gRenderCollision;
extern bool gRenderZones;
extern bool gRenderMapZones;
extern bool gRenderNavigZones;
extern bool gRenderInfoZones;
extern bool gRenderCullZones;
extern bool gRenderAttribZones;
extern bool gRenderPedPaths;
extern bool gRenderCarPaths;
extern bool gRenderEffects;
extern bool gRenderTimecycleBoxes;

// SA postfx
extern int  gColourFilter;
extern bool gRadiosity;

// SA building pipe
extern int gBuildingPipeSwitch;
extern float gDayNightBalance;
extern float gWetRoadEffect;

// Neo stuff
extern float gNeoLightMapStrength;

// Misc
bool FileExists(const char *filename);

// Game

enum eGame
{
	GAME_NA,
	GAME_III,
	GAME_VC,
	GAME_SA,
	GAME_LCS,
	GAME_VCS
};
using rw::PLATFORM_NULL;
using rw::PLATFORM_PS2;
using rw::PLATFORM_XBOX;
enum {
	PLATFORM_PC = rw::PLATFORM_D3D8,
	PLATFORM_PSP = 10
};
extern int gameversion;
extern int gameplatform;
inline bool isIII(void) { return gameversion == GAME_III; }
inline bool isVC(void) { return gameversion == GAME_VC; }
inline bool isSA(void) { return gameversion == GAME_SA; }

// Config

int readhex(const char *str);
int readint(const std::string& s, int default = 0);
float readfloat(const std::string& s, float default = 0);
 
// DK 26-03-2026: I've kept the fields the same as they were in the enum, with some extra additions, later we'd need to move some into their respective game structs (so for example cull zones for gta3/sa can have different settings)
struct Config
{
	int numObjectDefs;
	int numTexDicts;
	int numCols;
	int numScenes;
	int numIpls;
	int numCdImages;
	int numTcycBoxes;

	int numWaterVerts;
	int numWaterQuads;
	int numWaterTris;
	int numZones; // for each type

	int numCullZones;
	int numAttribZones;
	int numMirrorAttribZones;

	struct {
		int maxNumColBoxes;
		int maxNumColSpheres;
		int maxNumColTriangles;

		int numSectorsX;
		int numSectorsY;
		CRect worldBounds;
	} gta3;

	struct {
		int numSectorsX;
		int numSectorsY;
		CRect worldBounds;
	} gtavc;

	struct {
		int numSectorsX;
		int numSectorsY;
		CRect worldBounds;
	} gtasa;

	struct {
		int numSectorsX;
		int numSectorsY;
		CRect worldBounds;
	} gtalcs;

	struct {
		int numSectorsX;
		int numSectorsY;
		CRect worldBounds;
	} gtavcs;

	// Defaults; these don't necessarily match the game's values, roughly double of SA PC
	Config() {
		numObjectDefs = 40000;
		numTexDicts = 10000;
		numCols = 510;
		numScenes = 80;
		numIpls = 512;
		numCdImages = 100;
		numTcycBoxes = 64;

		numWaterVerts = 4000;
		numWaterQuads = 1000;
		numWaterTris = 1000;
		numZones = 500;

		//numCullZones = 800;
		numCullZones = 1000;
		numAttribZones = 2000;
		numMirrorAttribZones = 200;

		// game specific settings
		gta3.maxNumColBoxes = 32;
		gta3.maxNumColSpheres = 128;
		gta3.maxNumColTriangles = 600;
		gta3.numSectorsX = 100;
		gta3.numSectorsY = 100;
		gta3.worldBounds.left = -2000.0f;
		gta3.worldBounds.bottom = -2000.0f;
		gta3.worldBounds.right = 2000.0f;
		gta3.worldBounds.top = 2000.0f;

		gtavc.numSectorsX = 80;
		gtavc.numSectorsY = 80;
		gtavc.worldBounds.left = -2400.0f;
		gtavc.worldBounds.bottom = -2000.0f;
		gtavc.worldBounds.right = 1600.0f;
		gtavc.worldBounds.top = 2000.0f;

		gtasa.numSectorsX = 120;
		gtasa.numSectorsY = 120;
		gtasa.worldBounds.left = -3000.0f;
		gtasa.worldBounds.bottom = -3000.0f;
		gtasa.worldBounds.right = 3000.0f;
		gtasa.worldBounds.top = 3000.0f;

		gtalcs.numSectorsX = 100;
		gtalcs.numSectorsY = 100;
		gtalcs.worldBounds.left = -2000.0f;
		gtalcs.worldBounds.bottom = -2000.0f;
		gtalcs.worldBounds.right = 2000.0f;
		gtalcs.worldBounds.top = 2000.0f;

		gtavcs.numSectorsX = 80;
		gtavcs.numSectorsY = 80;
		gtavcs.worldBounds.left = -2400.0f;
		gtavcs.worldBounds.bottom = -2000.0f;
		gtavcs.worldBounds.right = 1600.0f;
		gtavcs.worldBounds.top = 2000.0f;
	}

	void WriteDefaultConfig() {
		linb::ini cfg;
		cfg.set("Default", "numObjectDefs", std::to_string(numObjectDefs));
		cfg.set("Default", "numTexDicts", std::to_string(numTexDicts));
		cfg.set("Default", "numCols", std::to_string(numCols));
		cfg.set("Default", "numScenes", std::to_string(numScenes));
		cfg.set("Default", "numIpls", std::to_string(numIpls));
		cfg.set("Default", "numCdImages", std::to_string(numCdImages));
		cfg.set("Default", "numTcycBoxes", std::to_string(numTcycBoxes));
		cfg.set("Default", "numWaterVerts", std::to_string(numWaterVerts));
		cfg.set("Default", "numWaterQuads", std::to_string(numWaterQuads));
		cfg.set("Default", "numWaterTris", std::to_string(numWaterTris));
		cfg.set("Default", "numZones", std::to_string(numZones));
		cfg.set("Default", "numCullZones", std::to_string(numCullZones));
		cfg.set("Default", "numAttribZones", std::to_string(numAttribZones));
		cfg.set("Default", "numMirrorAttribZones", std::to_string(numMirrorAttribZones));

		switch (gameversion) {
		case GAME_III:
			cfg.set("Default", "maxNumColBoxes", std::to_string(gta3.maxNumColBoxes));
			cfg.set("Default", "maxNumColSpheres", std::to_string(gta3.maxNumColSpheres));
			cfg.set("Default", "maxNumColTriangles", std::to_string(gta3.maxNumColTriangles));
			cfg.set("Default", "numSectorsX", std::to_string(gta3.numSectorsX));
			cfg.set("Default", "numSectorsY", std::to_string(gta3.numSectorsY));
			cfg.set("Default", "worldBoundsLeft", std::to_string(gta3.worldBounds.left));
			cfg.set("Default", "worldBoundsBottom", std::to_string(gta3.worldBounds.bottom));
			cfg.set("Default", "worldBoundsRight", std::to_string(gta3.worldBounds.right));
			cfg.set("Default", "worldBoundsTop", std::to_string(gta3.worldBounds.top));
			break;

		case GAME_VC:
			cfg.set("Default", "numSectorsX", std::to_string(gtavc.numSectorsX));
			cfg.set("Default", "numSectorsY", std::to_string(gtavc.numSectorsY));
			cfg.set("Default", "worldBoundsLeft", std::to_string(gtavc.worldBounds.left));
			cfg.set("Default", "worldBoundsBottom", std::to_string(gtavc.worldBounds.bottom));
			cfg.set("Default", "worldBoundsRight", std::to_string(gtavc.worldBounds.right));
			cfg.set("Default", "worldBoundsTop", std::to_string(gtavc.worldBounds.top));
			break;

		case GAME_SA:
			cfg.set("Default", "numSectorsX", std::to_string(gtasa.numSectorsX));
			cfg.set("Default", "numSectorsY", std::to_string(gtasa.numSectorsY));
			cfg.set("Default", "worldBoundsLeft", std::to_string(gtasa.worldBounds.left));
			cfg.set("Default", "worldBoundsBottom", std::to_string(gtasa.worldBounds.bottom));
			cfg.set("Default", "worldBoundsRight", std::to_string(gtasa.worldBounds.right));
			cfg.set("Default", "worldBoundsTop", std::to_string(gtasa.worldBounds.top));
			break;

		case GAME_LCS:
			cfg.set("Default", "numSectorsX", std::to_string(gtalcs.numSectorsX));
			cfg.set("Default", "numSectorsY", std::to_string(gtalcs.numSectorsY));
			cfg.set("Default", "worldBoundsLeft", std::to_string(gtalcs.worldBounds.left));
			cfg.set("Default", "worldBoundsBottom", std::to_string(gtalcs.worldBounds.bottom));
			cfg.set("Default", "worldBoundsRight", std::to_string(gtalcs.worldBounds.right));
			cfg.set("Default", "worldBoundsTop", std::to_string(gtalcs.worldBounds.top));
			break;

		case GAME_VCS:
			cfg.set("Default", "numSectorsX", std::to_string(gtavcs.numSectorsX));
			cfg.set("Default", "numSectorsY", std::to_string(gtavcs.numSectorsY));
			cfg.set("Default", "worldBoundsLeft", std::to_string(gtavcs.worldBounds.left));
			cfg.set("Default", "worldBoundsBottom", std::to_string(gtavcs.worldBounds.bottom));
			cfg.set("Default", "worldBoundsRight", std::to_string(gtavcs.worldBounds.right));
			cfg.set("Default", "worldBoundsTop", std::to_string(gtavcs.worldBounds.top));
			break;
		}

		cfg.write_file("euryopa.ini");
	}

	void Load() {
		if (!FileExists("euryopa.ini")) 
		{
			WriteDefaultConfig();
			return;
		}

		linb::ini cfg;
		cfg.load_file("euryopa.ini");

		numObjectDefs = readint(cfg.get("Default", "numObjectDefs", ""), 40000);
		numTexDicts = readint(cfg.get("Default", "numTexDicts", ""), 10000);
		numCols = readint(cfg.get("Default", "numCols", ""), 510);
		numScenes = readint(cfg.get("Default", "numScenes", ""), 80);
		numIpls = readint(cfg.get("Default", "numIpls", ""), 512);
		numCdImages = readint(cfg.get("Default", "numCdImages", ""), 100);
		numTcycBoxes = readint(cfg.get("Default", "numTcycBoxes", ""), 64);
		numWaterVerts = readint(cfg.get("Default", "numWaterVerts", ""), 4000);
		numWaterQuads = readint(cfg.get("Default", "numWaterQuads", ""), 1000);
		numWaterTris = readint(cfg.get("Default", "numWaterTris", ""), 1000);
		numZones = readint(cfg.get("Default", "numZones", ""), 500);
		numCullZones = readint(cfg.get("Default", "numCullZones", ""), 1000);
		numAttribZones = readint(cfg.get("Default", "numAttribZones", ""), 2000);
		numMirrorAttribZones = readint(cfg.get("Default", "numMirrorAttribZones", ""), 200);

		switch (gameversion) {
		case GAME_III:
			gta3.maxNumColBoxes = readint(cfg.get("Default", "maxNumColBoxes", ""), 32);
			gta3.maxNumColSpheres = readint(cfg.get("Default", "maxNumColSpheres", ""), 128);
			gta3.maxNumColTriangles = readint(cfg.get("Default", "maxNumColTriangles", ""), 600);
			gta3.numSectorsX = readint(cfg.get("Default", "numSectorsX", ""), 100);
			gta3.numSectorsY = readint(cfg.get("Default", "numSectorsY", ""), 100);
			gta3.worldBounds.left = readfloat(cfg.get("Default", "worldBoundsLeft", ""), -2000.0f);
			gta3.worldBounds.bottom = readfloat(cfg.get("Default", "worldBoundsBottom", ""), -2000.0f);
			gta3.worldBounds.right = readfloat(cfg.get("Default", "worldBoundsRight", ""), 2000.0f);
			gta3.worldBounds.top = readfloat(cfg.get("Default", "worldBounds.Top", ""), 2000.0f);
			break;

		case GAME_VC:
			gtavc.numSectorsX = readint(cfg.get("Default", "numSectorsX", ""), 80);
			gtavc.numSectorsY = readint(cfg.get("Default", "numSectorsY", ""), 80);
			gtavc.worldBounds.left = readfloat(cfg.get("Default", "worldBoundsLeft", ""), -2400.0f);
			gtavc.worldBounds.bottom = readfloat(cfg.get("Default", "worldBoundsBottom", ""), -2000.0f);
			gtavc.worldBounds.right = readfloat(cfg.get("Default", "worldBoundsRight", ""), 1600.0f);
			gtavc.worldBounds.top = readfloat(cfg.get("Default", "worldBoundsTop", ""), 2000.0f);
			break;

		case GAME_SA:
			gtasa.numSectorsX = readint(cfg.get("Default", "numSectorsX", ""), 120);
			gtasa.numSectorsY = readint(cfg.get("Default", "numSectorsY", ""), 120);
			gtasa.worldBounds.left = readfloat(cfg.get("Default", "worldBoundsLeft", ""), -3000.0f);
			gtasa.worldBounds.bottom = readfloat(cfg.get("Default", "worldBoundsBottom", ""), -3000.0f);
			gtasa.worldBounds.right = readfloat(cfg.get("Default", "worldBoundsRight", ""), 3000.0f);
			gtasa.worldBounds.top = readfloat(cfg.get("Default", "worldBoundsTop", ""), 3000.0f);
			break;

		case GAME_LCS:
			gtalcs.numSectorsX = readint(cfg.get("Default", "numSectorsX", ""), 100);
			gtalcs.numSectorsY = readint(cfg.get("Default", "numSectorsY", ""), 100);
			gtalcs.worldBounds.left = readfloat(cfg.get("Default", "worldBoundsLeft", ""), -2000.0f);
			gtalcs.worldBounds.bottom = readfloat(cfg.get("Default", "worldBoundsBottom", ""), -2000.0f);
			gtalcs.worldBounds.right = readfloat(cfg.get("Default", "worldBoundsRight", ""), 2000.0f);
			gtalcs.worldBounds.top = readfloat(cfg.get("Default", "worldBoundsTop", ""), 2000.0f);
			break;

		case GAME_VCS:
			gtavcs.numSectorsX = readint(cfg.get("Default", "numSectorsX", ""), 80);
			gtavcs.numSectorsY = readint(cfg.get("Default", "numSectorsY", ""), 80);
			gtavcs.worldBounds.left = readfloat(cfg.get("Default", "worldBoundsLeft", ""), -2400.0f);
			gtavcs.worldBounds.bottom = readfloat(cfg.get("Default", "worldBoundsBottom", ""), -2000.0f);
			gtavcs.worldBounds.right = readfloat(cfg.get("Default", "worldBoundsRight", ""), 1600.0f);
			gtavcs.worldBounds.top = readfloat(cfg.get("Default", "worldBoundsTop", ""), 2000.0f);
			break;
		}
	}
};

extern Config globalConfig;

enum {
	MODELNAMELEN = 30
};

#define LODDISTANCE (300.0f)

#include "Rect.h"
#include "PtrNode.h"
#include "PtrList.h"

struct CRGBA
{
	uint8 r, g, b, a;
};

#include "timecycle.h"
#include "Sprite.h"

namespace Zones
{
void Init();
void CreateZone(const char *name, int type, CBox box, int level, const char *text);
void Render(void);
void AddAttribZone(CBox box, int flags, int wantedLevelDrop);
void AddAttribZone(rw::V3d pos, float s1x, float s1y,
	float s2x, float s2y, float zmin, float zmax, int flags);
void AddMirrorAttribZone(rw::V3d pos, float s1x, float s1y,
	float s2x, float s2y, float zmin, float zmax,
	int flags, rw::Plane mirror);
void RenderAttribZones(void);
void RenderCullZones(void);
}

struct WeatherInfo;

struct Params
{
	int map;
	rw::V3d initcampos;
	rw::V3d initcamtarg;
	int numAreas;
	const char **areaNames;

	int objFlagset;

	int timecycle;
	int numHours;
	int numWeathers;
	int extraColours;	// weather ID where extra colours start
	int numExtraColours;	// number of extra colour blocks
	WeatherInfo *weatherInfo;
	int background;
	int daynightPipe;

	int water;
	const char *waterTex;
	rw::V2d waterStart, waterEnd;	// waterpro

	int alphaRefDefault;	// preset value
	int alphaRef;		// the regular one we want to use for rendering
	bool ps2AlphaTest;	// emulate PS2 alpha test

	bool backfaceCull;
	bool txdFallbackGeneric;

	int neoWorldPipe;

	int leedsPipe;

	bool checkColModels;
	int maxNumColSpheres;
	int maxNumColBoxes;
	int maxNumColTriangles;
};
extern Params params;

extern int gameTxdSlot;

extern int currentHour, currentMinute;
extern int extraColours;
extern int currentArea;

struct WeatherInfo
{
	const char *name;
	int flags;
};

namespace Weather
{
enum {
	Sunny = 0x01,
	Foggy = 0x02,
	Extrasunny = 0x04,
};

extern int oldWeather, newWeather;
extern float interpolation;
extern float cloudCoverage;
extern float foggyness;
extern float extraSunnyness;

void Update(void);
};

// TODO, this is a stub
struct GameFile
{
	char *name;
	// types of files we want to have eventually:
	//	IDE Definition
	//	IPL Scene
	//	COL ?
	//	IMG ?
};
GameFile *NewGameFile(char *path);

bool IsHourInRange(int h1, int h2);
void FindVersion(void);
void LoadGame(void);
void Idle(void);
void DefinedState(void);

// Game Data structures

void AllocateCdImageList();
void AddCdImage(const char *path);
void InitCdImages(void);
uint8 *ReadFileFromImage(int i, int *size);
GameFile *GetGameFileFromImage(int i);
void RequestObject(int id);
void LoadAllRequestedObjects(void);


struct TxdDef
{
	char name[MODELNAMELEN];
	rw::TexDictionary *txd;
	int parentId;
	int32 imageIndex;
	int32 refCount;	// just for information
};
extern rw::TexDictionary *defaultTxd;
void InitTxdStore();
void RegisterTexStorePlugin(void);
TxdDef *GetTxdDef(int i);
int FindTxdSlot(const char *name);
int AddTxdSlot(const char *name);
void TxdPush(void);
void TxdPop(void);
bool IsTxdLoaded(int i);
void CreateTxd(int i);
void LoadTxd(int i);
void LoadTxd(int i, const char *path);
void TxdMakeCurrent(int i);
void TxdSetParent(const char *child, const char *parent);


struct ColFileHeader
{
	uint32 fourcc;
	uint32 modelsize;
	char name[24];
};

struct ColDef
{
	char name[MODELNAMELEN];
	int32 imageIndex;
};
void InitColStore();
ColDef *GetColDef(int i);
int AddColSlot(const char *name);
void LoadCol(int slot);
void LoadAllCollisions(void);

// One class for all map objects
struct ObjectDef
{
	enum Type {
		ATOMIC,
		CLUMP
	};

	int m_id;	// our own id
	char m_name[MODELNAMELEN];
	int m_txdSlot;
	int m_type;
	CColModel *m_colModel;
	bool m_gotChildCol;
	int m_pedPathIndex;
	int m_carPathIndex;
	int m_effectIndex;
	int m_numEffects;
	int m_numInstances;

	// flags
	bool m_normalCull;	// only III
	bool m_noFade;
	bool m_drawLast;
	bool m_additive;
	bool m_isSubway;	// only III?
	bool m_ignoreLight;
	bool m_noZwrite;
	// VC
	bool m_wetRoadReflection;
	bool m_noShadows;
	bool m_ignoreDrawDist;	// needs a better name perhaps
	bool m_isCodeGlass;
	bool m_isArtistGlass;
	// SA Base
	bool m_noBackfaceCulling;
	// SA Atomic
	bool m_dontCollideWithFlyer;
	bool m_isGarageDoor;
	bool m_isDamageable;
	bool m_isTree;
	bool m_isPalmTree;
	bool m_isTag;
	bool m_noCover;
	bool m_wetOnly;
	// SA Clump
	bool m_isDoor;

	// atomic info
	int m_numAtomics;
	float m_drawDist[3];
	rw::Atomic *m_atomics[3];
	// time objects
	bool m_isTimed;
	int m_timeOn, m_timeOff;

	// clump info
	rw::Clump *m_clump;
	char m_animname[MODELNAMELEN];

	bool m_cantLoad;
	bool m_hasPreRendered;
	int32 m_imageIndex;
	float m_minDrawDist;
	bool m_isBigBuilding;
	bool m_isHidden;
	ObjectDef *m_relatedModel;
	ObjectDef *m_relatedTimeModel;

	GameFile *m_file;

	float GetLargestDrawDist(void);
	rw::Atomic *GetAtomicForDist(float dist);
	bool IsLoaded(void);
	void LoadAtomic(void);
	void LoadClump(void);
	void Load(void);
	void SetAtomic(int n, rw::Atomic *atomic);
	void SetClump(rw::Clump *clump);
	void CantLoad(void);
	void SetupBigBuilding(int first, int last);
	void SetFlags(int flags);
};
void InitModelInfo();
ObjectDef *AddObjectDef(int id);
ObjectDef *GetObjectDef(int id);
ObjectDef *GetObjectDef(const char *name, int *id);


struct FileObjectInstance
{
	rw::V3d position;
	rw::Quat rotation;
	int objectId;
	int area;
	int lod;
};

struct ObjectInst
{
	rw::V3d m_translation;
	rw::Quat m_rotation;
	// cached form of the above
	rw::Matrix m_matrix;
	int m_objectId;
	int m_area;

	void *m_rwObject;
	bool m_isBigBuilding;
	uint16 m_scanCode;

	// SA only
	int m_lodId;
	int m_iplSlot;
	ObjectInst *m_lod;
	int m_numChildren;	// hq versions
	int m_numChildrenRendered;
	// SA flags
	bool m_isUnimportant;
	bool m_isUnderWater;
	bool m_isTunnel;
	bool m_isTunnelTransition;

	// additional stuff
	int32 m_id;	// to identify when picking
	int m_selected;
	int m_highlight;	// various ways to highlight this object

	GameFile *m_file;

	void UpdateMatrix(void);
	void *CreateRwObject(void);
	void Init(FileObjectInstance *fi);
	void SetupBigBuilding(void);
	CRect GetBoundRect(void);
	bool IsOnScreen(void);
	void PreRender(void);

	void JumpTo(void);
	void Select(void);
	void Deselect(void);
	void ToggleSelect(void);
};
extern CPtrList instances;
extern CPtrList selection;
ObjectInst *GetInstanceByID(int32 id);
ObjectInst *AddInstance(void);
void ClearSelection(void);



enum EffectType {
	FX_LIGHT,
	FX_PARTICLE,
	FX_LOOKATPOINT,
	FX_PEDQUEUE,
	FX_SUNGLARE
};

enum FlareType {
        FLARE_NONE,
        FLARE_SUN,
        FLARE_HEADLIGHTS
};

// III and VC for now
struct Effect {
	int id;
	rw::V3d pos;
	rw::RGBA col;
	int type;
	struct Light {
		float lodDist;
		float size;
		float coronaSize;
		float shadowSize;
		int flashiness;
		int reflection;
		int lensFlareType;
		int shadowAlpha;
		int flags;
		char coronaTex[32];
		char shadowTex[32];
	};
	struct Particle {
		int particleType;
		rw::V3d dir;
		float size;
	};
	struct LookAtPoint {
		rw::V3d dir;
		int type;
		int probability;
	};
	struct PedQueue {
		rw::V3d queueDir;
		rw::V3d useDir;
		int type;
	};
	union {
		Light light;
		Particle prtcl;
		LookAtPoint look;
		PedQueue queue;
		// glare has no extra data
	};

	void JumpTo(ObjectInst *inst);
};

namespace Effects {
extern Effect *hoveredEffect, *guiHoveredEffect;
extern Effect *selectedEffect;
void AddEffect(Effect e);
Effect *GetEffect(int idx);
void Render(void);
}


enum PathType {
	PedPath,
	CarPath,
	WaterPath
};

#define LaneWidth 5.0f
struct PathNode {
	int type;
	int link;
	int linkType;
	float x, y, z;
	float width;
	int lanesIn, lanesOut;
	// VC
	int speed;
	int flags;
	float density;
	// SA
	int special;

	enum Type {
		NodeNone = 0,
		NodeExternal,
		NodeInternal
	};
	enum Flags {
		NodeDisabled = 1,
		NodeRoadBlock = 2,
		NodeBetweenLevels = 4,
		NodeUnderBridge = 8
	};

	bool water;
	int lanesInX, lanesOutX;

	int idx;
	int tabId;
	int objId;
	// for internal nodes
	int numLinks;
	int links[12];

	float laneOffset(void) {
		if(lanesInX == 0)
			return 0.5f - 0.5f*lanesOutX;
		if(lanesOutX == 0)
			return 0.5f - 0.5f*lanesInX;
		return 0.5f + width/(2.0f*LaneWidth);
	}
	float laneOffsetIII(void) {
		if(lanesInX == 0)
			return 0.5f - 0.5f*lanesOutX;
		if(lanesOutX == 0)
			return 0.5f - 0.5f*lanesInX;
		return 0.5f;
	}
	void JumpTo(ObjectInst *inst);
	bool isDetached(void);
};

namespace Path {
extern PathNode *hoveredNode, *guiHoveredNode;
extern PathNode *selectedNode;
void AddNode(PathType type, int id, PathNode node);
PathNode *GetPedNode(int base, int i);
PathNode *GetCarNode(int base, int i);
PathNode *GetDetachedPedNode(int base, int i);
PathNode *GetDetachedCarNode(int base, int i);
void RenderPedPaths(void);
void RenderCarPaths(void);
}


// World/sectors

struct Sector
{
	CPtrList buildings;
	CPtrList buildings_overlap;
	CPtrList bigbuildings;
	CPtrList bigbuildings_overlap;
};
extern int numSectorsX, numSectorsY;
extern CRect worldBounds;
extern Sector *sectors;
extern Sector outOfBoundsSector;
void InitSectors(void);
Sector *GetSector(int ix, int iy);
//Sector *GetSector(float x, float y);
int GetSectorIndexX(float x);
int GetSectorIndexY(float x);
bool IsInstInBounds(ObjectInst *inst);
void InsertInstIntoSectors(ObjectInst *inst);


struct IplDef
{
	char name[MODELNAMELEN];
	int instArraySlot;

	int32 imageIndex;
};
void InitIplStore();
int AddInstArraySlot(int n);
ObjectInst **GetInstArray(int i);
IplDef *GetIplDef(int i);
int AddIplSlot(const char *name);
void LoadIpl(int i);

// File Loader

namespace FileLoader
{

extern GameFile *currentFile;

struct DatDesc
{
	char name[5];
	void (*handler)(char *line);

	static void *get(DatDesc *desc, const char *name);
};

char *LoadLine(FILE *f);
void LoadLevel(const char *filename);
rw::TexDictionary *LoadTexDictionary(const char *path);
}

// Rendering

enum HighlightStyle
{
	HIGHLIGHT_NONE,
	HIGHLIGHT_FILTER,
	HIGHLIGHT_SELECTION,
	HIGHLIGHT_HOVER,
};

struct SceneGlobals {
	rw::World *world;
	rw::Camera *camera;
};
extern rw::Light *pAmbient, *pDirect;
extern rw::Texture *whiteTex;
extern SceneGlobals Scene;
extern CCamera TheCamera;

bool32 instWhite(int type, uint8 *dst, uint32 numVertices, uint32 stride);

void myRenderCB(rw::Atomic *atomic);

// Neo World pipeline
extern rw::ObjPipeline *neoWorldPipe;
void MakeNeoWorldPipe(void);

// SA DN building pipeline
bool IsBuildingPipeAttached(rw::Atomic *atm);
void SetupBuildingPipe(rw::Atomic *atm);
void UpdateDayNightBalance(void);
// this should perhaps not be public
void GetBuildingEnvMatrix(rw::Atomic *atomic, rw::Frame *envframe, rw::RawMatrix *envmat);
extern rw::ObjPipeline *buildingPipe;
extern rw::ObjPipeline *buildingDNPipe;
void MakeCustomBuildingPipelines(void);

// Leeds building pipeline
extern rw::ObjPipeline *leedsPipe;
void MakeLeedsPipe(void);

void RegisterPipes(void);
void RenderInit(void);
void BuildRenderList(void);
void RenderOpaque(void);
void RenderTransparent(void);
void RenderEverything(void);

// Debug Render
void RenderLine(rw::V3d v1, rw::V3d v2, rw::RGBA c1, rw::RGBA c2);
void RenderWireBoxVerts(rw::V3d *verts, rw::RGBA col);
void RenderWireBox(CBox *box, rw::RGBA col, rw::Matrix *xform);
void RenderSphereAsWireBox(CSphere *sphere, rw::RGBA col, rw::Matrix *xform);
void RenderSphereAsCross(CSphere *sphere, rw::RGBA col, rw::Matrix *xform);
void RenderWireSphere(CSphere *sphere, rw::RGBA col, rw::Matrix *xform);
void RenderWireTriangle(rw::V3d *v1, rw::V3d *v2, rw::V3d *v3, rw::RGBA col, rw::Matrix *xform);
void RenderAxesWidget(rw::V3d pos, rw::V3d x, rw::V3d y, rw::V3d z);

void RenderEverythingCollisions(void);
void RenderDebugLines(void);


void RenderPostFX(void);


namespace WaterLevel
{
	void Initialise(void);
	void Render(void);
};

namespace Clouds
{
	extern float CloudRotation;

	void Init(void);
	// III and VC
	void RenderBackground(int16 topred, int16 topgreen, int16 topblue,
		int16 botred, int16 botgreen, int16 botblue, int16 alpha);
	void RenderHorizon(void);

	void RenderLowClouds(void);
	void RenderFluffyClouds(void);
	// SA
	void RenderSkyPolys(void);
}


//
// GUI
//

void gui(void);
void uiShowCdImages(void);
