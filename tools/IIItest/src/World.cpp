#include "III.h"

CPtrList CWorld::ms_bigBuildingsList[4];
CPtrList CWorld::ms_listMovingEntityPtrs;
CSector  CWorld::ms_aSectors[100][100];

void
CWorld::Add(CEntity *ent)
{
//	if(ent->m_type == ENTITY_TYPE_VEHICLE ||
//	   ent->m_type == ENTITY_TYPE_PED)
//		; // do AUDIO stuff

	if(ent->m_isBigBuilding)
		ms_bigBuildingsList[ent->m_level].InsertItem(ent);
	else
		ent->Add();

	if(ent->m_type != ENTITY_TYPE_BUILDING &&
	   ent->m_type != ENTITY_TYPE_DUMMY &&
	   !ent->m_flagA4)
		((CPhysical*)ent)->AddToMovingList();
}

void
dumpEntity(CEntity *e)
{
	CBaseModelInfo *mi = CModelInfo::GetModelInfo(e->m_modelIndex);
	CVector *pos = e->GetPosition();
	debug("%d %s %f %f %f\n", e->m_modelIndex, mi->GetName(), pos->x, pos->y, pos->z);
}

