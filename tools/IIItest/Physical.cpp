#include "III.h"
#include "Ped.h"
#include "Vehicle.h"
#include "CarCtrl.h"

#define min(a,b) ((a) < (b) ? (a) : (b))

CPhysical::CPhysical(void)
{
	m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
	m_vecMoveFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);
	m_vecMoveSpeedAvg = CVector(0.0f, 0.0f, 0.0f);
	m_vecTurnSpeedAvg = CVector(0.0f, 0.0f, 0.0f);

	m_movingListNode = nil;
	m_nStaticFrames = 0;
	m_nCollisionRecords = 0;
	for(int i = 0; i < PHYSICAL_MAX_COLLISIONRECORDS; i++)
		m_aCollisionRecords[i] = nil;
	m_unk2 = 0;

	m_fCollisionImpulse = 0.0f;
	m_pCollidingEntity = nil;
	m_vecCollisionDirection = CVector(0.0f, 0.0f, 0.0f);
	m_nCollisionPieceType = 0;


	m_vecCentreOfMass = CVector(0.0f, 0.0f, 0.0f);

	bAffectedByGravity = true;

	m_phy_flagA1 = false;
	bUsesCollision = true;
	bInfiniteMass = false;
	m_phy_flagA8 = false;
	m_phy_flagA10 = false;
	m_phy_flagA20 = false;
	m_phy_flagA40 = false;
	m_phy_flagA80 = false;

	m_fDistanceTravelled = 0.0f;
}

CPhysical::~CPhysical(void)
{
}

void
CPhysical::Add(void)
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
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS];
				break;
			}else switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES_OVERLAP];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS_OVERLAP];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS_OVERLAP];
				break;
			}
			CPtrNode *node = list->InsertItem(this);
			m_entryInfoList.InsertItem(list, node, s);
		}
}

void
CPhysical::Remove(void)
{
	CEntryInfoNode *node, *next;
	for(node = m_entryInfoList.first; node; node = next){
		next = node->next;
		node->list->DeleteNode(node->listnode);
		m_entryInfoList.DeleteNode(node);
	}
}

void
CPhysical::RemoveAndAdd(void)
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

	// we'll try to recycle nodes from here
	CEntryInfoNode *next = m_entryInfoList.first;

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid) switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS];
				break;
			}else switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_lists[ENTITYLIST_VEHICLES_OVERLAP];
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_lists[ENTITYLIST_PEDS_OVERLAP];
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_lists[ENTITYLIST_OBJECTS_OVERLAP];
				break;
			}
			if(next){
				// If we still have old nodes, use them
				next->list->RemoveNode(next->listnode);
				list->InsertNode(next->listnode);
				next->list = list;
				next->sector = s;
				next = next->next;
			}else{
				CPtrNode *node = list->InsertItem(this);
				m_entryInfoList.InsertItem(list, node, s);
			}
		}

	// Remove old nodes we no longer need
	CEntryInfoNode *node;
	for(node = next; node; node = next){
		next = node->next;
		node->list->DeleteNode(node->listnode);
		m_entryInfoList.DeleteNode(node);
	}
}

CRect
CPhysical::GetBoundRect(void)
{
	CVector center;
	float radius;
	GetBoundCenter(center);
	radius = GetBoundRadius();
	return CRect(center.x-radius, center.y-radius, center.x+radius, center.y+radius);
}

void
CPhysical::AddToMovingList(void)
{
	m_movingListNode = CWorld::GetMovingEntityList().InsertItem(this);
}

void
CPhysical::RemoveFromMovingList(void)
{
	if(m_movingListNode){
		CWorld::GetMovingEntityList().DeleteNode(m_movingListNode);
		m_movingListNode = nil;
	}
}

CVector
CPhysical::GetSpeed(const CVector &p)
{
	return m_vecMoveSpeed + m_vecMoveFriction + CrossProduct(m_vecTurnFriction + m_vecTurnSpeed, p);
}

void
CPhysical::ApplyMoveSpeed(void)
{
	GetPosition() += m_vecMoveSpeed * CTimer::ms_fTimeStep;
}

void
CPhysical::ApplyTurnSpeed(void)
{
	CVector turnvec = m_vecTurnSpeed*CTimer::ms_fTimeStep;
	GetRight() += CrossProduct(turnvec, GetRight());
	GetForward() += CrossProduct(turnvec, GetForward());
	GetUp() += CrossProduct(turnvec, GetUp());
}

