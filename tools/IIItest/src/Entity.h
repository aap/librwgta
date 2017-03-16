enum eEntityType
{
	ENTITY_TYPE_NOTHING = 0,
	ENTITY_TYPE_BUILDING,
	ENTITY_TYPE_VEHICLE,
	ENTITY_TYPE_PED,
	ENTITY_TYPE_OBJECT,
	ENTITY_TYPE_DUMMY,
	ENTITY_TYPE_6,
	ENTITY_TYPE_7,
};

enum eEntityStatus
{
	ENTITY_STATUS_0 = 0,
	ENTITY_STATUS_1 = 1,
	ENTITY_STATUS_2 = 2,
	ENTITY_STATUS_3 = 3,
	ENTITY_STATUS_4 = 4,
};

class CEntity : public CPlaceable
{
	// disable allocation
	static void *operator new(size_t) { assert(0); }
	static void operator delete(void*, size_t) { assert(0); }

public:
	rw::Object *m_rwObject;
	uint m_type : 3;
	uint m_status : 5;

	// flagsA
	uint m_doCollision : 1;
	uint m_flagA2 : 1;
	uint m_flagA4 : 1;
	uint m_flagA8 : 1;
	uint m_flagA10 : 1;
	uint m_flagA20 : 1;
	uint m_flagA40 : 1;
	uint m_flagA80 : 1;

	// flagsB
	uint m_flagB1 : 1;
	uint m_flagB2 : 1;
	uint m_isVisible : 1;
	uint m_flagB8 : 1;
	uint m_flagB10 : 1;
	uint m_flagB20 : 1;
	uint m_isBigBuilding : 1;
	// VC inserts one more flag here: if drawdist <= 2000
	uint m_drawDamaged : 1;

	// flagsC
	uint m_flagC1 : 1;
	uint m_flagC2 : 1;
	uint m_flagC4 : 1;
	uint m_flagC8 : 1;
	uint m_flagC10 : 1;
	uint m_flagC20 : 1;
	uint m_flagC40 : 1;
	uint m_flagC80 : 1;

	// flagsD
	uint m_flagD1 : 1;
	uint m_flagD2 : 1;
	uint m_isBeingRendered : 1;
	uint m_flagD8 : 1;
	uint m_flagD10 : 1;
	uint m_flagD20 : 1;
	uint m_flagD40 : 1;
	uint m_flagD80 : 1;

	// flagsE
	uint m_flagE1 : 1;
	uint m_flagE2 : 1;
	uint m_flagE4 : 1;
	uint m_flagE8 : 1;
	uint m_flagE10 : 1;
	uint m_flagE20 : 1;
	uint m_flagE40 : 1;
	uint m_flagE80 : 1;

	short m_scanCode;
	short m_modelIndex;
	eLevelName m_level;	// short

	CEntity(void);
	void SetupBigBuilding(void);
	bool IsVisible(void);

	// from CPlaceable
	~CEntity(void);

	virtual void Add(void);
	// Remove
	virtual void SetModelIndexNoCreate(uint i) { m_modelIndex = i; }
	// SetModelIndex
	virtual void CreateRwObject(void);
	virtual void DeleteRwObject(void);
	virtual CRect GetBoundRect(void);
	// ProcessControl
	// ProcessCollision
	// ProcessShift
	// Teleport
	// PreRender
	virtual void Render(void);
	virtual void SetupLighting(void);
	virtual void RemoveLighting(void);
	// FlagToDestroyWhenNextProcessed
};
