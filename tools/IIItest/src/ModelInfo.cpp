#include "III.h"

//
// CBaseModelInfo
//

CBaseModelInfo::CBaseModelInfo(int type)
{
	this->name[0] = '\0';
	this->colModel = nil;
	this->twodEffects = nil;
	this->id = -1;
	this->refCount = 0;
	this->txdSlot = -1;
	this->type = type;
	this->num2dEffects = 0;
	this->freeCol = false;
}

void
CBaseModelInfo::AddRef(void)
{
	this->refCount++;
	AddTexDictionaryRef();
}

void
CBaseModelInfo::SetTexDictionary(const char *name)
{
	int slot = CTxdStore::FindTxdSlot(name);
	if(slot < 0)
		slot = CTxdStore::AddTxdSlot(name);
	this->txdSlot = slot;
}

void
CBaseModelInfo::AddTexDictionaryRef(void)
{
	CTxdStore::AddRef(this->txdSlot);
}

void
CBaseModelInfo::Add2dEffect(C2dEffect *fx)
{
	if(this->twodEffects)
		this->num2dEffects++;
	else{
		this->num2dEffects = 1;
		this->twodEffects = fx;
	}
}

//
// CSimpleModelInfo
//

void
CSimpleModelInfo::Init(void)
{
	this->atomics[0] = nil;
	this->atomics[1] = nil;
	this->atomics[2] = nil;
	this->numAtomics = 0;
	this->furthest      = 0;
	this->normalCull    = 0;
	this->unknownFlag   = 0;
	this->isBigBuilding = 0;
	this->noFade        = 0;
	this->drawLast      = 0;
	this->additive      = 0;
	this->isSubway      = 0;
	this->ignoreLight   = 0;
	this->noZwrite      = 0;
}

void
CSimpleModelInfo::SetAtomic(int n, rw::Atomic *atomic)
{
	AddTexDictionaryRef();
	this->atomics[n] = atomic;
	if(this->ignoreLight)
		atomic->geometry->geoflags &= ~rw::Geometry::LIGHT;
}

void
CSimpleModelInfo::SetLodDistances(float *dist)
{
	this->lodDistances[0] = dist[0];
	this->lodDistances[1] = dist[1];
	this->lodDistances[2] = dist[2];
}

float
CSimpleModelInfo::GetLargestLodDistance(void)
{
	float d;
	if(this->furthest != 0 && !this->unknownFlag)
		d = this->lodDistances[this->furthest-1];
	else
		d = this->lodDistances[this->numAtomics-1];
	return d;	// TODO camera multiplier
}

void
CSimpleModelInfo::FindRelatedModel(void)
{
	int i;
	CBaseModelInfo *mi;
	for(i = 0; i < MODELINFOSIZE; i++){
		mi = CModelInfo::GetModelInfo(i);
		if(mi && mi != this &&
		   strcmp(this->name+3, mi->name+3) == 0){
			assert(mi->type == CSimpleModelInfo::ID ||
			       mi->type == CTimeModelInfo::ID);
			this->SetRelatedModel((CSimpleModelInfo*)mi);
			return;
		}
	}
}

void
CSimpleModelInfo::SetupBigBuilding(void)
{
	CSimpleModelInfo *related;
	if(this->lodDistances[0] > 300.0f && this->atomics[2] == nil){
		this->isBigBuilding = 1;
		this->FindRelatedModel();
		related = this->GetRelatedModel();
		if(related)
			// TODO camera multiplier
			this->lodDistances[2] = related->GetLargestLodDistance();
		else
			this->lodDistances[2] = 100.0f;
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

	strcpy(name, this->name);
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
		if(mi && mi->type == CTimeModelInfo::ID &&
		   strncmp(name, mi->name, 24) == 0){
			this->otherTimeModelID = i;
			return (CTimeModelInfo*)mi;
		}
	}
	return nil;
}

//
// CClumpModelInfo
//

void
CClumpModelInfo::SetClump(rw::Clump *clump)
{
	this->clump = clump;
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
	mi->colModel = &col;\
	mi->freeCol = false;\
	mi->SetTexDictionary("generic");\
	mi->numAtomics = 1;\
	mi->lodDistances[0] = 80.0f;

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
	modelinfo->clump = nil;
	return modelinfo;
}

CPedModelInfo*
CModelInfo::AddPedModel(int id)
{
	CPedModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_pedModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->clump = nil;
	return modelinfo;
}

CVehicleModelInfo*
CModelInfo::AddVehicleModel(int id)
{
	CVehicleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_vehicleModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->clump = nil;
	modelinfo->vehicleType = -1;
	modelinfo->extraModelIndex = -1;
	modelinfo->primaryMaterials[0] = nil;
	modelinfo->secondaryMaterials[0] = nil;
	return modelinfo;
}

CBaseModelInfo*
CModelInfo::GetModelInfo(char *name, int *id)
{
	CBaseModelInfo *modelinfo;
	for(int i = 0; i < MODELINFOSIZE; i++){
		modelinfo = CModelInfo::ms_modelInfoPtrs[i];
		if(modelinfo && strcmp(modelinfo->name, name) == 0){
			if(id)
				*id = i;
			return modelinfo;
		}
	}
	return nil;
}
