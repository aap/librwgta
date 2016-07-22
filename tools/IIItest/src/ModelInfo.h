// temporary
typedef void unknown;
typedef float *RwMatrixTag;

class CBaseModelInfo
{
public:
	char name[24];
	CColModel *colModel;
	C2dEffect *twodEffects;
	short id;	// ??
	ushort refCount;
	short txdSlot;
	uchar type;	// ModelInfo type
	uchar num2dEffects;
	bool freeCol;

	CBaseModelInfo(int type);
	virtual ~CBaseModelInfo() {}
	virtual unknown Shutdown(void) {}
	virtual unknown DeleteRwObject(void) = 0;
	virtual unknown CreateInstance(void) = 0;
	virtual unknown CreateInstance(RwMatrixTag *) = 0;
	virtual unknown GetRwObject(void) = 0;

	void AddRef(void);
	void SetTexDictionary(const char *name);
	void AddTexDictionaryRef(void);
	void Add2dEffect(C2dEffect *fx);
};

class CClumpModelInfo : public CBaseModelInfo
{
public:
	enum { ID = 4 };
	rw::Clump *clump;

	CClumpModelInfo(void) : CBaseModelInfo(ID) {}
	CClumpModelInfo(int id) : CBaseModelInfo(id) {}
	~CClumpModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	unknown CreateInstance(RwMatrixTag *) {}
	unknown GetRwObject(void) {}

	virtual void SetClump(rw::Clump *);
};


// Unused
class CMloModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 2 };
	float drawDist;
	int firstInstance;
	int lastInstance;

	~CMloModelInfo() {}
};

class CPedModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 6 };
	int animGroup;
	int pedType;
	int pedStats;
	int carsCanDrive;
	CColModel *hitColModel;
#ifdef SKINNED
	RpAtomic *head;
	RpAtomic *lhand;
	RpAtomic *rhand;
#endif

	CPedModelInfo(void) : CClumpModelInfo(ID) {}
	~CPedModelInfo() {}
	unknown DeleteRwObject(void) {}
	void SetClump(rw::Clump *clump){
		CClumpModelInfo::SetClump(clump);
	}
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

	CVehicleModelInfo(void) : CClumpModelInfo(ID) {}
	~CVehicleModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	void SetClump(rw::Clump *){
		CClumpModelInfo::SetClump(clump);
	}
};

// Unused
class CXtraCompsModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 7 };
	int unk;

	~CXtraCompsModelInfo() {}
	unknown Shutdown(void) {}
	unknown CreateInstance(void) {}
	void SetClump(rw::Clump *){
		CClumpModelInfo::SetClump(clump);
	}
};

class CSimpleModelInfo : public CBaseModelInfo
{
public:
	enum { ID = 1 };
	// atomics[2] is often a pointer to the non-LOD modelinfo
	rw::Atomic *atomics[3];
	float lodDistances[3];
	uchar numAtomics;
	uchar alpha;
	uint  furthest      : 3; // 0: numAtomics-1 is furthest visible
	                         // 1: atomic 0 is furthest
	                         // 2: atomic 1 is furthest
	uint  normalCull    : 1;
	uint  unknownFlag   : 1;
	uint  isBigBuilding : 1;
	uint  noFade        : 1;
	uint  drawLast      : 1;
	uint  additive      : 1;
	uint  isSubway      : 1;
	uint  ignoreLight   : 1;
	uint  noZwrite      : 1;

	CSimpleModelInfo(void) : CBaseModelInfo(ID) {}
	CSimpleModelInfo(int id) : CBaseModelInfo(id) {}
	~CSimpleModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	unknown CreateInstance(RwMatrixTag *) {}
	unknown GetRwObject(void) {}

	void Init(void);
	void SetAtomic(int n, rw::Atomic *atomic);
	void SetLodDistances(float *dist);
	float GetLargestLodDistance(void);
	void FindRelatedModel(void);
	void SetupBigBuilding(void);

	CSimpleModelInfo *GetRelatedModel(void){
		return (CSimpleModelInfo*)this->atomics[2]; }
	void SetRelatedModel(CSimpleModelInfo *m){
		this->atomics[2] = (rw::Atomic*)m; }
};

class CTimeModelInfo : public CSimpleModelInfo
{
public:
	enum { ID = 3 };
	int timeOn;
	int timeOff;
	int otherTimeModelID;

	CTimeModelInfo(void) : CSimpleModelInfo(ID) {}
	~CTimeModelInfo() {}

	CTimeModelInfo *FindOtherTimeModel(void);
};


class CModelInfo
{
public:
	static void Initialise(void);

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
	static CBaseModelInfo *GetModelInfo(int id){
		return ms_modelInfoPtrs[id];
	}
};
