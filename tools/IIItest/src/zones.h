//
// Zones
//

enum eZoneType
{
	ZONE_AUDIO,
	ZONE_TYPE1,
	ZONE_TYPE2,
	ZONE_MAPZONE,
};

enum eLevelName
{
	LEVEL_NONE = 0,
	LEVEL_INDUSTRIAL,
	LEVEL_COMMERCIAL,
	LEVEL_SUBURBAN
};

class CZone
{
public:
	char       name[8];
	float      minx;
	float      miny;
	float      minz;
	float      maxx;
	float      maxy;
	float      maxz;
	eZoneType  type;
	eLevelName level;
	short      zoneinfoNight;
	short      zoneinfoDay;
	CZone     *child;
	CZone     *parent;
	CZone     *next;
};

class CZoneInfo
{
public:
	ushort carDensity;

	ushort carThreshold[6];
	ushort copThreshold;
	ushort gangThreshold[8];
	ushort car6Threshold;

	ushort pedDensity;
	ushort copDensity;
	ushort gangDensity[9]; // really 9?
	ushort pedGroup;
};

class CTheZones
{
	static eLevelName m_CurrLevel;
	static CZone *m_pPlayersZone;
	// FindIndex

	static ushort NumberOfAudioZones;
	static short AudioZoneArray[NUMAUDIOZONES];
	static ushort TotalNumberOfMapZones;
	static ushort TotalNumberOfZones;
	static CZone ZoneArray[NUMZONES];
	static CZone MapZoneArray[NUMMAPZONES];
	static ushort TotalNumberOfZoneInfos;
	static CZoneInfo ZoneInfoArray[2*NUMZONES];
public:
	static void Init(void);
	static void PostZoneCreation(void);
	static void CreateZone(char *name, eZoneType type,
	                       float minx, float miny, float minz,
	                       float maxx, float maxy, float maxz,
	                       eLevelName level);
	static void CreateMapZone(char *name, eZoneType type,
	                          float minx, float miny, float minz,
	                          float maxx, float maxy, float maxz,
	                          eLevelName level);
	static CZone *GetZone(ushort i) { return &ZoneArray[i]; }
	static void InsertZoneIntoZoneHierarchy(CZone *zone);
	static bool InsertZoneIntoZoneHierRecursive(CZone *z1, CZone *z2);
	static bool ZoneIsEntirelyContainedWithinOtherZone(CZone *z1, CZone *z2);
	static bool PointLiesWithinZone(CVector const *v, CZone *zone);
	static eLevelName GetLevelFromPosition(CVector const *v);
	static void AddZoneToAudioZoneArray(CZone *zone);
	static void InitialiseAudioZoneArray(void);
};

//
// Cull zones
//

class CCullZone
{
public:
	CVector position;
	float minx;
	float maxx;
	float miny;
	float maxy;
	float minz;
	float maxz;
	int unk1;
	short unk2;
	short unk3;
	short unk4;
	int unk5;
};

struct CAttributeZone
{
	float minx;
	float maxx;
	float miny;
	float maxy;
	float minz;
	float maxz;
	short attributes;
	short wantedLevel;
};

class CCullZones
{
	static int  CurrentWantedLevelDrop_Player;
	static int  CurrentFlags_Camera;
	static int  OldCullZone;
	static int  EntityIndicesUsed;
	static int  CurrentFlags_Player;
	static bool bCurrentSubwayIsInvisible;

	static int       NumCullZones;
	static CCullZone aZones[NUMCULLZONES];
	static int       NumAttributeZones;
	static CAttributeZone aAttributeZones[NUMATTRIBZONES];
	static ushort    aIndices[NUMZONEINDICES];
	static short     aPointersToBigBuildingsForBuildings[NUMBUILDINGS];
	static short     aPointersToBigBuildingsForTreadables[NUMTREADABLES];
public:
	static void Init(void);
	static void AddCullZone(CVector const &position,
	                        float minx, float maxx,
	                        float miny, float maxy,
	                        float minz, float maxz,
	                        ushort flag, short wantedLevel);
};
