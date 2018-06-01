#include "III.h"

CEntity::CEntity(void)
{
	m_type = ENTITY_TYPE_NOTHING;
	m_status = STATUS_ABANDONED;

	bUsesCollision = 0;
	bCollisionProcessed = 0;
	bIsStatic = 0;
	bHasContacted = 0;
	m_flagA10 = 0;
	bIsStuck = 0;
	bIsInSafePosition = 0;
	bUseCollisionRecords = 0;

	bWasPostponed = 0;
	m_flagB2 = 0;
	bIsVisible = 1;
	bHasCollided = 0;
	bRenderScorched = 0;
	m_flagB20 = 0;
	bIsBIGBuilding = 0;
	bRenderDamaged = 0;

	m_flagC1 = 0;
	m_flagC2 = 0;
	m_flagC4 = 0;
	m_flagC8 = 0;
	m_flagC10 = 0;
	m_flagC20 = 0;
	m_flagC40 = 0;
	m_flagC80 = 0;

	bRemoveFromWorld = 0;
	bHasHitWall = 0;
	bImBeingRendered = 0;
	m_flagD8 = 0;
	m_flagD10 = 0;
	bDrawLast = 0;
	m_flagD40 = 0;
	m_flagD80 = 0;	// tested in CObject::Render

	bDistanceFade = 0;
	m_flagE2 = 0;

	m_scanCode = -1;
	m_modelIndex = -1;
	m_rwObject = nil;
}

CEntity::~CEntity(void)
{
	DeleteRwObject();
	// TODO: ResolveReferences
}

void
CEntity::SetupBigBuilding(void)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(m_modelIndex);
	bIsBIGBuilding = 1;
	m_flagC20 = 1;
	bUsesCollision = 0;
	m_level = CTheZones::GetLevelFromPosition(GetPosition());
	if(m_level == LEVEL_NONE &&
	   mi->GetTxdSlot() != CTxdStore::FindTxdSlot("generic")){
		mi->SetTexDictionary("generic");
		printf("%d:%s txd has been set to generic\n",
		       m_modelIndex, mi->GetName());
	}
	if(mi->GetLodDistance(0) > 2000.0f)
		m_level = LEVEL_NONE;
}

bool
CEntity::IsVisible(void)
{
	return (m_rwObject && bIsVisible) ? GetIsOnScreen() : false;
}

bool
CEntity::GetIsOnScreen(void)
{
	CVector center;
	float radius;
	GetBoundCenter(center);
	radius = GetBoundRadius();
	// TODO: implement whatever GTA actually does
	return TheCamera.isSphereVisible(center, radius);
}

bool
CEntity::GetIsOnScreenComplex(void)
{
	// TODO:
	// this shouldn't be the same as GetIsOnScreen...
	return true;
}

bool
CEntity::GetIsTouching(const CVector center, float radius)
{
	CVector c1;
	float r1;
	GetBoundCenter(c1);
	r1 = GetBoundRadius();
	return (center - c1).MagnitudeSqr() < sq(r1+radius);
}

void
CEntity::GetBoundCenter(CVector &out)
{
	out = m_matrix * CModelInfo::GetModelInfo(m_modelIndex)->GetColModel()->boundingSphere.center;
};


/* Adds an entity into all sectors of its type it belongs to */
void
CEntity::Add(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.bottom);
	yend   = CWorld::GetSectorIndexY(bounds.top);
	ymid   = CWorld::GetSectorIndexY((bounds.bottom + bounds.top)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_buildings;
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_vehicles;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_peds;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objects;
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_dummies;
				break;
			}else switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_buildingsOverlap;
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_vehiclesOverlap;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_pedsOverlap;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objectsOverlap;
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_dummiesOverlap;
				break;
			}
			list->InsertItem(this);
		}
}

/* Removes an entity from all sectors of its type it belongs to */
void
CEntity::Remove(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;
	CPtrNode *node;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.bottom);
	yend   = CWorld::GetSectorIndexY(bounds.top);
	ymid   = CWorld::GetSectorIndexY((bounds.bottom + bounds.top)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_buildings;
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_vehicles;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_peds;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objects;
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_dummies;
				break;
			}else switch(m_type){
			case ENTITY_TYPE_BUILDING:
				list = &s->m_buildingsOverlap;
				break;
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_vehiclesOverlap;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_pedsOverlap;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objectsOverlap;
				break;
			case ENTITY_TYPE_DUMMY:
				list = &s->m_dummiesOverlap;
				break;
			}
			node = list->FindItem(this);
			assert(node);
			list->DeleteNode(node);
		}
}

void
CEntity::DeleteRwObject(void)
{
	m_matrix.Detach();
	if(m_rwObject){
		if(m_rwObject->type == rw::Atomic::ID){
			rw::Atomic *a = (rw::Atomic*)m_rwObject;
			rw::Frame *f = a->getFrame();
			a->destroy();
			f->destroy();
		}else if(m_rwObject->type == rw::Clump::ID)
			((rw::Clump*)m_rwObject)->destroy();
		m_rwObject = nil;
		CModelInfo::GetModelInfo(m_modelIndex)->RemoveRef();
	}
}

void
CEntity::UpdateRwFrame(void)
{
	if(m_rwObject){
		if(m_rwObject->type == rw::Atomic::ID)
			((rw::Atomic*)m_rwObject)->getFrame()->updateObjects();
		else if(m_rwObject->type == rw::Clump::ID)
			((rw::Clump*)m_rwObject)->getFrame()->updateObjects();
	}
}

void
CEntity::CreateRwObject(void)
{
	CBaseModelInfo *mi = CModelInfo::GetModelInfo(m_modelIndex);
	m_rwObject = mi->CreateInstance();
	if(m_rwObject == nil)
		return;
	if(m_rwObject->type == rw::Atomic::ID){
		rw::Atomic *a = (rw::Atomic*)m_rwObject;
		m_matrix.AttachRW(&a->getFrame()->matrix, false);
	}else if(m_rwObject->type == rw::Clump::ID){
		rw::Clump *c = (rw::Clump*)m_rwObject;
		m_matrix.AttachRW(&c->getFrame()->matrix, false);
	}
	mi->AddRef();
}

CRect
CEntity::GetBoundRect(void)
{
	CRect rect;
	CVector v;
	CColModel *col = CModelInfo::GetModelInfo(m_modelIndex)->GetColModel();

	rect.ContainPoint(m_matrix * col->boundingBox.min);
	rect.ContainPoint(m_matrix * col->boundingBox.max);

	v = col->boundingBox.min;
	v.x = col->boundingBox.max.x;
	rect.ContainPoint(m_matrix * v);

	v = col->boundingBox.max;
	v.x = col->boundingBox.min.x;
	rect.ContainPoint(m_matrix * v);

	return rect;
}

void
CEntity::Render(void)
{
	if(m_rwObject){
		bImBeingRendered = 1;
		if(m_rwObject->type == rw::Atomic::ID)
			((rw::Atomic*)m_rwObject)->render();
		else
			((rw::Clump*)m_rwObject)->render();
		bImBeingRendered = 0;
	}
}

void
CEntity::SetupLighting(void)
{
	DeActivateDirectional();
	SetAmbientColours();
}
