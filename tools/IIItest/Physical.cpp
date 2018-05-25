#include "III.h"

CPhysical::CPhysical(void)
{
	bUsesCollision = 1;
}

CPhysical::~CPhysical(void)
{
}

void
CPhysical::AddToMovingList(void)
{
	m_movingListNode = CWorld::GetMovingEntityList().InsertItem(this);
}
