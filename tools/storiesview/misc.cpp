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


#ifdef VCS
struct
{
	int32 worldId;
	int32 iplId;
} links[] = {
#include "vcs_links.inc"
};

void
LinkInstances(void)
{
	int i, n;
	CEntity *e;
	BuildingExt *be;
	BuildingLink *bl;

	// reuse vtable to store world building ids
	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		bl = rwNewT(BuildingLink, 1, 0);
		bl->n = 0;
		bl->insts = nil;
		e->vtable = bl;
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		bl = rwNewT(BuildingLink, 1, 0);
		bl->n = 0;
		bl->insts = nil;
		e->vtable = bl;
	}


	int levid = gLevel->levelid << 16;
	for(i = 0; i < nelem(links); i++){
		if((links[i].worldId & ~0xFFFF) != levid)
			continue;

		be = GetBuildingExt(links[i].worldId&0x7FFF);
		be->iplId = links[i].iplId;
		if((links[i].iplId & ~0xFFFF) == 0)
			e = pBuildingPool->GetSlot(links[i].iplId & 0xFFFF);
		else
			e = pTreadablePool->GetSlot(links[i].iplId & 0xFFFF);
		bl = (BuildingLink*)e->vtable;
		bl->n++;
		bl->insts = rwResizeT(BuildingExt*, bl->insts, bl->n, 0);
		bl->insts[bl->n-1] = be;
	}
}

#else

void
LinkInstances(void)
{
	int i, n;
	CEntity *e;
	BuildingLink *bl;

	// reuse vtable to store world building ids
	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		bl = rwNewT(BuildingLink, 1, 0);
		bl->n = 0;
		bl->insts = nil;
		e->vtable = bl;
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		bl = rwNewT(BuildingLink, 1, 0);
		bl->n = 0;
		bl->insts = nil;
		e->vtable = bl;
	}
}

#endif
