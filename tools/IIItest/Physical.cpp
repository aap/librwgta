#include "III.h"

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

	m_vecCentreOfMass = CVector(0.0f, 0.0f, 0.0f);

	bAffectedByGravity = true;

	m_phy_flagA1 = false;
	bUsesCollision = true;
	m_phy_flagA4 = false;
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
				list = &s->m_vehicles;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_peds;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objects;
				break;
			}else switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_vehiclesOverlap;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_pedsOverlap;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objectsOverlap;
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
				list = &s->m_vehicles;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_peds;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objects;
				break;
			}else switch(m_type){
			case ENTITY_TYPE_VEHICLE:
				list = &s->m_vehiclesOverlap;
				break;
			case ENTITY_TYPE_PED:
				list = &s->m_pedsOverlap;
				break;
			case ENTITY_TYPE_OBJECT:
				list = &s->m_objectsOverlap;
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
CPhysical::GetSpeed(CVector &v)
{
	return m_vecMoveSpeed + m_vecMoveFriction + CrossProduct(m_vecTurnFriction + m_vecTurnSpeed, v);
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
CPhysical::ApplyFrictionMoveForce(float x, float y, float z)
{
	m_vecMoveFriction += CVector(x, y, z)*(1.0f/m_fMass);
}

void
CPhysical::ApplyFrictionTurnForce(float x1, float y1, float z1, float x2, float y2, float z2)
{
	CVector com = Multiply3x3(m_matrix, m_vecCentreOfMass);
	CVector turnf = CrossProduct(CVector(x2, y2, z2)-com, CVector(x1, y1, z1));
	m_vecTurnFriction += turnf*(1.0f/m_fTurnMass);
}

void
CPhysical::ApplySpringCollision(float f1, CVector &vec1, CVector &vec2, float f2, float f3)
{
	if(1.0f - f2 <= 0.0f)
		return;
	float step = min(CTimer::ms_fTimeStep, 3.0f);
	float strength = -0.008f*m_fMass*2.0f*step * f1 * (1.0f-f2) * f3;
	ApplyMoveForce(vec1.x*strength, vec1.y*strength, vec1.z*strength);
	ApplyTurnForce(vec1.x*strength, vec1.y*strength, vec1.z*strength, vec2.x, vec2.y, vec2.z);
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

	m_nCollisionRecords = 0;
	bHasCollided = 0;
	// TODO: three more values
	if(!bIsStuck){
		if(IsObject() ||
		   IsPed() && !m_flagA10){
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

bool
CPhysical::ProcessCollisionSectorList_SimpleCar(CPtrList *lists)
{
	// TODO, quite a bit of work
	return false;
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
		if(ProcessCollisionSectorList(node->list))
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
		if(ProcessCollisionSectorList_SimpleCar(node->list))
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

	if(!bUsesCollision){
		bIsStuck = false;
		bIsInSafePosition = true;
		RemoveAndAdd();
		return;
	}

	if(m_status == STATUS_SIMPLE){
		if(CheckCollision_SimpleCar() && m_status == STATUS_SIMPLE){	// does CheckCollision_SimpleCar change the status?
			m_status = STATUS_PHYSICS;
			if(IsVehicle())
				;// TODO: CCarCtrl::SwitchVehicleToRealPhysics
		}
		bIsStuck = false;
		bIsInSafePosition = true;
		RemoveAndAdd();
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
			// TODO: stuff if ped
			// restore state
			m_matrix = matrix;
			CTimer::ms_fTimeStep = savedTimeStep;
			if(collided)
				return;
			// TODO: stuff if vehicle
		}

		// No collision happened in the substeps
		ApplyMoveSpeed();
		ApplyTurnSpeed();
		m_matrix.Reorthogonalise();

		m_unk2 = 0;
		m_phy_flagA80 = false;
		if(m_vecMoveSpeed.x == 0.0f && m_vecMoveSpeed.y == 0.0f && m_vecMoveSpeed.z == 0.0f &&
		   m_vecTurnSpeed.x == 0.0f && m_vecTurnSpeed.y == 0.0f && m_vecTurnSpeed.z == 0.0f &&
		   !m_phy_flagA40 &&
		   m_status != STATUS_PLAYER &&
		   (!IsPed() /*TODO: || !CPed::IsPlayer()*/)){
			// Could this be a function?
	safe:
			m_phy_flagA40 = false;
			m_phy_flagA80 = false;
			m_fDistanceTravelled = (*GetMatrix().GetPosition() - *matrix.GetPosition()).Magnitude();
			bIsStuck = false;
			bIsInSafePosition = true;
			RemoveAndAdd();
			return;
		}
		// TODO: flag if vehicle

		if(!CheckCollision())
			goto safe;

		// there was a collision, restore old state
		GetMatrix() = matrix;
	}
}

void
CPhysical::ProcessShift(void)
{
	m_fDistanceTravelled = 0.0f;
	if(m_status == STATUS_SIMPLE){
		bIsStuck = false;
		bIsInSafePosition = true;
		RemoveAndAdd();
		return;
	}
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
		hasshifted |= ProcessShiftSectorList(node->list);
	m_unk2 = 0;
	if(hasshifted){
		CWorld::AdvanceCurrentScanCode();
		for(node = m_entryInfoList.first; node; node = node->next)
			if(ProcessCollisionSectorList(node->list)){
				GetMatrix() = matrix;
				return;
			}
	}

	bIsStuck = false;
	bIsInSafePosition = true;
	m_fDistanceTravelled = (*GetMatrix().GetPosition() - *matrix.GetPosition()).Magnitude();
	RemoveAndAdd();
}
