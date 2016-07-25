// temporary
typedef void unknown;
typedef float *RwMatrixTag;

class CBaseModelInfo
{
protected:
	// TODO?: make more things protected
	char       m_name[24];
	CColModel *m_colModel;
	C2dEffect *m_twodEffects;
	short      m_objectId;
public:
	ushort     m_refCount;
	short      m_txdSlot;
	uchar      m_type;	// ModelInfo type
	uchar      m_num2dEffects;
	bool       m_freeCol;

	CBaseModelInfo(int type);
	virtual ~CBaseModelInfo() {}
	virtual unknown Shutdown(void) {}
	virtual unknown DeleteRwObject(void) = 0;
	virtual unknown CreateInstance(void) = 0;
	virtual unknown CreateInstance(RwMatrixTag *) = 0;
	virtual unknown GetRwObject(void) = 0;

	char *GetName(void) { return m_name; }
	void SetName(const char *name) { strncpy(m_name, name, 24); }
	void SetColModel(CColModel *col, bool free = false){
		m_colModel = col; m_freeCol = free; }
	CColModel *GetColModel(void) { return m_colModel; }
	short GetObjectID(void) { return m_objectId; }
	void SetObjectID(short id) { m_objectId = id; }
	void AddRef(void);
	void SetTexDictionary(const char *name);
	void AddTexDictionaryRef(void);
	void Add2dEffect(C2dEffect *fx);
};

class CClumpModelInfo : public CBaseModelInfo
{
public:
	enum { ID = 4 };
	rw::Clump *m_clump;

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
	float m_drawDist;
	int   m_firstInstance;
	int   m_lastInstance;

	~CMloModelInfo() {}
};

class CPedModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 6 };
	int         m_animGroup;
	int         m_pedType;
	int         m_pedStats;
	int         m_carsCanDrive;
	CColModel  *m_hitColModel;
#ifdef SKINNED
	rw::Atomic *m_head;
	rw::Atomic *m_lhand;
	rw::Atomic *m_rhand;
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

	uchar   m_lastPrimaryColor;
	uchar   m_lastSecondaryColor;
	char    m_gameName[32];
	int     m_vehicleType;
	int     m_extraModelIndex;
	float   m_wheelScale;
	int     m_numDoors;
	int     m_handlingId;
	int     m_vehicleClass;
	int     m_lvl;
	rw::V3d m_dummyPos[10];
	int     m_compRules;
	float   m_bikeValue;
	rw::Material **m_primaryMaterials[26];
	rw::Material **m_secondaryMaterials[26];
	uchar   m_primaryColorIds[8];
	uchar   m_secondaryColorIds[8];
	uchar   m_numColors;
	uchar   m_unk; // unused?
	uchar   m_currentPrimaryColor;
	uchar   m_currentSecondaryColor;
	rw::Texture *m_envMap;
	rw::Atomic *m_comps[6];
	int     m_numComps;

	CVehicleModelInfo(void) : CClumpModelInfo(ID) {}
	~CVehicleModelInfo() {}
	unknown DeleteRwObject(void) {}
	unknown CreateInstance(void) {}
	void SetClump(rw::Clump *clump){
		CClumpModelInfo::SetClump(clump);
	}
};

// Unused
class CXtraCompsModelInfo : public CClumpModelInfo
{
public:
	enum { ID = 7 };
	int m_unk;

	~CXtraCompsModelInfo() {}
	unknown Shutdown(void) {}
	unknown CreateInstance(void) {}
	void SetClump(rw::Clump *clump){
		CClumpModelInfo::SetClump(clump);
	}
};

class CSimpleModelInfo : public CBaseModelInfo
{
public:
	enum { ID = 1 };
	// atomics[2] is often a pointer to the non-LOD modelinfo
	rw::Atomic *m_atomics[3];
	float m_lodDistances[3];
	uchar m_numAtomics;
	uchar m_alpha;
	uint  m_furthest      : 3; // 0: numAtomics-1 is furthest visible
	                         // 1: atomic 0 is furthest
	                         // 2: atomic 1 is furthest
	uint  m_normalCull    : 1;
	uint  m_unknownFlag   : 1;
	uint  m_isBigBuilding : 1;
	uint  m_noFade        : 1;
	uint  m_drawLast      : 1;
	uint  m_additive      : 1;
	uint  m_isSubway      : 1;
	uint  m_ignoreLight   : 1;
	uint  m_noZwrite      : 1;

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

	void SetNumAtomics(int n) { m_numAtomics = n; }
	CSimpleModelInfo *GetRelatedModel(void){
		return (CSimpleModelInfo*)m_atomics[2]; }
	void SetRelatedModel(CSimpleModelInfo *m){
		m_atomics[2] = (rw::Atomic*)m; }
};

class CTimeModelInfo : public CSimpleModelInfo
{
public:
	enum { ID = 3 };
	int m_timeOn;
	int m_timeOff;
	int m_otherTimeModelID;

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
