#include "III.h"

//
// CBaseModelInfo
//

CBaseModelInfo::CBaseModelInfo(int type)
{
	m_name[0] = '\0';
	m_colModel = nil;
	m_twodEffects = nil;
	m_objectId = -1;
	m_refCount = 0;
	m_txdSlot = -1;
	m_type = type;
	m_num2dEffects = 0;
	m_freeCol = false;
}

void
CBaseModelInfo::AddRef(void)
{
	m_refCount++;
	AddTexDictionaryRef();
}

void
CBaseModelInfo::RemoveRef(void)
{
	m_refCount--;
	RemoveTexDictionaryRef();
}

void
CBaseModelInfo::SetTexDictionary(const char *name)
{
	int slot = CTxdStore::FindTxdSlot(name);
	if(slot < 0)
		slot = CTxdStore::AddTxdSlot(name);
	m_txdSlot = slot;
}

void
CBaseModelInfo::AddTexDictionaryRef(void)
{
	CTxdStore::AddRef(m_txdSlot);
}

void
CBaseModelInfo::RemoveTexDictionaryRef(void)
{
	CTxdStore::RemoveRef(m_txdSlot);
}

void
CBaseModelInfo::Add2dEffect(C2dEffect *fx)
{
	if(m_twodEffects)
		m_num2dEffects++;
	else{
		m_num2dEffects = 1;
		m_twodEffects = fx;
	}
}

//
// CSimpleModelInfo
//

void
CSimpleModelInfo::DeleteRwObject(void)
{
	int i;
	for(i = 0; i < m_numAtomics; i++)
		if(m_atomics[i]){
			rw::Frame *f = m_atomics[i]->getFrame();
			m_atomics[i]->destroy();
			f->destroy();
			m_atomics[i] = nil;
			RemoveTexDictionaryRef();
		}
}

rw::Object*
CSimpleModelInfo::CreateInstance(void)
{
	if(GetRwObject()){
		rw::Atomic *a = ((rw::Atomic*)GetRwObject())->clone();
		a->setFrame(rw::Frame::create());
		return (rw::Object*)a;
	}else
		return nil;
}

rw::Object*
CSimpleModelInfo::CreateInstance(rw::Matrix *mat)
{
	if(GetRwObject()){
		rw::Atomic *a = (rw::Atomic*)CreateInstance();
		a->getFrame()->matrix = *mat;
		return (rw::Object*)a;
	}else
		return nil;
}

void
CSimpleModelInfo::Init(void)
{
	m_atomics[0] = nil;
	m_atomics[1] = nil;
	m_atomics[2] = nil;
	m_numAtomics = 0;
	m_furthest      = 0;
	m_normalCull    = 0;
	m_unknownFlag   = 0;
	m_isBigBuilding = 0;
	m_noFade        = 0;
	m_drawLast      = 0;
	m_additive      = 0;
	m_isSubway      = 0;
	m_ignoreLight   = 0;
	m_noZwrite      = 0;
}

void
CSimpleModelInfo::SetAtomic(int n, rw::Atomic *atomic)
{
	AddTexDictionaryRef();
	m_atomics[n] = atomic;
	if(m_ignoreLight)
		atomic->geometry->geoflags &= ~rw::Geometry::LIGHT;
}

void
CSimpleModelInfo::SetLodDistances(float *dist)
{
	m_lodDistances[0] = dist[0];
	m_lodDistances[1] = dist[1];
	m_lodDistances[2] = dist[2];
}

float
CSimpleModelInfo::GetLargestLodDistance(void)
{
	float d;
	// TODO: what exactly is going on here?
	if(m_furthest != 0 && !m_unknownFlag)
		d = m_lodDistances[m_furthest-1];
	else
		d = m_lodDistances[m_numAtomics-1];
	return d * TheCamera.m_LODmult;
}

rw::Atomic*
CSimpleModelInfo::GetAtomicFromDistance(float dist)
{
	int i;
	i = 0;
	// TODO: what exactly is going on here?
	if(m_unknownFlag)
		i = m_furthest;
	for(; i < m_numAtomics; i++)
		if(dist < m_lodDistances[i]*TheCamera.m_LODmult)
			return m_atomics[i];
	return nil;
}

void
CSimpleModelInfo::FindRelatedModel(void)
{
	int i;
	CBaseModelInfo *mi;
	for(i = 0; i < MODELINFOSIZE; i++){
		mi = CModelInfo::GetModelInfo(i);
		if(mi && mi != this &&
		   strcmp(GetName()+3, mi->GetName()+3) == 0){
			assert(mi->IsSimple());
			this->SetRelatedModel((CSimpleModelInfo*)mi);
			return;
		}
	}
}

void
CSimpleModelInfo::SetupBigBuilding(void)
{
	CSimpleModelInfo *related;
	if(m_lodDistances[0] > 300.0f && m_atomics[2] == nil){
		m_isBigBuilding = 1;
		FindRelatedModel();
		related = GetRelatedModel();
		if(related)
			m_lodDistances[2] = related->GetLargestLodDistance()/TheCamera.m_LODmult;
		else
			m_lodDistances[2] = 100.0f;
	}
}

