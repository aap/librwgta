#include "euryopa.h"

static ObjectInst **instArrays[NUMSCENES];
static int numInstArrays;

static IplDef ipllist[NUMIPLS];
static int numIpls;

int
AddInstArraySlot(int n)
{
	ObjectInst **instArray = rwNewT(ObjectInst*, n, 0);
	instArrays[numInstArrays++] = instArray;
	return numInstArrays-1;
}

ObjectInst**
GetInstArray(int i)
{
	return instArrays[i];
}

// streaming

static int
FindIplSlot(const char *name)
{
	int i;
	for(i = 0; i < numIpls; i++){
		if(rw::strncmp_ci(ipllist[i].name, name, MODELNAMELEN) == 0)
			return i;
	}
	return -1;
}

IplDef*
GetIplDef(int i)
{
	if(i < 0 || i >= numIpls){
//		log("warning: invalid Ipl slot %d\n", i);
		return nil;
	}
	return &ipllist[i];
}

int
AddIplSlot(const char *name)
{
	int i;
	i = FindIplSlot(name);
	if(i >= 0)
		return i;
	i = numIpls++;
	strncpy(ipllist[i].name, name, MODELNAMELEN);
	ipllist[i].instArraySlot = -1;
	ipllist[i].imageIndex = -1;
	return i;
}

void
LoadIpl(int slot)
{
	int i;
	int size;
	uint8 *buffer;
	FileObjectInstance *insts;
	GameFile *file;

	IplDef *ipl = GetIplDef(slot);

	if(ipl->imageIndex < 0){
		log("warning: no streaming info for ipl %s\n", ipl->name);
		return;
	}

	ObjectInst *lodinst;
	ObjectInst **instArray = GetInstArray(ipl->instArraySlot);

	buffer = ReadFileFromImage(ipl->imageIndex, &size);
	file = GetGameFileFromImage(ipl->imageIndex);
	if(*(uint32*)buffer == 0x79726E62){	// bnry
		int16 numInsts = *(int16*)(buffer+4);
		insts = (FileObjectInstance*)(buffer + *(int32*)(buffer+0x1C));
		for(i = 0; i < numInsts; i++){

			ObjectDef *obj = GetObjectDef(insts->objectId);
			if(obj == nil){
				log("warning: object %d was never defined\n", insts->objectId);
				return;
			}

			ObjectInst *inst = AddInstance();
			inst->Init(insts);
			inst->m_file = file;

			if(inst->m_lodId < 0)
				inst->m_lod = nil;
			else{
				lodinst = instArray[inst->m_lodId];
				inst->m_lod = lodinst;
				lodinst->m_numChildren++;
				ObjectDef *lodobj = GetObjectDef(lodinst->m_objectId);
				if(lodinst->m_numChildren == 1 && obj->m_colModel && lodobj->m_colModel != obj->m_colModel)
					lodobj->m_colModel = obj->m_colModel;
			}

			insts++;
		}

		// numCars: 0x14
		// cars: 0x3C
	}
	// TODO: parse text file (but no cars section :/)
}
