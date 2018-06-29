#ifndef _PHYSICAL_H_
#define _PHYSICAL_H_

enum
{
	PHYSICAL_MAX_COLLISIONRECORDS = 6
};

class CPtrNode;

class CPhysical : public CEntity
{
	// disable allocation
	static void *operator new(size_t) { assert(0); return nil; }
	static void operator delete(void*, size_t) { assert(0); }

public:
	// TODO

	uint32 m_nLastTimeCollided;
	CVector m_vecMoveSpeed;
	CVector m_vecTurnSpeed;
	CVector m_vecMoveFriction;
	CVector m_vecTurnFriction;
	CVector m_vecMoveSpeedAvg;
	CVector m_vecTurnSpeedAvg;
	float m_fMass;
	float m_fTurnMass;
	float m_fAirResistance;
	float m_fElasticity;
	CVector m_vecCentreOfMass;

	CEntryInfoList m_entryInfoList;
	CPtrNode *m_movingListNode;

	uint8 m_nStaticFrames;
	uint8 m_nCollisionRecords;
	uint8 m_unk2;
	CEntity *m_aCollisionRecords[PHYSICAL_MAX_COLLISIONRECORDS];

	float m_fDistanceTravelled;

	// damaged piece
	float m_fCollisionImpulse;
	CEntity *m_pCollidingEntity;
	CVector m_vecCollisionDirection;
	int16 m_nCollisionPieceType;

	uint8 m_phy_flagA1 : 1;
	uint8 bAffectedByGravity : 1;
	uint8 bInfiniteMass : 1;
	uint8 m_phy_flagA8 : 1;
	uint8 m_phy_flagA10 : 1;
	uint8 m_phy_flagA20 : 1;
	uint8 m_phy_flagA40 : 1;
	uint8 m_phy_flagA80 : 1;

	uint8 m_phy_flagB1 : 1;
	uint8 m_phy_flagB2 : 1;
	uint8 m_phy_flagB4 : 1;
	uint8 m_phy_flagB8 : 1;
	uint8 m_phy_flagB10 : 1;
	uint8 m_phy_flagB20 : 1;
	uint8 m_phy_flagB40 : 1;
	uint8 m_phy_flagB80 : 1;

public:
	CPhysical(void);
	// from CPlaceable
	~CPhysical(void);
	// from CEntity
	void Add(void);
	void Remove(void);
	CRect GetBoundRect(void);
	void ProcessControl(void);
	void ProcessCollision(void);
	void ProcessShift(void);

	// get speed of point p relative to entity center
	CVector GetSpeed(CVector &p);
	CVector GetSpeed(void) { return GetSpeed(CVector(0.0f, 0.0f, 0.0f)); }
	void UnsetIsInSafePosition(void) {
		m_vecMoveSpeed *= -1.0f;
		m_vecTurnSpeed *= -1.0f;
		ApplyTurnSpeed();
		ApplyMoveSpeed();
		m_vecMoveSpeed *= -1.0f;
		m_vecTurnSpeed *= -1.0f;
		bIsInSafePosition = false;	
	}

	void ApplyMoveSpeed(void);
	void ApplyTurnSpeed(void);
	void ApplyMoveForce(float x, float y, float z);
	void ApplyMoveForce(const CVector &v) { ApplyMoveForce(v.x, v.y, v.z); }
	// v(x,y,z) is direction of force, p(x,y,z) is point relative to model center where force is applied
	void ApplyTurnForce(float vx, float vy, float vz, float px, float py, float pz);
	// v is direction of force, p is point relative to model center where force is applied
	void ApplyTurnForce(const CVector &v, const CVector &p) { ApplyTurnForce(v.x, v.y, v.z, p.x, p.y, p.z); }
	void ApplyFrictionMoveForce(float x, float y, float z);
	void ApplyFrictionMoveForce(const CVector &v) { ApplyFrictionMoveForce(v.x, v.y, v.z); }
	void ApplyFrictionTurnForce(float vx, float vy, float vz, float px, float py, float pz);
	void ApplyFrictionTurnForce(const CVector &v, const CVector &p) { ApplyFrictionTurnForce(v.x, v.y, v.z, p.x, p.y, p.z); }
	void ApplySpringCollision(float f1, CVector &v, CVector &p, float f2, float f3);
	void ApplyGravity(void);
	void ApplyFriction(void);
	void ApplyAirResistance(void);
	bool ApplyCollision(CPhysical *phys, CColPoint &colpoint, float &f1, float &f2);
	bool ApplyFriction(CPhysical *B, float adhesiveLimit, CColPoint &colpoint);

	void RemoveAndAdd(void);
	void AddToMovingList(void);
	void RemoveFromMovingList(void);

	void SetDamagedPieceRecord(uint16 pieceType, float colImpulse, CEntity *colEntity, CVector colDirection); 
	void AddCollisionRecord(CEntity *ent);
	void AddCollisionRecord_Treadable(CEntity *ent);
	bool GetHasCollidedWith(CEntity *ent);
	bool ProcessCollisionSectorList(CPtrList *lists);
	bool ProcessCollisionSectorList_SimpleCar(CSector *sector);
	bool ProcessShiftSectorList(CPtrList *lists);
	bool CheckCollision(void);
	bool CheckCollision_SimpleCar(void);

	virtual int32 ProcessEntityCollision(CEntity *ent, CColPoint *colpoints);
};

#endif
