#ifdef _WIN32
#include <Windows.h>	// necessary for the moment
#endif

#include <rw.h>
#include <skeleton.h>
#include "imgui/ImGuizmo.h"
#include <string.h>
#include <ctype.h>
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
using rw::int64;
using rw::uint64;
using rw::float32;
using rw::bool32;
using rw::uintptr;

struct ObjectInst;

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
inline float clamp(float v, float min, float max){ return v<min ? min : v>max ? max : v; }
inline float sq(float x){ return x*x; }

void plCapturePad(int arg);
void plUpdatePad(CControllerState *state);

void ConvertTxd(rw::TexDictionary *txd);

extern float timeStep;
extern float avgTimeStep;

#define DEGTORAD(d) (d/180.0f*PI)

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

// non-rendering things
extern bool gRenderCollision;
extern bool gRenderZones;
extern bool gRenderMapZones;
extern bool gRenderNavigZones;
extern bool gRenderInfoZones;
extern bool gRenderCullZones;
extern bool gRenderAttribZones;
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


// These don't necessarily match the game's values, roughly double of SA PC
enum {
	MODELNAMELEN = 30,
	NUMOBJECTDEFS = 40000,
	NUMTEXDICTS = 10000,
	NUMCOLS = 510,
	NUMSCENES = 80,
	NUMIPLS = 512,
	NUMCDIMAGES = 100,
	NUMTCYCBOXES = 64,

	NUMWATERVERTICES = 4000,
	NUMWATERQUADS = 1000,
	NUMWATERTRIS = 1000,
	NUMZONES = 500,	// for each type
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
	char *name;
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
};
extern rw::TexDictionary *defaultTxd;
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
};
extern CPtrList instances;
extern CPtrList selection;
ObjectInst *GetInstanceByID(int32 id);
ObjectInst *AddInstance(void);
void ClearSelection(void);


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

void RenderInit(void);
void BuildRenderList(void);
void RenderOpaque(void);
void RenderTransparent(void);
void RenderEverything(void);

// Debug Render
void RenderLine(rw::V3d v1, rw::V3d v2, rw::RGBA c1, rw::RGBA c2);
void RenderWireBoxVerts(rw::V3d *verts, rw::RGBA col);
void RenderWireBox(CBox *box, rw::RGBA col, rw::Matrix *xform);
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
