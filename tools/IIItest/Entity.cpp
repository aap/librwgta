#include "III.h"

CEntity::CEntity(void)
{
	m_type = ENTITY_TYPE_NOTHING;
	m_status = ENTITY_STATUS_4;
	m_doCollision = 0;
	m_flagA2 = 0;
	m_flagA4 = 0;
	m_flagA8 = 0;
	m_flagA10 = 0;
	m_flagA20 = 0;
	m_flagA40 = 0;
	m_flagA80 = 0;

	m_flagB1 = 0;
	m_flagB2 = 0;
	m_isVisible = 1;
	m_flagB8 = 0;
	m_flagB10 = 0;
	m_flagB20 = 0;
	m_isBigBuilding = 0;
	m_drawDamaged = 0;

	m_flagC1 = 0;
	m_flagC2 = 0;
	m_flagC4 = 0;
	m_flagC8 = 0;
	m_flagC10 = 0;
	m_flagC20 = 0;
	m_flagC40 = 0;
	m_flagC80 = 0;

	m_flagD1 = 0;
	m_flagD2 = 0;
	m_isBeingRendered = 0;
	m_flagD8 = 0;
	m_flagD10 = 0;
	m_flagD20 = 0;
	m_flagD40 = 0;
	m_flagD80 = 0;

	m_isFading = 0;
	m_flagE2 = 0;

	m_scanCode = -1;
	m_modelIndex = -1;
	m_rwObject = nil;
}

CEntity::~CEntity(void)
{
}

void
CEntity::SetupBigBuilding(void)
{
	CSimpleModelInfo *mi = (CSimpleModelInfo*)CModelInfo::GetModelInfo(m_modelIndex);
	m_isBigBuilding = 1;
	m_flagC20 = 1;
	m_doCollision = 0;
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
	return (m_rwObject && m_isVisible) ? GetIsOnScreen() : false;
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
	assert(xend < 100);
	assert(ystart >= 0);
	assert(yend < 100);

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
		m_isBeingRendered = 1;
		if(m_rwObject->type == rw::Atomic::ID)
			((rw::Atomic*)m_rwObject)->render();
		else
			((rw::Clump*)m_rwObject)->render();
		m_isBeingRendered = 0;
	}
}

void
CEntity::SetupLighting(void)
{
	DeActivateDirectional();
	SetAmbientColours();
}

void
CEntity::RemoveLighting(void)
{
}
