#include "III.h"

void
CBaseModelInfo::ctor(int type)
{
	this->name[0] = '\0';
	this->colModel = nil;
	this->twodEffects = nil;
	this->id = -1;
	this->refCount = 0;
	this->txdSlot = -1;
	this->type = type;
	this->num2dEffects = 0;
	this->freeCol = 0;
}

void
CBaseModelInfo::AddRef(void)
{
	this->refCount++;
	AddTexDictionaryRef();
}

void
CBaseModelInfo::SetTexDictionary(char *name)
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

void
CSimpleModelInfo::SetAtomic(int n, rw::Atomic *atomic)
{
	AddTexDictionaryRef();
	this->atomics[n] = atomic;
	// TODO: lighting flag
}

void
CClumpModelInfo::SetClump(rw::Clump *clump)
{
	this->clump = clump;
	AddTexDictionaryRef();
	// TODO: more
}

CBaseModelInfo *CModelInfo::ms_modelInfoPtrs[MODELINFOSIZE];
CStore<CSimpleModelInfo, SIMPLEMODELSIZE> CModelInfo::ms_simpleModelStore;
CStore<CTimeModelInfo, TIMEMODELSIZE> CModelInfo::ms_timeModelStore;
CStore<CClumpModelInfo, CLUMPMODELSIZE> CModelInfo::ms_clumpModelStore;
CStore<CPedModelInfo, PEDMODELSIZE> CModelInfo::ms_pedModelStore;
CStore<CVehicleModelInfo, VEHICLEMODELSIZE> CModelInfo::ms_vehicleModelStore;
CStore<C2dEffect, TWODFXSIZE> CModelInfo::ms_2dEffectStore;

CSimpleModelInfo*
CModelInfo::AddSimpleModel(int id)
{
	CSimpleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_simpleModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->ctor(CSimpleModelInfo::ID);
	modelinfo->atomics[0] = nil;
	modelinfo->atomics[1] = nil;
	modelinfo->atomics[2] = nil;
	modelinfo->numAtomics = 0;
	modelinfo->flags = 0;
	return modelinfo;
}

CTimeModelInfo*
CModelInfo::AddTimeModel(int id)
{
	CTimeModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_timeModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->ctor(CTimeModelInfo::ID);
	modelinfo->atomics[0] = nil;
	modelinfo->atomics[1] = nil;
	modelinfo->atomics[2] = nil;
	modelinfo->numAtomics = 0;
	modelinfo->flags = 0;
	return modelinfo;
}

CClumpModelInfo*
CModelInfo::AddClumpModel(int id)
{
	CClumpModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_clumpModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->ctor(CClumpModelInfo::ID);
	modelinfo->clump = nil;
	return modelinfo;
}

CPedModelInfo*
CModelInfo::AddPedModel(int id)
{
	CPedModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_pedModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->ctor(CPedModelInfo::ID);
	modelinfo->clump = nil;
	return modelinfo;
}

CVehicleModelInfo*
CModelInfo::AddVehicleModel(int id)
{
	CVehicleModelInfo *modelinfo;
	modelinfo = CModelInfo::ms_vehicleModelStore.alloc();
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->ctor(CVehicleModelInfo::ID);
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
