// temporary
typedef void unknown;
typedef float *RwMatrixTag;

class CBaseModelInfo
{
public:
	char name[24];
	CColModel *colModel;
	C2dEffect *twodEffects;
	int16 id;	// ??
	uint16 refCount;
	int16 txdSlot;
	uint8 type;	// ModelInfo type
	uint8 num2dEffects;
	bool freeCol;

	void ctor(int type);
	virtual ~CBaseModelInfo() {}
	virtual unknown Shutdown(void) {}
	virtual unknown DeleteRwObject(void) = 0;
	virtual unknown CreateInstance(void) = 0;
	virtual unknown CreateInstance(RwMatrixTag *) = 0;
	virtual unknown GetRwObject(void) = 0;

	void SetTexDictionary(char *name);
	void Add2dEffect(C2dEffect *fx);
};

class CClumpModelInfo : public CBaseModelInfo
{
public:
	enum { ID = 4 };
	rw::Clump *clump;

	~CClumpModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	unknown CreateInstance(RwMatrixTag *) {}
	unknown GetRwObject(void) {}

	virtual unknown SetClump(rw::Clump *) {}
};


// Unused
class CMloModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 2 };
	float unknown;           // ??
	int32 instStoreSize1;    // ??
	int32 instStoreSize2;    // ??

	~CMloModelInfo() {}
};

class CPedModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 6 };
	int32 animGroup;
	int32 pedType;
	int32 pedStats;
	int32 carsCanDrive;
	CColModel *hitColModel;
#ifdef SKINNED
	RpAtomic *head;
	RpAtomic *lhand;
	RpAtomic *rhand;
#endif

	~CPedModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown SetClump(rw::Clump *) {}
};

class CVehicleModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 5 };
	enum Type { Car, Boat, Train, Heli, Plane, Bike };
	enum Class { Poorfamily, Richfamily, Executive, Worker, Special, Big, Taxi };

	uchar lastPrimaryColor;
	uchar lastSecondaryColor;
	char gameName[32];
	int vehicleType;
	int extraModelIndex;
	float wheelScale;
	int numDoors;
	int handlingId;
	int vehicleClass;
	int lvl;
	rw::V3d dummyPos[10];
	int compRules;
	float bikeValue;
	rw::Material **primaryMaterials[26];
	rw::Material **secondaryMaterials[26];
	uchar primaryColorIds[8];
	uchar secondaryColorIds[8];
	uchar numColors;
	uchar unk; // unused?
	uchar currentPrimaryColor;
	uchar currentSecondaryColor;
	rw::Texture *envMap;
	rw::Atomic *comps[6];
	int numComps;

	~CVehicleModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	unknown SetClump(rw::Clump *) {}
};

// Unused
class CXtraCompsModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 7 };
	int32 unk;

	~CXtraCompsModelInfo() {}
	unknown Shutdown(void) {}
	unknown CreateInstance(void) {}
	unknown SetClump(rw::Clump *) {}
};

class CSimpleModelInfo : public CBaseModelInfo
{
public:
	enum { ID = 1 };
	rw::Atomic *atomics[3];
	float lodDistances[3];
	uint8 numAtomics;
	uint8 alpha; // ?
	uint16 flags;

	~CSimpleModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	unknown CreateInstance(RwMatrixTag *) {}
	unknown GetRwObject(void) {}
};

class CTimeModelInfo : public CSimpleModelInfo
{
public:
	enum { ID = 3 };
	int32 timeOn;
	int32 timeOff;
	int32 otherTimeModelID;

	~CTimeModelInfo() {}
};


class CModelInfo
{
public:
	static CBaseModelInfo *ms_modelInfoPtrs[MODELINFOSIZE];
	static CStore<CSimpleModelInfo, SIMPLEMODELSIZE> ms_simpleModelStore;
	static CStore<CTimeModelInfo, TIMEMODELSIZE> ms_timeModelStore;
	static CStore<CClumpModelInfo, CLUMPMODELSIZE> ms_clumpModelStore;
	static CStore<CPedModelInfo, PEDMODELSIZE> ms_pedModelStore;
	static CStore<CVehicleModelInfo, VEHICLEMODELSIZE> ms_vehicleModelStore;
	static CStore<C2dEffect, TWODFXSIZE> ms_2dEffectStore;

	static CSimpleModelInfo *AddSimpleModel(int id);
	static CTimeModelInfo *AddTimeModel(int id);
	static CClumpModelInfo *AddClumpModel(int id);
	static CPedModelInfo *AddPedModel(int id);
	static CVehicleModelInfo *AddVehicleModel(int id);

	static CBaseModelInfo *GetModelInfo(char *name, int *id);
};
