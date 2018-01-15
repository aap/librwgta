


#ifdef VCS
#define NUMSECTORSX 32
#define NUMSECTORSY 36
#endif

#ifdef LCS
#define NUMSECTORSX 40
#define NUMSECTORSY 46
#endif

// some type names from LCS:
//	sDMAModel	  // lucid
//	sDMAMaterial	  // lucid
//	sDMAMesh	  // lucid
//	cWorldGeom
//	cWorldGeom::sModel
//	cWorldGeom::sModelInstance
//	cWorldGeom::cSector
//	sLevelSectorDirectory
//	sStripData
//	sDynamic
//	sLevelSwap
//	sInteriorSwap
//	sBuildingSwapInfo

struct sBuildingGeometry	// leeds name
{
	uint16 numMeshes;
	uint16 size;
	// array of numMeshes sClippableBuildingMeshes
	// dma data
};

struct sClippableBuildingMesh	// leeds name
{
#ifdef LCS
	uint16  texID;		// index into resource table
	uint16  packetSize;	// upper bit unknown
#else
	int32   packetSize;
	uint16  texID;		// index into resource table
#endif
	float16 uvScale[2];	// half precision float
	uint16  unk1;
	float16 min[3];		// bounding box
	float16 max[3];

	int GetPacketSize(void){
#ifdef LCS
		return packetSize&0x7FFF;
#else
		return packetSize>>1;
#endif
	}
};


struct Resource
{
	union {
		RslRaster         *raster;
		sBuildingGeometry *geometry;
		uint8             *raw;
	};
	void *dmaChain;	// at runtime
#ifdef VCS
	int id;
#endif
};

// One row of streamable sectors
struct SectorRow
{
	sChunkHeader *header;	// an array
	int32 startOff;	// starting offset of this row
			// (startOff sectors in the world will be considered empty)
};

struct TriggerInfo
{
	// There are three cases:
	// timeOff == 0xFF -> unconditional swap
	// timeOff &  0x80 -> timed object with timeOff&0x7F
	// otherwise -> conditioned by cWorldStream data
	uint8 timeOff;
	uint8 timeOn;
	int16 id;	// instance ID for level, sector ID for sector
};

struct AreaResource
{
	int16 id1;
	int16 id2;
	void *data;
};

struct Area
{
	int32 numResources;
	AreaResource *resources;
};

struct AreaInfo
{
	int16 a, b;	// sector coords or something?
	uint32 fileOffset;	// into image, start of sChunkHeader
	uint32 fileSize;
	int32 numResources;
};

struct sInteriorSwap
{
	uint8 secx, secy;	// position of this sector
	// not quite sure what these two do exactly
	uint8 buildingIndex;	// index into the building swap array
	uint8 buildingSwap;	// some kind of id?
	int16 sectorId;		// sector index
};

struct OverlayResource
{
	int32 id;
	union {
		RslRaster         *raster;
		sBuildingGeometry *geometry;
		uint8             *raw;
	};
};

struct sGeomInstance
{
	uint16  id;
	uint16  resId;
	float16 bound[4];
	int32   pad;
	RslMatrix matrix;

	int16 GetId(void) { return id & 0x7FFF; }
	bool IsLOD(void) {
#ifdef LCS
		return !(id & 0x8000);
#else
		return !!(id & 0x8000);
#endif
	}
};
static_assert(sizeof(sGeomInstance) == 0x50, "Placement: error");

struct Sector
{
	OverlayResource *resources;
	uint16           numResources;
	uint16           unk1;
	sGeomInstance   *sectionA;
	sGeomInstance   *sectionB;
	sGeomInstance   *sectionC;
	sGeomInstance   *sectionD;
	sGeomInstance   *sectionE;
	sGeomInstance   *sectionF;
	sGeomInstance   *sectionG;
#ifdef VCS
	sGeomInstance   *sectionH;
#endif
	sGeomInstance   *sectionEnd;
	int16            numTriggeredObjects;
	uint16           unk3;
	TriggerInfo     *triggeredObjects;
};

struct sLevelChunk	// leeds name
{
	Resource *resourceTable;
	SectorRow sectorRows[NUMSECTORSY];
	SectorRow sectorEnd;
	int32 numResources;
	// Positions for the first 32 building instances...what's so special about them?
	CVector positions[32];

#ifdef VCS
	int32 numX;
	void *xs;	// 28 bytess
#endif
	int32 numTriggeredObjects;
	TriggerInfo *triggeredObjects;
#ifdef LCS
	// 2dfx according to gtamodding.ru
	int32 numY;
	void *ys;	// 48 bytes
#endif
	int32 numInteriors;
	sInteriorSwap *interiors;

#ifdef LCS
	int32 numRadarSections;
	sChunkHeader *radarSections;
#endif

#ifdef VCS
	// 2dfx again?
	int32 numY;
	void *ys;	// 48 bytes

	int32 numAreas;
	AreaInfo *areas;
#endif
};
//#ifdef VCS
//static_assert(offsetof(sLevelChunk, numResources)+0x20 == 0x14c, "sLevelChunk: error");
////static_assert(offsetof(sLevelChunk, numTimeObjects)+0x20 == 0x2d0, "sLevelChunk: error");
//#endif
//
//#ifdef LCS
//static_assert(offsetof(sLevelChunk, numResources)+0x20 == 0x19c, "sLevelChunk: error");
////static_assert(offsetof(sLevelChunk, numTimeObjects)+0x20 == 0x320, "sLevelChunk: error");
//#endif
