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
	CVector m_vecCentreOfMass;

	CEntryInfoList m_entryInfoList;
	CPtrNode *m_movingListNode;

	uint8 m_nStaticFrames;
	uint8 m_nCollisionRecords;
	uint8 m_unk2;
	CEntity *m_aCollisionRecords[PHYSICAL_MAX_COLLISIONRECORDS];

	float m_fDistanceTravelled;
/*
  float fCollisionPower;
  float pPhysColliding;
  CVector vecCollisionPower;
  __int16 wComponentCol;
*/

	uint8 m_phy_flagA1 : 1;
	uint8 bAffectedByGravity : 1;
	uint8 m_phy_flagA4 : 1;
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

	CVector GetSpeed(CVector &v);

	void ApplyMoveSpeed(void);
	void ApplyTurnSpeed(void);
	void ApplyMoveForce(float x, float y, float z);
	void ApplyMoveForce(const CVector &v) { ApplyMoveForce(v.x, v.y, v.z); }
	void ApplyTurnForce(float x1, float y1, float z1, float x2, float y2, float z2);
	void ApplyTurnForce(const CVector &v1, const CVector &v2) { ApplyTurnForce(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z); }
	void ApplyGravity(void);
	void ApplyFriction(void);
	void ApplyAirResistance(void);

	void RemoveAndAdd(void);
	void AddToMovingList(void);
	void RemoveFromMovingList(void);

	void AddCollisionRecord(CEntity *ent);
	void AddCollisionRecord_Treadable(CEntity *ent);
	bool GetHasCollidedWith(CEntity *ent);
	bool ProcessCollisionSectorList(CPtrList *lists);
	bool ProcessCollisionSectorList_SimpleCar(CPtrList *lists);
	bool CheckCollision(void);
	bool CheckCollision_SimpleCar(void);

	virtual int32 ProcessEntityCollision(CEntity *ent, CColPoint *colpoints);
};

#endif
