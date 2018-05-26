#ifndef _PHYSICAL_H_
#define _PHYSICAL_H_

class CPhysical : public CEntity
{
	// disable allocation
	static void *operator new(size_t) { assert(0); return nil; }
	static void operator delete(void*, size_t) { assert(0); }

public:
	CVector m_vecMoveSpeed;
	CVector m_vecTurnSpeed;
	CVector m_vecMoveFriction;
	CVector m_vecTurnFriction;
	float m_fMass;
	float m_fTurnMass;
	float m_fAirResistance;
	CVector m_vecCentreOfMass;

	CPtrNode *m_movingListNode;

	uint8 bAffectedByGravity : 1;

public:
	CPhysical(void);
	// from CPlaceable
	~CPhysical(void);

	void ApplyMoveSpeed(void);
	void ApplyTurnSpeed(void);
	void ApplyMoveForce(float x, float y, float z);
	void ApplyMoveForce(const CVector &v) { ApplyMoveForce(v.x, v.y, v.z); }
	void ApplyTurnForce(float x1, float y1, float z1, float x2, float y2, float z2);
	void ApplyTurnForce(const CVector &v1, const CVector &v2) { ApplyTurnForce(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z); }
	void ApplyGravity(void);
	void ApplyFriction(void);
	void ApplyAirResistance(void);

	void AddToMovingList(void);
};

#endif
