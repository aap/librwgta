#include "storiesview.h"

FILE *logfile;

BuildingExt *GetBuildingExt(int id);

void
dumpEntityBS(FILE *f, int pid, CEntity *e)
{
	rw::Matrix mat;
	CBaseModelInfo *mi = CModelInfo::Get(e->modelIndex);
	if(mi == nil || mi->colModel == nil)
		return;
	if(mi->type != MODELINFO_SIMPLE && mi->type != MODELINFO_TIME)
		return;
	CSimpleModelInfo *smi = (CSimpleModelInfo*)mi;
	CTimeModelInfo *tmi = nil;
	if(mi->type == MODELINFO_TIME)
		tmi = (CTimeModelInfo*)mi;

	mat = *(rw::Matrix*)&e->placeable.matrix.matrix;
	mat.optimize();
	rw::V3d center;
	rw::V3d::transformPoints(&center, (rw::V3d*)&mi->colModel->boundingSphere.center, 1, &mat);

	fprintf(f, "%x  %d %d %s  %f %f %f %f", pid, e->modelIndex, !!(smi->flags & 0x10), mi->name, center.x, center.y, center.z, mi->colModel->boundingSphere.radius);
	if(tmi)
		fprintf(f, " %d %d", tmi->timeOn, tmi->timeOff);
	fprintf(f, "\n");
}

void
dumpInstBS(int level, sGeomInstance *inst)
{
	float x = halfFloatToFloat(inst->bound[0]);
	float y = halfFloatToFloat(inst->bound[1]);
	float z = halfFloatToFloat(inst->bound[2]);
	float r = halfFloatToFloat(inst->bound[3]);
	BuildingExt *be = GetBuildingExt(inst->GetId());

	fprintf(logfile, "%X %d %d  %f %f %f %f", inst->GetId() | level<<16, inst->IsLOD(), be->interior, x, y, z, r);
	if(be->isTimed)
		fprintf(logfile, " %d %d", be->timeOn, be->timeOff);
	fprintf(logfile, "\n");
}

void
openLogFile(char *path)
{
	logfile = fopen(path, "w");
	assert(logfile);
}

void
closeLogFile(void)
{
	fclose(logfile);
}

void
dumpIPLBoundingSpheres(void)
{
	CEntity *e;
	int i, n;

	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		dumpEntityBS(logfile, i, e);
	}

	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		dumpEntityBS(logfile, 1<<16 | i, e);
	}
}


CEntity*
GetEntityById(int id)
{
	if((id & ~0xFFFF) == 0)
		return pBuildingPool->GetSlot(id & 0xFFFF);
	else
		return pTreadablePool->GetSlot(id & 0xFFFF);
}

#ifdef VCS

char *linkpath = "C:\\Users\\aap\\Desktop\\stories\\vcs_map\\vcs_links.txt";

struct Link
{
	int worldId;
	int iplId;
} inactiveLinks[0x8000];
int numInactiveLinks;

void
LinkInstances(void)
{
	int i, j, n;
	CEntity *e;
	BuildingExt *be;
	EntityExt *ee;

	// reuse vtable to store world building ids
	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		ee = rwNewT(EntityExt, 1, 0);
		memset(ee, 0, sizeof(*ee));
		ee->entity = e;
		e->vtable = ee;
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		ee = rwNewT(EntityExt, 1, 0);
		memset(ee, 0, sizeof(*ee));
		ee->entity = e;
		e->vtable = ee;
	}

	FILE *f = fopen(linkpath, "r");
	if(f == nil)
		abort();

	int levid = gLevel->levelid << 16;
	int worldId, iplId;
	while(fscanf(f, "%x %x", &worldId, &iplId) == 2){
		if((worldId & ~0xFFFF) != levid){
			inactiveLinks[numInactiveLinks].worldId = worldId;
			inactiveLinks[numInactiveLinks].iplId = iplId;
			numInactiveLinks++;
			continue;
		}

		be = GetBuildingExt(worldId&0x7FFF);
		be->SetEntity(iplId);
	}

/*
	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		ee = (EntityExt*)e->vtable;
		if(ee->n > 1){
			for(j = 0; j < ee->n; j++)
				ee->insts[j]->iplId = -1;
			ee->n = 0;
		}
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		ee = (EntityExt*)e->vtable;
		if(ee->n > 1){
			for(j = 0; j < ee->n; j++)
				ee->insts[j]->iplId = -1;
			ee->n = 0;
		}
	}
*/
	fclose(f);
}

void
WriteLinks(void)
{
	int i, j, n;
	CEntity *e;
	EntityExt *ee;
	int iplId;

	FILE *f = fopen(linkpath, "w");
	if(f == nil)
		abort();

	int levid = gLevel->levelid << 16;
	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		ee = (EntityExt*)e->vtable;
		iplId = i;
		for(j = 0; j < ee->n; j++)
			fprintf(f, "%x %x\n", ee->insts[j]->id | levid, iplId);
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		ee = (EntityExt*)e->vtable;
		iplId = i | 0x10000;
		for(j = 0; j < ee->n; j++)
			fprintf(f, "%x %x\n", ee->insts[j]->id | levid, iplId);
	}
	for(i = 0; i < numInactiveLinks; i++)
		fprintf(f, "%x %x\n", inactiveLinks[i].worldId, inactiveLinks[i].iplId);

	fclose(f);
}

#else

void
LinkInstances(void)
{
	int i, n;
	CEntity *e;
	EntityExt *bl;

	// reuse vtable to store world building ids
	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		bl = rwNewT(EntityExt, 1, 0);
		memset(bl, 0, sizeof(*bl));
		bl->entity = e;
		e->vtable = bl;
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		bl = rwNewT(EntityExt, 1, 0);
		memset(bl, 0, sizeof(*bl));
		bl->entity = e;
		e->vtable = bl;
	}
}

#endif
