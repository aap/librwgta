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

	if(++CWorld::ms_nCurrentScanCode == 0){
		CWorld::ClearScanCodes();
		CWorld::ms_nCurrentScanCode = 1;
	}

	for(int i = 0; i < 100; i++)
	for(int j = 0; j < 100; j++){
		CSector *s = CWorld::GetSector(i, j);
		ScanSectorList(&s->m_buildings);
		ScanSectorList(&s->m_objects);
		ScanSectorList(&s->m_dummies);
	}

	ScanBigBuildingList(&CWorld::GetBigBuildingList(CCollision::ms_collisionInMemory));
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

void
CRenderer::ScanSectorList(CPtrList *list)
{
	CPtrNode *node;
	CEntity *e;
	for(node = list->first; node; node = node->next){
		e = (CEntity*)node->item;
		if(e->m_scanCode == CWorld::ms_nCurrentScanCode)
			continue;
		e->m_scanCode = CWorld::ms_nCurrentScanCode;
		switch(SetupEntityVisibility(e)){
		case 0:		// invisible
			break;
		case 1:		// visible
			ms_aVisibleEntityPtrs[ms_nNoOfVisibleEntities++] = e;
			break;
		case 2:		// kinda invisible?
			break;
		case 3:		// request
			CStreaming::RequestModel(e->m_modelIndex, 0);
			break;
		}
	}
}

// TODO: figure out magic values subtracted from distances
//       implement GetIsOnScreen etc.

bool
CRenderer::SetupBigBuildingVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;

	// If there are two time objects, we want one of them visible at
	// all times. If there is only one, we have it easier.
	// So the following conditions hold:
	// If there's no related time object,
	//   this one is invisible if its time is out of range.
	// If there is a related time object,
	//   it must be loaded if this one is to be invisible.
	if(mi->m_type == TIMEMODELINFO){
 		ti = (CTimeModelInfo*)mi;
		if((ti->m_otherTimeModelID == -1 ||
		    ti->GetOtherModel()->GetRwObject()) &&
		   !CClock::GetIsTimeInRange(ti->m_timeOn, ti->m_timeOff))
			return false;
	}else if(mi->m_type == VEHICLEMODELINFO)
		return ent->IsVisible();

	float dist = (ms_vecCameraPosition-*ent->GetPosition()).Magnitude();
	CSimpleModelInfo *nonLOD = mi->GetRelatedModel();

	// Find out whether to draw below near distance.
	// This is only the case if there is a non-LOD which is either not
	// loaded or not completely faded in yet.
	if(dist < mi->GetNearDistance() && dist < 330.0f){
		// No non-LOD or non-LOD is completely visible.
		if(nonLOD == nil ||
		   nonLOD->GetRwObject() && nonLOD->m_alpha == 0xFF)
			return false;

		// But if it is a time object, we'd rather draw the wrong
		// non-LOD than the right LOD.
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

		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
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

	if(mi->m_noFade){
		ent->DeleteRwObject();
		return false;
	}


	// get faded atomic
	a = mi->GetAtomicFromDistance(dist-20.0f);
	if(a == nil){
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

int
CRenderer::SetupEntityVisibility(CEntity *ent)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(ent->m_modelIndex);
	CTimeModelInfo *ti;

	bool request = true;
	if(mi->m_type == TIMEMODELINFO){
 		ti = (CTimeModelInfo*)mi;
		if(CClock::GetIsTimeInRange(ti->m_timeOn, ti->m_timeOff)){
			// don't fade in, or between time objects
			if(ti->m_otherTimeModelID == -1 ||
			   ti->GetOtherModel()->GetRwObject())
				ti->m_alpha = 0xFF;
		}else{
			// If we're out of range, this object is invisible
			// if there is no other object or
			//    the other object can be drawn
			if(ti->m_otherTimeModelID == -1 ||
			   ti->GetOtherModel()->GetRwObject())
				return 0;
			// ...so we'll try to draw this one, but don't request
			// it since what we really want is the other one.
			request = false;
		}
	}else if(mi->m_type != SIMPLEMODELINFO){
		// vehicles and peds and shit
		return 0;
	}else if(ent->m_type == ENTITY_TYPE_OBJECT)
		return 0;

	float dist = (ms_vecCameraPosition-*ent->GetPosition()).Magnitude();
	// uhm...what? when is is 330.0f < GetLargestLodDistance even true?
	if(330.0f < dist && dist < mi->GetLargestLodDistance())
		dist = mi->GetLargestLodDistance();
	if(ent->m_type == ENTITY_TYPE_OBJECT && ent->m_drawDamaged)
		mi->m_isDamaged = 1;

	rw::Atomic *a = mi->GetAtomicFromDistance(dist);
	if(a){
		mi->m_isDamaged = 0;
		if(ent->m_rwObject == nil)
			ent->CreateRwObject();
		assert(ent->m_rwObject);
		rw::Atomic *rwobj = (rw::Atomic*)ent->m_rwObject;
		// Make sure our atomic uses the right geometry and not
		// that of an atomic for another draw distance.
		if(a->geometry != rwobj->geometry)
			rwobj->setGeometry(a->geometry);
		mi->IncreaseAlpha();

		if(!(ent->m_rwObject && ent->m_isVisible))
			return 0;
		// TODO: GetIsOnScreen
		if(mi->m_alpha == 255){
			if(mi->m_drawLast || ent->m_flagD20){
				CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
				ent->m_flagE1 = 0;
				return 0;
			}
			return 1;
		}
		CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
		ent->m_flagE1 = 1;
		return 0;
	}

	if(mi->m_noFade){
		mi->m_isDamaged = 0;
		// request model
		if(dist-30.0f < mi->GetLargestLodDistance() && request)
			return 3;
		return 0;
	}

	// get faded atomic
	a = mi->GetAtomicFromDistance(dist-20.0f);
	mi->m_isDamaged = 0;
	if(a == nil){
		// request model
		if(dist-50.0f < mi->GetLargestLodDistance() && request)
			return 3;
		return 0;
	}

	if(ent->m_rwObject == nil)
		ent->CreateRwObject();
	assert(ent->m_rwObject);
	rw::Atomic *rwobj = (rw::Atomic*)ent->m_rwObject;
	if(a->geometry != rwobj->geometry)
		rwobj->setGeometry(a->geometry);
	mi->IncreaseAlpha();

	if(!(ent->m_rwObject && ent->m_isVisible))
		return 0;
	// TODO: GetIsOnScreen
	CVisibilityPlugins::InsertEntityIntoSortedList(ent, dist);
	ent->m_flagE1 = 1;
	return 2;
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