void
CPhysical::ApplyMoveForce(float x, float y, float z)
{
	m_vecMoveSpeed += CVector(x, y, z)*(1.0f/m_fMass);
}

void
CPhysical::ApplyTurnForce(float vx, float vy, float vz, float px, float py, float pz)
{
	CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
	CVector turnf = CrossProduct(CVector(px, py, pz)-com, CVector(vx, vy, vz));
	m_vecTurnSpeed += turnf*(1.0f/m_fTurnMass);
}

void
CPhysical::ApplyFrictionMoveForce(float x, float y, float z)
{
	m_vecMoveFriction += CVector(x, y, z)*(1.0f/m_fMass);
}

void
CPhysical::ApplyFrictionTurnForce(float vx, float vy, float vz, float px, float py, float pz)
{
	CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
	CVector turnf = CrossProduct(CVector(px, py, pz)-com, CVector(vx, vy, vz));
	m_vecTurnFriction += turnf*(1.0f/m_fTurnMass);
}

void
CPhysical::ApplySpringCollision(float f1, CVector &v, CVector &p, float f2, float f3)
{
	if(1.0f - f2 <= 0.0f)
		return;
	float step = min(CTimer::ms_fTimeStep, 3.0f);
	float strength = -0.008f*m_fMass*2.0f*step * f1 * (1.0f-f2) * f3;
	ApplyMoveForce(v.x*strength, v.y*strength, v.z*strength);
	ApplyTurnForce(v.x*strength, v.y*strength, v.z*strength, p.x, p.y, p.z);
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

// TODO
bool
CPhysical::ApplyCollision(CPhysical *B, CColPoint &colpoint, float &impulseA, float &impulseB)
{
	CPhysical *A = this;

	bool ispedcontactA = false;
	bool ispedcontactB = false;

	float timestepA;
	if(B->bPedPhysics){
		timestepA = 10.0f;
		if(B->IsPed() && B->Ped()->m_pContactEntity == A)
			ispedcontactA = true;
	}else
		timestepA = A->m_phy_flagA1 ? 2.0f : 1.0f;

	float timestepB;
	if(A->bPedPhysics){
		if(A->IsPed() && A->Ped()->IsPlayer() && B->IsVehicle() &&
		   (B->m_status == STATUS_ABANDONED || B->m_status == STATUS_WRECKED || A->bHasHitWall))
			timestepB = 2200.0f / B->m_fMass;
		else
			timestepB = 10.0f;

		if(A->IsPed() && A->Ped()->m_pContactEntity == B)
			ispedcontactB = true;
	}else
		timestepB = B->m_phy_flagA1 ? 2.0f : 1.0f;

	float speedA, speedB;
	if(B->bIsStatic){
		if(A->bPedPhysics){
			speedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
			if(speedA < 0.0f){
				if(B->IsObject()){
					impulseA = -speedA * A->m_fMass;
					impulseB = impulseA;
					assert(0 && "TODO: implement object and glass");
				}else if(!B->bInfiniteMass)
					B->bIsStatic = false;
	
				if(B->bInfiniteMass){
					impulseA = -speedA * A->m_fMass;
					impulseB = 0.0f;
					if(!A->bInfiniteMass)
						A->ApplyMoveForce(colpoint.normal*(1.0f + A->m_fElasticity)*impulseA);
					return true;
				}
		}else{
			}
			CVector pointposA = colpoint.point - A->GetPosition();
			speedA = DotProduct(A->GetSpeed(pointposA), colpoint.normal);
			if(speedA < 0.0f){
				if(B->IsObject()){
					float sA = -1.0f;
					if(A->bHasHitWall)
						sA -= A->m_fElasticity;
					impulseA = sA * speedA *
						(1.0f / (CrossProduct(pointposA, colpoint.normal).MagnitudeSqr()/A->m_fTurnMass +
					                1.0f/A->m_fMass));
					impulseB = impulseA;
					assert(0 && "TODO: implement object and glass");
				}else if(!B->bInfiniteMass)
					B->bIsStatic = false;
			}
		}
	
		if(B->bIsStatic)
			return false;
		if(!B->bInfiniteMass)
			B->AddToMovingList();
	}

	// B is not static
	if(A->bPedPhysics && B->bPedPhysics){
		// negative if A is moving towards B
		speedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
		// positive if B is moving towards A
		speedB = DotProduct(B->m_vecMoveSpeed, colpoint.normal);
		// not interested in how much B moves into A apparently?
		// only interested in cases where A collided into B
		if(speedB > 0.0f) speedB = 0.0f;
		// A has moved into B
		if(speedA < speedB){
			if(!A->bHasHitWall)
				speedB -= (speedA - speedB) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (speedB-speedA) * A->m_fMass * timestepA;
			if(!A->bInfiniteMass)
				A->ApplyMoveForce(colpoint.normal*(impulseA/timestepA));
			return true;
		}else
			return false;
	}else if(A->bPedPhysics){
		CVector pointposB = colpoint.point - B->GetPosition();
		speedA = DotProduct(A->m_vecMoveSpeed, colpoint.normal);
		speedB = DotProduct(B->GetSpeed(pointposB), colpoint.normal);

		float a = A->m_fMass*timestepA;
		float b = 1.0f /
			(CrossProduct(pointposB, colpoint.normal).MagnitudeSqr()/(B->m_fTurnMass*timestepB) +
				1.0f/(B->m_fMass*timestepB));
		float speedSum = (b*speedB + a*speedA)/(a + b);
		if(speedA < speedSum){
			float sA = speedSum;
			float sB = speedSum;
			if(A->bHasHitWall)
				sA -= (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			if(B->bHasHitWall)
				sB -= (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (sA - speedA) * a;
			impulseB = (sB - speedB) * b;
			if(!A->bInfiniteMass){
				CVector f = colpoint.normal*(impulseA/timestepA);
				if(f.z < 0.0f) f.z = 0.0f;
				if(ispedcontactB){
					f.x *= 2.0f;
					f.y *= 2.0f;
				}
				A->ApplyMoveForce(f);
			}
			if(!B->bInfiniteMass && !ispedcontactB){
				CVector f = colpoint.normal*(-impulseB/timestepB);
				B->ApplyMoveForce(f);
				B->ApplyTurnForce(f, pointposB);
			}
			return true;
		}else
			return false;
	}else if(B->bPedPhysics){
		CVector pointposA = colpoint.point - A->GetPosition();
		speedA = DotProduct(A->GetSpeed(pointposA), colpoint.normal);
		speedB = DotProduct(B->m_vecMoveSpeed, colpoint.normal);

		float a = 1.0f /
			(CrossProduct(pointposA, colpoint.normal).MagnitudeSqr()/(A->m_fTurnMass*timestepA) +
				1.0f/(A->m_fMass*timestepA));
		float b = B->m_fMass*timestepB;
		float speedSum = (b*speedB + a*speedA)/(a + b);
		if(speedA < speedSum){
			float sA = speedSum;
			float sB = speedSum;
			if(A->bHasHitWall)
				sA -= (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			if(B->bHasHitWall)
				sB -= (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (sA - speedA) * a;
			impulseB = (sB - speedB) * b;
			if(!A->bInfiniteMass && !ispedcontactA){
				CVector f = colpoint.normal*(impulseA/timestepA);
				if(f.z < 0.0f) f.z = 0.0f;
				A->ApplyMoveForce(f);
				A->ApplyTurnForce(f, pointposA);
			}
			if(!B->bInfiniteMass){
				CVector f = colpoint.normal*(-impulseB/timestepB);
				if(f.z < 0.0f){
					f.z = 0.0f;
					if(abs(speedA) < 0.01f)
						f *= 0.5f;
				}
				if(ispedcontactA){
					f.x *= 2.0f;
					f.y *= 2.0f;
				}
				B->ApplyMoveForce(f);
			}
			return true;
		}else
			return false;
	}else{
		CVector pointposA = colpoint.point - A->GetPosition();
		CVector pointposB = colpoint.point - B->GetPosition();
		speedA = DotProduct(A->GetSpeed(pointposA), colpoint.normal);
		speedB = DotProduct(B->GetSpeed(pointposB), colpoint.normal);
		float a = 1.0f /
			(CrossProduct(pointposA, colpoint.normal).MagnitudeSqr()/(A->m_fTurnMass*timestepA) +
				1.0f/(A->m_fMass*timestepA));
		float b = 1.0f /
			(CrossProduct(pointposB, colpoint.normal).MagnitudeSqr()/(B->m_fTurnMass*timestepB) +
				1.0f/(B->m_fMass*timestepB));
		float speedSum = (b*speedB + a*speedA)/(a + b);
		if(speedA < speedSum){
			float sA = speedSum;
			float sB = speedSum;
			if(A->bHasHitWall)
				sA -= (speedA - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			if(B->bHasHitWall)
				sB -= (speedB - speedSum) * (A->m_fElasticity+B->m_fElasticity)/2.0f;
			impulseA = (sA - speedA) * a;
			impulseB = (sB - speedB) * b;
			CVector fA = colpoint.normal*(impulseA/timestepA);
			CVector fB = colpoint.normal*(-impulseB/timestepB);
			if(A->IsVehicle() && !A->bHasHitWall){
				fA.x *= 1.4f;
				fA.y *= 1.4f;
				if(colpoint.normal.z < 0.7f)
					fA.z *= 0.3f;
				if(A->m_status == STATUS_PLAYER)
					pointposA *= 0.8f;
				if(CWorld::bNoMoreCollisionTorque){
					A->ApplyFrictionMoveForce(fA*-0.3f);
					A->ApplyFrictionTurnForce(fA*-0.3f, pointposA);
				}
			}
			if(B->IsVehicle() && !B->bHasHitWall){
				fB.x *= 1.4f;
				fB.y *= 1.4f;
				if(colpoint.normal.z < 0.7f)
					fB.z *= 0.3f;
				if(B->m_status == STATUS_PLAYER)
					pointposB *= 0.8f;
				if(CWorld::bNoMoreCollisionTorque){
					// BUG: the game actually uses A here, but this can't be right
					B->ApplyFrictionMoveForce(fB*-0.3f);
					B->ApplyFrictionTurnForce(fB*-0.3f, pointposB);
				}
			}
			if(!A->bInfiniteMass){
				A->ApplyMoveForce(fA);
				A->ApplyTurnForce(fA, pointposA);
			}
			if(!B->bInfiniteMass){
				if(B->bIsInSafePosition)
					B->UnsetIsInSafePosition();
				B->ApplyMoveForce(fB);
				B->ApplyTurnForce(fB, pointposB);
			}
			return true;
		}else
			return false;
	}

	// not reached
	return false;
}

// TODO
bool
CPhysical::ApplyFriction(CPhysical *B, float adhesiveLimit, CColPoint &colpoint)
{
	return false;
}

void
CPhysical::SetDamagedPieceRecord(uint16 pieceType, float colImpulse, CEntity *colEntity, CVector colDirection)
{
	if(m_fCollisionImpulse < colImpulse){
		m_fCollisionImpulse = colImpulse;
		m_nCollisionPieceType = pieceType;
		m_pCollidingEntity = this;
		RegisterReference(&m_pCollidingEntity);
		m_vecCollisionDirection = colDirection;
	}
}

void
CPhysical::AddCollisionRecord(CEntity *ent)
{
	AddCollisionRecord_Treadable(ent);
	this->bHasCollided = true;
	ent->bHasCollided = true;
	if(IsVehicle() && ent->IsVehicle()){
		// TODO
	}
	if(bUseCollisionRecords){
		int i;
		for(i = 0; i < m_nCollisionRecords; i++)
			if(m_aCollisionRecords[i] == ent)
				return;
		if(m_nCollisionRecords < PHYSICAL_MAX_COLLISIONRECORDS)
			m_aCollisionRecords[m_nCollisionRecords++] = ent;
		m_nLastTimeCollided = CTimer::m_snTimeInMilliseconds;

		if(bHasContacted){
		}else{
		}
	}
}

void
CPhysical::AddCollisionRecord_Treadable(CEntity *ent)
{
	// TODO
}

bool
CPhysical::GetHasCollidedWith(CEntity *ent)
{
	int i;
	if(bUseCollisionRecords)
		for(i = 0; i < m_nCollisionRecords; i++)
			if(m_aCollisionRecords[i] == ent)
				return true;
	return false;
}

void
CPhysical::ProcessControl(void)
{
	if(!IsPed())
		m_phy_flagA8 = false;
	bHasContacted = false;
	bIsInSafePosition = false;
	bWasPostponed = false;
	bHasHitWall = false;

	if(m_status == STATUS_SIMPLE)
		return;

	m_nCollisionPieceType = 0;
	m_pCollidingEntity = nil;
	m_fCollisionImpulse = 0.0f;

	m_nCollisionRecords = 0;
	bHasCollided = 0;
	if(!bIsStuck){
		if(IsObject() ||
		   IsPed() && !bPedPhysics){
			m_vecMoveSpeedAvg = (m_vecMoveSpeedAvg + m_vecMoveSpeed)/2.0f;
			m_vecTurnSpeedAvg = (m_vecTurnSpeedAvg + m_vecTurnSpeed)/2.0f;
			float step = CTimer::ms_fTimeStep * 0.003;
			if(m_vecMoveSpeedAvg.MagnitudeSqr() < step*step &&
			   m_vecTurnSpeedAvg.MagnitudeSqr() < step*step)
				m_nStaticFrames = 0;
			else if(m_nStaticFrames++ > 10){
				m_nStaticFrames = 10;
				bIsStatic = true;
				m_vecMoveFriction = m_vecMoveSpeed;
				m_vecTurnFriction = m_vecTurnSpeed;
				m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
				m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);
				return;
			}
		}
	}
	ApplyGravity();
	ApplyFriction();
	ApplyAirResistance();
}

bool
CPhysical::ProcessCollisionSectorList(CPtrList *lists)
{
	// TODO, a ton of work
	return false;
}

// TODO
bool
CPhysical::ProcessCollisionSectorList_SimpleCar(CSector *sector)
{
	static CColPoint aColPoints[32];

	if(!bUsesCollision)
		return false;

	float radius = GetBoundRadius();
	CVector boundcenter;
	GetBoundCenter(boundcenter);

	CEntity *ent = nil;	// our first colliding entity
	for(int listtype = 3; listtype >= 0; listtype--){
		// Go through vehicles and objects
		CPtrList *list;
		switch(listtype){
		case 0:	list = &sector->m_lists[ENTITYLIST_VEHICLES]; break;
		case 1:	list = &sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP]; break;
		case 2:	list = &sector->m_lists[ENTITYLIST_OBJECTS]; break;
		case 3:	list = &sector->m_lists[ENTITYLIST_OBJECTS_OVERLAP]; break;
		}

		// Find first collision in list
		CPtrNode *listnode;
		for(listnode = list->first; listnode; listnode = listnode->next){
			ent = (CEntity*)listnode->item;
			if(ent != this &&
			   ent->m_scanCode != CWorld::GetCurrentScanCode() &&
			   ent->bUsesCollision &&
			   ent->GetIsTouching(boundcenter, radius)){
				ent->m_scanCode = CWorld::GetCurrentScanCode();
				if(ProcessEntityCollision(ent, aColPoints) > 0)
					goto collision;
			}
		}
	}
	// no collision
	return false;

collision:

	// TODO
	if(bHasContacted){
		if(ent->bHasContacted){
		}else{
		}
	}else{
		if(ent->bHasContacted){
		}else{
		}
	}

	if(ent->m_status == STATUS_SIMPLE){
		ent->m_status = STATUS_PHYSICS;
		if(IsVehicle())
			CCarCtrl::SwitchVehicleToRealPhysics(Vehicle());
	}
	return true;
}

bool
CPhysical::ProcessShiftSectorList(CPtrList *lists)
{
	// TODO, quite a bit of work
	return false;
}

bool
CPhysical::CheckCollision(void)
{
	CEntryInfoNode *node;

	bCollisionProcessed = false;
	CWorld::AdvanceCurrentScanCode();
	for(node = m_entryInfoList.first; node; node = node->next)
		if(ProcessCollisionSectorList(node->sector->m_lists))
			return true;
	return false;
}

bool
CPhysical::CheckCollision_SimpleCar(void)
{
	CEntryInfoNode *node;

	bCollisionProcessed = false;
	CWorld::AdvanceCurrentScanCode();
	for(node = m_entryInfoList.first; node; node = node->next)
		if(ProcessCollisionSectorList_SimpleCar(node->sector))
			return true;
	return false;
}

int32
CPhysical::ProcessEntityCollision(CEntity *ent, CColPoint *colpoints)
{
	int32 numSpheres = CCollision::ProcessColModels(
		GetMatrix(), *CModelInfo::GetModelInfo(GetModelIndex())->GetColModel(),
		ent->GetMatrix(), *CModelInfo::GetModelInfo(ent->GetModelIndex())->GetColModel(),
		colpoints,
		nil, nil);	// No Lines allowed!
	if(numSpheres > 0){
		AddCollisionRecord(ent);
		if(!ent->IsBuilding())	// Can't this catch dummies too?
			((CPhysical*)ent)->AddCollisionRecord(this);
		if(ent->IsBuilding() || ent->bIsStatic)
			this->bHasHitWall = true;
	}
	return numSpheres;
}


// TODO
void
CPhysical::ProcessCollision(void)
{
	m_phy_flagA80 = false;
	m_fDistanceTravelled = 0.0f;
	m_unk2 = 0;

	if(bUsesCollision){
		if(m_status == STATUS_SIMPLE){
			if(CheckCollision_SimpleCar() && m_status == STATUS_SIMPLE){	// does CheckCollision_SimpleCar change the status?
				m_status = STATUS_PHYSICS;
				if(IsVehicle())
					CCarCtrl::SwitchVehicleToRealPhysics(Vehicle());
			}
		}else{
			// Save current state
			CMatrix matrix(GetMatrix());
			float savedTimeStep = CTimer::ms_fTimeStep;

			int n;	// The number of steps we divide the time step into
			float step = 0.0f;	// divided time step

			// Find out how to subdivide the time step
			if(IsPed()){
				// TODO
				assert(0);
			}else if(IsVehicle()){
				float distsq = m_vecMoveSpeed.MagnitudeSqr() * savedTimeStep*savedTimeStep;
				if(distsq < 0.16f)
					n = 0;
				else{
					int m;
					if(m_status == STATUS_PLAYER)
						m = 2;
					else if(distsq > 0.32)
						m = 3;
					else
						m = 4;
					n = (uint8)ceil(sqrt(distsq) * (10.0f/m));
					step = savedTimeStep / n;
				}
			}else if(IsObject()){
				// TODO
				assert(0);
			}else
				n = 0;

			// Check for collision every subdivided timestep
			for(int i = 1; i < n; i++){
				CTimer::ms_fTimeStep = step * i;
				ApplyMoveSpeed();
				ApplyTurnSpeed();
				bool collided = CheckCollision();
				if(IsPed()){
					// TODO
				}
				// restore state
				m_matrix = matrix;
				CTimer::ms_fTimeStep = savedTimeStep;
				if(collided)
					return;
				if(IsVehicle()){
					// TODO
				}
			}

			// No collision happened in the substeps
			ApplyMoveSpeed();
			ApplyTurnSpeed();
			m_matrix.Reorthogonalise();

			m_unk2 = 0;
			m_phy_flagA80 = false;
			if(m_vecMoveSpeed.x != 0.0f || m_vecMoveSpeed.y != 0.0f || m_vecMoveSpeed.z != 0.0f ||
			   m_vecTurnSpeed.x != 0.0f || m_vecTurnSpeed.y != 0.0f || m_vecTurnSpeed.z != 0.0f ||
			   m_phy_flagA40 ||
			   m_status == STATUS_PLAYER ||
			   IsPed() && Ped()->IsPlayer()){
				if(IsVehicle()){
					// TODO: flag
				}
				if(CheckCollision()){
					// there was a collision, restore old state
					GetMatrix() = matrix;
					return;
				}
			}

			m_phy_flagA40 = false;
			m_phy_flagA80 = false;
			m_fDistanceTravelled = (GetPosition() - *matrix.GetPosition()).Magnitude();
		}
	}

	bIsStuck = false;
	bIsInSafePosition = true;
	RemoveAndAdd();
}

void
CPhysical::ProcessShift(void)
{
	m_fDistanceTravelled = 0.0f;
	if(m_status != STATUS_SIMPLE){
		CMatrix matrix(GetMatrix());
		ApplyMoveSpeed();
		ApplyTurnSpeed();
		GetMatrix().Reorthogonalise();

		CWorld::AdvanceCurrentScanCode();

		if(IsVehicle())
			m_unk2 = 1;

		CEntryInfoNode *node;
		bool hasshifted = false;	// whatever that means...
		for(node = m_entryInfoList.first; node; node = node->next)
			hasshifted |= ProcessShiftSectorList(node->sector->m_lists);
		m_unk2 = 0;
		if(hasshifted){
			CWorld::AdvanceCurrentScanCode();
			for(node = m_entryInfoList.first; node; node = node->next)
				if(ProcessCollisionSectorList(node->sector->m_lists)){
					GetMatrix() = matrix;
					return;
				}
		}
		m_fDistanceTravelled = (GetPosition() - *matrix.GetPosition()).Magnitude();
	}
	bIsStuck = false;
	bIsInSafePosition = true;
	RemoveAndAdd();
}
