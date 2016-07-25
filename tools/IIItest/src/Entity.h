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

protected:
	rw::Clump *m_rwObject;
	uint m_type : 3;
	uint m_status : 5;

	// flagsA
	uint m_doCollision : 1;

	short m_modelIndex;
public:
	eLevelName m_level;	// short

	CEntity(void);

	// from CPlaceable
	~CEntity(void);

	virtual void SetModelIndexNoCreate(uint i) { m_modelIndex = i; }
};
