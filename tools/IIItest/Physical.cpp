#include "III.h"

CPhysical::CPhysical(void)
{
	m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecCentreOfMass = CVector(0.0f, 0.0f, 0.0f);

	bUsesCollision = 1;
	bAffectedByGravity = 1;

	m_movingListNode = nil;
}

CPhysical::~CPhysical(void)
{
}

void
CPhysical::ApplyMoveSpeed(void)
{
	*m_matrix.GetPosition() += m_vecMoveSpeed * CTimer::ms_fTimeStep;
}

void
CPhysical::ApplyTurnSpeed(void)
{
	CVector turnvec = m_vecTurnSpeed*CTimer::ms_fTimeStep;
	*m_matrix.GetLeft() += CrossProduct(turnvec, *m_matrix.GetLeft());
	*m_matrix.GetUp() += CrossProduct(turnvec, *m_matrix.GetUp());
	*m_matrix.GetAt() += CrossProduct(turnvec, *m_matrix.GetAt());
}

void
CPhysical::ApplyMoveForce(float x, float y, float z)
{
	m_vecMoveSpeed += CVector(x, y, z)*(1.0f/m_fMass);
}

void
CPhysical::ApplyTurnForce(float x1, float y1, float z1, float x2, float y2, float z2)
{
	CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
	CVector turnf = CrossProduct(CVector(x2, y2, z2)-com, CVector(x1, y1, z1));
	m_vecTurnSpeed += turnf*(1.0f/m_fTurnMass);
}

void
CPhysical::ApplyGravity(void)
{
	if(bAffectedByGravity)
		m_vecMoveSpeed.z -= 0.008f * CTimer::ms_fTimeStep;
}

void
CPhysical::ApplyFriction(void)
{
	m_vecMoveSpeed += m_vecMoveFriction;
	m_vecTurnSpeed += m_vecTurnFriction;
	m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
}

void
CPhysical::ApplyAirResistance(void)
{
	if(m_fAirResistance > 0.1f){
		float f = powf(m_fAirResistance, CTimer::ms_fTimeStep);
		m_vecMoveSpeed *= f;
		m_vecTurnSpeed *= f;
	}else{
		float f = powf(1.0f/(m_fAirResistance*0.5f*m_vecMoveSpeed.MagnitudeSqr() + 1.0f), CTimer::ms_fTimeStep);
		m_vecMoveSpeed *= f;
		m_vecTurnSpeed *= 0.99f;
	}
}

void
CPhysical::AddToMovingList(void)
{
	m_movingListNode = CWorld::GetMovingEntityList().InsertItem(this);
}
