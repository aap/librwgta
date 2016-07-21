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

#include "input.h"

using rw::uint8;
using rw::int8;
using rw::uint16;
using rw::int16;
using rw::uint32;
using rw::int32;
typedef unsigned int uint;
typedef unsigned char uchar;

void debug(const char *fmt, ...);

int init(void);
void shutdown(void);
void update(double t);
void display(void);
#ifdef RW_GL3
void pollinput(GLFWwindow*);
void keypress(GLFWwindow*, int key, int scancode, int action, int mods);
#endif

#include "templates.h"
#include "config.h"

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

#include "Animation.h"
#include "ModelInfo.h"
#include "TimeCycle.h"
#include "VisibilityPlugins.h"

char *getPath(const char *path);
FILE *fopen_ci(const char *path, const char *mode);
char *skipWhite(char *s);

struct StrAssoc {
	const char *key;
	int val;

	static int get(StrAssoc *desc, const char *key);
};

struct DatDesc {
	char name[5];
	void (*handler)(char *line);

	static void *get(DatDesc *desc, const char *name);
};

class CGame
{
public:
	static int currLevel;
	static void Initialise(void);
	static void InitialiseRW(void);
	static void InitialiseAfterRW(void);
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

class CPathFind
{
public:
	static void AllocatePathFindInfoMem(int n);
	static void StoreNodeInfoCar(short id, short i, signed char type, signed char next,
	                             short x, short y, short z,
	                             short width, signed char left, signed char right);
	static void StoreNodeInfoPed(short id, short i, signed char type, signed char next,
	                             short x, short y, short z,
	                             short width, bool crossing);
};

class CTxdStore
{
	struct TxdDef {
		rw::TexDictionary *texDict;
		int refCount;
		char name[20];
	};
	static TxdDef *entries;
	static uchar *flags;
	static int capacity;
	static int allocPtr;

	static rw::TexDictionary *ms_pStoredTxd;
public:
	static void Initialize(void);
	static int AddTxdSlot(const char *name);
	static int FindTxdSlot(const char *name);
	static char *GetTxdName(int slot);
	static void PushCurrentTxd(void);
	static void PopCurrentTxd(void);
	static void SetCurrentTxd(int slot);
	static void Create(int slot);
	static void AddRef(int slot);
	static void RemoveRefWithoutDelete(int slot);
	static bool LoadTxd(int slot, rw::Stream *stream);
	static bool LoadTxd(int slot, const char *filename);

	static TxdDef *getDef(int slot);
	static bool isTxdLoaded(int slot);
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

class CdStream
{
public:
	static int numImages;
	static char imageNames[NUMCDIMAGES][64];
	static FILE *images[NUMCDIMAGES];

	static void init();
	static void addImage(const char *name);
	static void removeImages(void);
	static void read(char *buf, uint pos, uint size);
};

class CDirectory
{
public:
	struct DirectoryInfo {
		uint32 position;
		uint32 size;
		char name[24];
	};
	DirectoryInfo *m_entries;
	int m_maxEntries;
	int m_numEntries;

	CDirectory(int size);
	void AddItem(DirectoryInfo *dirinfo);
};

class CStreamingInfo
{
public:
	CStreamingInfo *m_next;
	CStreamingInfo *m_prev;
	uchar m_loadState;
	uchar m_flags;	// 8 - priority request, 10 - subway, don't fade?

	short m_nextID;
	uint  m_position;
	uint  m_size;

	bool GetCdPosnAndSize(uint *pos, uint *size);
	void SetCdPosnAndSize(uint pos, uint size);
	void AddToList(CStreamingInfo *link);
	void RemoveFromList(void);
};

class CStreaming
{
public:
	enum {
		MODELOFFSET = 0,
		TXDOFFSET = MODELOFFSET+MODELINFOSIZE,
		NUMSTREAMINFO = TXDOFFSET+TXDSTORESIZE
	};
	static CStreamingInfo ms_aInfoForModel[NUMSTREAMINFO];
	static CDirectory *ms_pExtraObjectsDir;
	static CStreamingInfo ms_startLoadedList, ms_endLoadedList;
	static CStreamingInfo ms_startRequestedList, ms_endRequestedList;
	static int ms_streamingBufferSize;
	static char *ms_pStreamingBuffer;

	static void Init(void);
	static void LoadCdDirectory(void);
	static void LoadCdDirectory(const char *dirname, int n);
	static void ConvertBufferToObject(char *buffer, int id);
	static void RequestModel(int id, int flags);

	static void LoadAllRequestedModels(void);
	static void dumpRequestList(void);
	static int getNextFile(void);

	static CStreamingInfo *Model(int n) { return &ms_aInfoForModel[MODELOFFSET+n]; };
	static CStreamingInfo *Txd(int n) { return &ms_aInfoForModel[TXDOFFSET+n]; };
};
