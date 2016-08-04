#include "III.h"

int CRenderer::ms_nNoOfVisibleEntities;
int CRenderer::ms_nNoOfInVisibleEntities;
CEntity *CRenderer::ms_aVisibleEntityPtrs[2000];
CVector CRenderer::ms_vecCameraPosition;

void
CRenderer::ConstructRenderList(void)
{
	ms_nNoOfVisibleEntities = 0;
	ms_nNoOfInVisibleEntities = 0;
	ms_vecCameraPosition = TheCamera.m_position;
	ScanWorld();
}

void
CRenderer::ScanWorld(void)
{
	CVisibilityPlugins::InitAlphaEntityList();
	// TODO: collision level
	ScanBigBuildingList(&CWorld::GetBigBuildingList(CGame::currLevel));
	ScanBigBuildingList(&CWorld::GetBigBuildingList(LEVEL_NONE));
}

void
CRenderer::ScanBigBuildingList(CPtrList *list)
{
	CPtrNode *node;
	CEntity *e;
	for(node = list->first; node; node = node->next){
		e = (CEntity*)node->item;
		if(!e->m_flagC40 && SetupBigBuildingVisibility(e))
			ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = e;
	}
}

bool
CRenderer::SetupBigBuildingVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;

	// Timed object is not visible when other object exists
	// and this one is out of range.
	if(mi->m_type == TIMEMODELINFO){
 		ti = (CTimeModelInfo*)mi;
		if(ti->m_otherTimeModelID == -1 ||
		    ti->GetOtherModel()->GetRwObject() &&
		    !CClock::GetIsTimeInRange(ti->m_timeOn, ti->m_timeOff))
			return false;
	}else if(mi->m_type == VEHICLEMODELINFO)
		return ent->IsVisible();

	float dist = (ms_vecCameraPosition-*ent->GetPosition()).Magnitude();
	CSimpleModelInfo *nonLOD = mi->GetRelatedModel();

	// Find out whether to draw below near distance
	if(dist < mi->GetNearDistance() && dist < 330.0f){
		// If no non-LOD or the non-LOD is visible, don't render LOD
		if(nonLOD == nil ||
		   nonLOD->GetRwObject() && nonLOD->m_alpha == 0xFF)
			return false;

		// If the other non-LOD time model exists, don't render this one
		ti = (CTimeModelInfo*)nonLOD;
		if(nonLOD->m_type == TIMEMODELINFO &&
		   ti->m_otherTimeModelID != -1 &&
		   ti->GetOtherModel()->GetRwObject())
			return false;
	}

	rw::Atomic *a = mi->GetAtomicFromDistance(dist);
	if(a){
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		rw::Atomic *rwobj = (rw::Atomic*)ent->m_rwObject;
		if(a->geometry != rwobj->geometry)
			rwobj->setGeometry(a->geometry);
		// TODO: GetIsOnScreenComplex
		if(!ent->IsVisible())
			return false;
		if(mi->m_drawLast){
			CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
			ent->m_flagE1 = 0;
			return false;
		}
		return true;
	}

	// get faded atomic
	a = mi->GetAtomicFromDistance(dist-20.0f);

	// If nothing to fade...
	if(mi->m_noFade || a == nil){
		ent->DeleteRwObject();
		return false;
	}

	// Fade...
	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	rw::Atomic *rwobj = (rw::Atomic*)ent->m_rwObject;
	if(a->geometry != rwobj->geometry)
		rwobj->setGeometry(a->geometry);
	// TODO: GetIsOnScreenComplex
	if(ent->IsVisible())
		CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
	return false;
}

void
CRenderer::RenderEverything(void)
{
	int i;
	for(i = 0; i < ms_nNoOfVisibleEntities; i++)
		ms_aVisibleEntityPtrs[i]->Render();
}

void
CRenderer::RenderFadingInEntities(void)
{
	CVisibilityPlugins::RenderFadingEntities();
}
