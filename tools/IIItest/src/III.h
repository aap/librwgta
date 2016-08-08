#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>

#ifdef _WIN32
#define RW_D3D9
#else
// RW_GL3 defined by makefile

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#endif

#include <rw.h>
#include "rwgta.h"
#include "collision.h"

using namespace gta;

using rw::uint8;
using rw::int8;
using rw::uint16;
using rw::int16;
using rw::uint32;
using rw::int32;
typedef unsigned int uint;
typedef unsigned char uchar;

extern  uchar work_buff[55000];
void debug(const char *fmt, ...);

#include "Pad.h"
#include "templates.h"
#include "config.h"
#include "math/Vector.h"
#include "math/Matrix.h"

class C2dEffect
{
public:
	struct Light {
		float dist;
		float outerRange;
		float size;
		float innerRange;
		uchar flash;
		uchar wet;
		uchar flare;
		uchar shadowIntens;
		uchar flag;
		rw::Texture *corona;
		rw::Texture *shadow;
	};
	struct Particle {
		int particleType;
		float dir[3];
		float scale;
	};
	struct Attractor {
		float dir[3];
		uchar flag;
		uchar probability;
	};

	rw::V3d pos;
	rw::RGBA col;
	uchar type;
	union {
		Light light;
		Particle particle;
		Attractor attractor;
	};
};

#include "Timer.h"
#include "Clock.h"
#include "Weather.h"
#include "Rect.h"
#include "zones.h"
#include "Animation.h"
#include "ModelInfo.h"
#include "ModelIndices.h"
#include "PtrNode.h"
#include "PtrList.h"
#include "Placeable.h"
#include "Entity.h"
#include "Building.h"
#include "Treadable.h"
#include "Dummy.h"
#include "DummyObject.h"
#include "DummyPed.h"
#include "Physical.h"
#include "Camera.h"
#include "World.h"
#include "Pools.h"
#include "PathFind.h"
#include "ObjectData.h"
#include "Collision.h"

#include "TxdStore.h"
#include "TimeCycle.h"
#include "VisibilityPlugins.h"
#include "TempColModels.h"
#include "Game.h"
#include "Renderer.h"
#include "streaming.h"

inline float
clamp(float v, float min, float max){ return v<min ? min : v>max ? max : v; }

char *getPath(const char *path);
FILE *fopen_ci(const char *path, const char *mode);
char *skipWhite(char *s);
void convertTxd(rw::TexDictionary *txd);


// misc
extern CCamera TheCamera;
extern rw::Camera *rwCamera;
extern rw::World  *rwWorld;
extern rw::Light  *ambient;
extern rw::Light  *direct;
extern bool isRunning;
CVector FindPlayerCoors(void);
void SetLightsWithTimeOfDayColour(rw::World*);
void DefinedState(void);

void TheGame(void);

// platform implementations
int plGetTimeInMS(void);
bool plWindowclosed(void);
void plPresent(void);
void plHandleEvents(void);
void plCapturePad(int n);
void plUpdatePad(CControllerState *state);

struct StrAssoc
{
	const char *key;
	int val;

	static int get(StrAssoc *desc, const char *key);
};

struct DatDesc
{
	char name[5];
	void (*handler)(char *line);

	static void *get(DatDesc *desc, const char *name);
};

class CFileLoader
{
	static DatDesc ideDesc[];
	static DatDesc iplDesc[];
	static DatDesc zoneDesc[];
public:
	static char *LoadLine(FILE *f);
	static void LoadLevel(const char *filename);
	static void LoadObjectTypes(const char *filename);
	static void LoadScene(const char *filename);
	static void LoadMapZones(const char *filename);
	static void LoadDataFile(const char *filename, DatDesc *desc);
	static void LoadCollisionFile(const char *filename);

	static void LoadNothing(char *line) {}

	// IDE
	static void LoadObject(char *line);
	static void LoadTimeObject(char *line);
	static void LoadClumpObject(char *line);
	static void LoadVehicleObject(char *line);
	static void LoadPedObject(char *line);
	static void LoadPathLine(char *line);
	 static int  LoadPathHeader(char *line);
	 static void LoadPedPathNode(char *line, int id, int i);
	 static void LoadCarPathNode(char *line, int id, int i);
	static void Load2dEffect(char *line);

	// IPL
	static void LoadObjectInstance(char *line);
	static void LoadZone(char *line);
	static void LoadCullZone(char *line);
	static void LoadPickup(char *line);

	// ZONE
	static void LoadMapZone(char *line);

	// models
	static bool LoadAtomicFile(rw::Stream *stream, int id);
	static bool LoadClumpFile(rw::Stream *stream, int id);
	static void LoadClumpFile(const char *filename);
	static void LoadModelFile(const char *filename);

	// textures
	static rw::TexDictionary *LoadTexDictionary(const char *filename);
	static void AddTexDictionaries(rw::TexDictionary *dst,
	                               rw::TexDictionary *src);
};

class CPedType
{
public:
	enum {
		Player1, Player2, Player3, Player4,
		CivMale, CivFemale,
		Cop,
		Gang1, Gang2, Gang3, Gang4, Gang5,
		Gang6, Gang7, Gang8, Gang9,
		Emergency,
		Fireman,
		Criminal,
		Unused1, Unused2,
		Special,
		Prostitute,
		Invalid
	};
	static int FindPedType(const char *name);
};

class CPedStats
{
	struct Stats {
		int id;
		char name[24];
		float fleeDistance;
		float headingChangeRate;
		uchar fear;
		uchar temper;
		uchar lawfullness;
		uchar sexiness;
		float attackStrength;
		float defendWeakness;
		uint16 flags;

		Stats(void);
	};
	static Stats *ms_apPedStats[NUMPEDSTATS];
public:
	static void Initialise(void);
	static void LoadPedStats(void);
	static int  GetPedStatType(const char *name);
};

class CHandlingData
{
	// NOT reversed data, just read from file
	struct Transmission {
		char nDriveType;
		char nEngineType;
		char nNumberOfGears;
		uchar Flags;
		float fEngineAcceleration;
		float fMaxVelocity;
	};
	struct Data {
		int ident;
		float fMass;
		rw::V3d Dimensions;
		rw::V3d CenterOfMass;
		int nPercentSubmerged;
		float fTractionMultiplier;
		float fTractionLoss;
		float fTractionBias;
		Transmission trans;

		float fBrakeDeceleration;
		float fBrakeBias;
		bool bABS;
		float fSteeringLock;
		float fSuspensionForceLevel;
		float fSuspensionDampingLevel;
		float fSeatOffsetDistance;
		float fCollisionDamageMultiplier;
		int nMonetaryValue;

		float fSuspensionUpperLimit;
		float fSuspensionLowerLimit;
		float fSuspensionBias;
		uchar Flags;
		uchar FrontLights;
		uchar RearLights;
	};
	static Data data[NUMHANDLINGS];
public:
	static void Initialise(void);
	static void LoadHandlingData(void);
	static int  GetHandlingData(const char *ident);
};