//
// CTimeModelInfo
//

CTimeModelInfo*
CTimeModelInfo::FindOtherTimeModel(void)
{
	char name[40];
	char *p;
	int i;

	strcpy(name, GetName());
	// change _nt to _dy
	if(p = strstr(name, "_nt"))
		strncpy(p, "_dy", 4);
	// change _dy to _nt
	else if(p = strstr(name, "_dy"))
		strncpy(p, "_nt", 4);
	else
		return nil;

	for(i = 0; i < MODELINFOSIZE; i++){
		CBaseModelInfo *mi = CModelInfo::GetModelInfo(i);
		if(mi && mi->m_type == TIMEMODELINFO &&
		   strncmp(name, mi->GetName(), 24) == 0){
			m_otherTimeModelID = i;
			return (CTimeModelInfo*)mi;
		}
	}
	return nil;
}

CTimeModelInfo*
CTimeModelInfo::GetOtherModel(void)
{
	return (CTimeModelInfo*)CModelInfo::GetModelInfo(m_otherTimeModelID);
}

//
// CClumpModelInfo
//

void
CClumpModelInfo::SetClump(rw::Clump *clump)
{
	m_clump = clump;
	AddTexDictionaryRef();
	// TODO: more
}

//
// CModelInfo
//

CBaseModelInfo *CModelInfo::ms_modelInfoPtrs[MODELINFOSIZE];
CStore<CSimpleModelInfo, SIMPLEMODELSIZE> CModelInfo::ms_simpleModelStore;
CStore<CTimeModelInfo, TIMEMODELSIZE> CModelInfo::ms_timeModelStore;
CStore<CClumpModelInfo, CLUMPMODELSIZE> CModelInfo::ms_clumpModelStore;
CStore<CPedModelInfo, PEDMODELSIZE> CModelInfo::ms_pedModelStore;
CStore<CVehicleModelInfo, VEHICLEMODELSIZE> CModelInfo::ms_vehicleModelStore;
CStore<C2dEffect, TWODFXSIZE> CModelInfo::ms_2dEffectStore;

void
CModelInfo::Initialise(void)
{
	CSimpleModelInfo *mi;

	for(int i = 0; i < MODELINFOSIZE; i++)
		ms_modelInfoPtrs[i] = nil;
	ms_simpleModelStore.clear();
	ms_timeModelStore.clear();
	ms_clumpModelStore.clear();
	ms_pedModelStore.clear();
	ms_vehicleModelStore.clear();
	ms_2dEffectStore.clear();

#define NEWMODEL(id, col) \
	mi = CModelInfo::AddSimpleModel(id);\
	mi->SetColModel(&col, false);\
	mi->SetTexDictionary("generic");\
	mi->m_numAtomics = 1;\
	mi->m_lodDistances[0] = 80.0f;

	// car components
	NEWMODEL(190, CTempColModels::ms_colModelDoor1);
	NEWMODEL(191, CTempColModels::ms_colModelBumper1);
	NEWMODEL(192, CTempColModels::ms_colModelPanel1);
	NEWMODEL(193, CTempColModels::ms_colModelBonnet1);
	NEWMODEL(194, CTempColModels::ms_colModelBoot1);
	NEWMODEL(195, CTempColModels::ms_colModelWheel1);

	// ped components
	NEWMODEL(196, CTempColModels::ms_colModelBodyPart1);
	NEWMODEL(197, CTempColModels::ms_colModelBodyPart2);

#undef NEWMODEL
}

CSimpleModelInfo*
CModelInfo::AddSimpleModel(int id)
{
	CSimpleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_simpleModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CTimeModelInfo*
CModelInfo::AddTimeModel(int id)
{
	CTimeModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_timeModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CClumpModelInfo*
CModelInfo::AddClumpModel(int id)
{
	CClumpModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_clumpModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	return modelinfo;
}

CPedModelInfo*
CModelInfo::AddPedModel(int id)
{
	CPedModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_pedModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	return modelinfo;
}

CVehicleModelInfo*
CModelInfo::AddVehicleModel(int id)
{
	CVehicleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_vehicleModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	modelinfo->m_vehicleType = -1;
	modelinfo->m_extraModelIndex = -1;
	modelinfo->m_primaryMaterials[0] = nil;
	modelinfo->m_secondaryMaterials[0] = nil;
	return modelinfo;
}

CBaseModelInfo*
CModelInfo::GetModelInfo(const char *name, int *id)
{
	CBaseModelInfo *modelinfo;
	for(int i = 0; i < MODELINFOSIZE; i++){
		modelinfo = CModelInfo::ms_modelInfoPtrs[i];
	 	if(modelinfo && strcmp(modelinfo->GetName(), name) == 0){
			if(id)
				*id = i;
			return modelinfo;
		}
	}
	return nil;
}
