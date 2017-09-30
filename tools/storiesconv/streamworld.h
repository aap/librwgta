


#ifdef VCS
#define NUMSECTORS 37
#endif

#ifdef LCS
#define NUMSECTORS 47
#endif

struct Resource
{
	void *res;
	void *dmaChain;	// at runtime
};

struct LevelEntry
{
	sChunkHeader *header;
	int32 num;	// number of something in this sector
};

struct TimeInfo
{
	uint8 timeOn;
	uint8 timeOff;
	int16 id;
};

struct Interior
{
	uint8 listOffset;	// index into sector list
	uint8 listIndex;	// offset inside entry list (LevelEntry::num is subtracted from this)
	uint8 c;
	uint8 d;
	int16 streamWorldId;
};

struct sLevelChunk
{
	Resource *resourceTables;
	LevelEntry levelEntries[NUMSECTORS];
	int32 numResources;
	CVector positions[32];	// there's some data there, but the game treats them as CVectors

	// not sure about the exact layout in VCS. it seems to have like 5 tables here

	// this looks wrong for all games
	int32 numTimeObjects;
	TimeInfo *timeObjects;

	int32 numInteriors;
	Interior *interiors;
#ifdef LCS
	int32 numRadarSection;
	sChunkHeader *radarSections;
#endif
};
#ifdef VCS
static_assert(offsetof(sLevelChunk, numResources)+0x20 == 0x14c, "sLevelChunk: error");
static_assert(offsetof(sLevelChunk, numTimeObjects)+0x20 == 0x2d0, "sLevelChunk: error");
#endif

#ifdef LCS
static_assert(offsetof(sLevelChunk, numResources)+0x20 == 0x19c, "sLevelChunk: error");
static_assert(offsetof(sLevelChunk, numTimeObjects)+0x20 == 0x320, "sLevelChunk: error");
#endif
