#include "storiesconv.h"

#include "leedsgta.h"
#include "streamworld.h"

char *argv0;
int32 atmOffset;

ResourceImage *gamedata;

const char *pedTypeStrings[] = {
	"PLAYER1",
	"PLAYER2",
	"PLAYER3",
	"PLAYER4",
	"CIVMALE",
	"CIVFEMALE",
	"COP",
	"GANG1",
	"GANG2",
	"GANG3",
	"GANG4",
	"GANG5",
	"GANG6",
	"GANG7",
	"GANG8",
	"GANG9",
	"EMERGENCY",
	"FIREMAN",
	"CRIMINAL",
	"SPECIAL",
	"PROSTITUTE",
};

const char *vehicleTypes[] = {
	"car",
	"boat",
#ifdef VCS
	"jetski",
#endif
	"train",
	"heli",
	"plane",
	"bike",
	"ferry",
};

// from mobile LCS
const char *vehicleClasses_[] = {
	"ignore",	// -1
	"normal",	// 0
	"poorfamily",
	"richfamily",
	"executive",
	"worker",
	"big",
	"taxi",
	"moped",
	"motorbike",
	"leisureboat",
	"workerboat",
	// VCS - made up names, marked with '_'
	"specialboat_",	// jetski and dinghy
	"speedboat_",	// squalo, jetmax, speeder and scarab
};
// so -1 works
const char **vehicleClasses = &vehicleClasses_[1];

void
panic(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void
writeClump(const char *filename, Clump *c)
{
	StreamFile stream;
	if(stream.open(filename, "wb") == nil)
		panic("couldn't open file %s", filename);
	c->streamWrite(&stream);
	stream.close();
}

RslNode *dumpNodeCB(RslNode *frame, void *data)
{
#ifdef LCS
	printf("%08x %p %s %d\n", frame->nodeId, frame->hier, frame->name, frame->hierId);
#else
	printf("%08x %08x %p %s %d\n", frame->nodeId, frame->nodeId2, frame->hier, frame->name, frame->hierId);
#endif
//	printf(" frm: %x %s %x\n", frame->nodeId, frame->name, frame->hierId);
	RslNodeForAllChildren(frame, dumpNodeCB, data);
	return frame;
}

RslMaterial *dumpMaterialCB(RslMaterial *material, void*)
{
	printf("  mat: %d %d %d %d %s %x\n", material->color.red, material->color.green, material->color.blue, material->color.alpha,
		material->texname, material->refCount);
	if(material->matfx){
		RslMatFX *fx = material->matfx;
		printf("   matfx: %d", fx->effectType);
		if(fx->effectType == 2)
			printf("env[%s %f] ", fx->env.texname, fx->env.intensity);
		printf("\n");
	}
	return material;
}

RslElement *dumpElementCB(RslElement *atomic, void*)
{
	printf(" atm: %x %x %x %p\n", atomic->renderCallBack, atomic->modelInfoId, atomic->visIdFlag, atomic->hier);
	RslGeometry *g = atomic->geometry;
	RslGeometryForAllMaterials(g, dumpMaterialCB, NULL);
	return atomic;
}

Clump*
LoadSimple(uint8 *data, const char *name, int nElements)
{
	int i;
	Clump *c;
	Atomic *a;
	Frame *root, *child;
	RslElement **e;
	char *nodename;

	e = (RslElement**)data;
	for(i = 0; i < nElements; i++)
		if(e[i]->object.object.type != 1)
			return nil;

	c = Clump::create();
	root = Frame::create();
	strncpy(gta::getNodeName(root), name, 24);
	c->setFrame(root);
	for(i = 0; i < nElements; i++){
		child = Frame::create();
		nodename = gta::getNodeName(child);
		snprintf(nodename, 24, "%s_L%d", name, i);
		root->addChild(child);
		makeTextures(e[i], NULL);
		a = convertAtomic(e[i]);
		a->setFrame(child);
		c->addAtomic(a);
	}
	return c;
}

Clump*
LoadElementGroup(uint8 *data)
{
	RslElementGroup *eg;
	eg = (RslElementGroup*)data;
	if(eg->object.type != 2)
		return nil;
	RslElementGroupForAllElements(eg, makeTextures, NULL);
//	dumpNodeCB((RslNode*)eg->object.parent, NULL);
	return convertClump(eg);
}

Clump*
LoadVehicle(uint8 *data)
{
	struct VehicleData
	{
		RslElementGroup *elementgroup;
		int32 numExtras;
		RslElement **extras;
		RslMaterial *primaryMaterials[NUMPRIM];
		RslMaterial *secondaryMaterials[NUMSEC];
	};
	int i;
	VehicleData *veh = (VehicleData*)data;
	if(veh->elementgroup->object.type != 2)
		return nil;
	for(i = 0; i < NUMPRIM; i++){
		if(veh->primaryMaterials[i]){
			veh->primaryMaterials[i]->color.red = 0x3C;
			veh->primaryMaterials[i]->color.green = 0xFF;
			veh->primaryMaterials[i]->color.blue = 0;
		}
	}
	for(i = 0; i < NUMSEC; i++){
		if(veh->secondaryMaterials[i]){
			veh->secondaryMaterials[i]->color.red = 0xFF;
			veh->secondaryMaterials[i]->color.green = 0;
			veh->secondaryMaterials[i]->color.blue = 0xAF;
		}
	}
	Clump *rwc = LoadElementGroup((uint8*)veh->elementgroup);
	moveAtomics(rwc->getFrame());
	for(i = 0; i < veh->numExtras; i++){
		makeTextures(veh->extras[i], nil);
		Atomic *a = convertAtomic(veh->extras[i]);
		Frame *f = convertFrame((RslNode*)veh->extras[i]->object.object.parent);
		a->setFrame(f);
		rwc->getFrame()->addChild(f);
		rwc->addAtomic(a);
	}
	return rwc;
}

Clump*
LoadPed(uint8 *data)
{
	struct PedData
	{
		void *colModel;
		RslElementGroup *elementgroup;
	};
	PedData *ped = (PedData*)data;
	if(ped->elementgroup->object.type != 2)
		return nil;
	Clump *rwc = LoadElementGroup((uint8*)ped->elementgroup);
	return rwc;
}

Clump*
LoadAny(sChunkHeader &header, uint8 *data, const char *name)
{
	Clump *c;
	c = LoadSimple(data, "object", header.numFuncs);
	if(c) return c;
	c = LoadElementGroup(data);
	if(c) return c;
	c = LoadVehicle(data);
	if(c) return c;
	c = LoadPed(data);
	if(c) return c;
	return nil;
}

int
computeFlags(int miflags)
{
	int f = 0;
	// 1, 2 are object index
	if(miflags & 4) f |= 1;		// wet road effect
	// 8?
	// 10 is big building flag
	if(miflags & 0x20) f |= 2;	// no fade
	if(miflags & 0x40) f |= 4;	// draw last
	if(miflags & 0x80) f |= 8;	// additive blend
	if(miflags & 0x100) f |= 0x10;	// is subway (also in VC?)
	if(miflags & 0x200) f |= 0x20;	// no lighting
	if(miflags & 0x400) f |= 0x40;	// no depth write
	if(miflags & 0x800) f |= 0x80;	// no shadows
	if(miflags & 0x1000) f |= 0x100;	// no draw distance
	if(miflags & 0x2000) f |= 0x200;	// glass1
	if(miflags & 0x4000) f |= 0x400;	// glass2
	return f;
}

#ifdef LCS
#include "animgroups_lcs.h"

#define GROUPDEFEND };

#define GROUPDEF(name, assocName, blockname, modelIndex) static char *name##names[] = {
#define ANIMDEF(animName, id, flags) animName
ANIMGROUPS
#undef ANIMDEF
#undef GROUPDEF

#define GROUPDEF(name, assocName, blockName, modelIndex) static AnimAssocInfo name##info[] = {
#define ANIMDEF(animName, id, flags) { id, flags },
ANIMGROUPS
#undef ANIMDEF
#undef GROUPDEF

#undef GROUPDEFEND

AnimAssocDefinition assocDefs[] = {
#define GROUPDEF(name, assocName, blockName, modelIndex) { assocName, blockName, modelIndex, nelem(name##names), name##names, name##info },
#define GROUPDEFEND
#define ANIMDEF(name, id, flags)
ANIMGROUPS
#undef ANIMDEF
#undef GROUPDEF
#undef GROUPDEFEND
};

#endif

const char*
getAnimBlockName(int32 i)
{
	if(i < 0)
		return "null";
	else
		return gamedata->animManagerInst->m_aAnimBlocks[i].name;
}

const char*
getAnimGroupName(int32 id)
{
#ifdef LCS
	return assocDefs[id].name;
#else
	return gamedata->animManagerInst->assocGroups[id].groupname;
#endif
}

void
writeAllModelInfo(void)
{
	int i, j;
	CBaseModelInfo *mi;
	CSimpleModelInfo *smi;
	CTimeModelInfo *tmi;
	CElementGroupModelInfo *emi;
	CWeaponModelInfo *wmi;
	CPedModelInfo *pmi;
	CVehicleModelInfo *vmi;
	char tmpname[50];
	const char *name;
	enum SectionType {
		SECTION_NONE,
		SECTION_OBJS,
		SECTION_TOBJ,
		SECTION_HIER,
		SECTION_PEDS,
		SECTION_CARS,
		SECTION_WEAP
	};
	SectionType currentSection = SECTION_NONE;
	for(i = 0; i < gamedata->numModelInfos; i++){
		mi = gamedata->modelInfoPtrs[i];
		if(mi == nil)
			continue;
		smi = (CSimpleModelInfo*)mi;
		tmi = (CTimeModelInfo*)mi;
		emi = (CElementGroupModelInfo*)mi;
		wmi = (CWeaponModelInfo*)mi;
		pmi = (CPedModelInfo*)mi;
		vmi = (CVehicleModelInfo*)mi;

		if(mi->type == MODELINFO_SIMPLE && currentSection != SECTION_OBJS){
			if(currentSection != SECTION_NONE) printf("end\n");
			currentSection = SECTION_OBJS;
			printf("objs\n");
		}else if(mi->type == MODELINFO_TIME && currentSection != SECTION_TOBJ){
			if(currentSection != SECTION_NONE) printf("end\n");
			currentSection = SECTION_TOBJ;
			printf("tobj\n");
		}else if(mi->type == MODELINFO_ELEMENTGROUP && currentSection != SECTION_HIER){
			if(currentSection != SECTION_NONE) printf("end\n");
			currentSection = SECTION_HIER;
			printf("hier\n");
		}else if(mi->type == MODELINFO_PED && currentSection != SECTION_PEDS){
			if(currentSection != SECTION_NONE) printf("end\n");
			currentSection = SECTION_PEDS;
			printf("peds\n");
		}else if(mi->type == MODELINFO_VEHICLE && currentSection != SECTION_CARS){
			if(currentSection != SECTION_NONE) printf("end\n");
			currentSection = SECTION_CARS;
			printf("cars\n");
		}else if(mi->type == MODELINFO_WEAPON && currentSection != SECTION_WEAP){
			if(currentSection != SECTION_NONE) printf("end\n");
			currentSection = SECTION_WEAP;
			printf("weap\n");
		}

	//if(mi->type != MODELINFO_VEHICLE)
	//	continue;

		printf("%d", i);

		name = lookupHashKey(mi->hashKey);
		if(mi->hashKey == 0)
			name = "null";
		else if(name == nil){
			snprintf(tmpname, 50, "hash:%x", mi->hashKey);
			name = tmpname;
		}
		printf(", %s", name);

		if(mi->txdSlot >= 0)
			name = gamedata->texlistPool->items[mi->txdSlot].name;
		else
			name = "null";
		printf(", %s", name);

		if(mi->type == MODELINFO_SIMPLE || mi->type == MODELINFO_TIME){
			printf(", %d", smi->numObjects);
			for(j = 0; j < smi->numObjects; j++)
				printf(", %.0f", smi->drawDistances[j]);
			printf(", %d", computeFlags(smi->flags));
			if(mi->type == MODELINFO_TIME)
				printf(", %d, %d", tmi->timeOn, tmi->timeOff);
		}

		if(mi->type == MODELINFO_ELEMENTGROUP)
			printf(", %s", getAnimBlockName(emi->animFileIndex));

		if(mi->type == MODELINFO_PED){
			printf(", %s", pedTypeStrings[pmi->pedType]);
			printf(", %s", gamedata->pedStats[pmi->pedStatType]->name);
			printf(", %s", getAnimGroupName(pmi->animGroup));
			printf(", %x", pmi->carsDriveMask);
			printf(", %s", getAnimBlockName(pmi->animFileIndex));
			printf(", %d,%d", pmi->radio1, pmi->radio2);
//#ifdef VCS
//			printf(", %s(%d), %x, %x", pmi->someName, strlen(pmi->someName), pmi->unknown1, pmi->unknown2);
//#endif
		}

		if(mi->type == MODELINFO_VEHICLE){
			printf(",\t%s", vehicleTypes[vmi->m_vehicleType]);
			printf(",\tHANDLING_TODO");
			printf(",\t%.8s", vmi->m_gameName);
			printf(",\t%s", getAnimBlockName(vmi->m_animFileIndex));
			printf(",\t%s", vehicleClasses[vmi->m_vehicleClass]);
			printf(",\t%d,\t%d,\t%x", vmi->m_frequency, vmi->m_level, vmi->m_compRules);
			if(vmi->m_vehicleType == VEHICLETYPE_CAR){
				printf(",\t%d,\t%.3f", vmi->m_wheelId, vmi->m_wheelScale);
#ifdef VCS
				printf(",%.3f", vmi->m_wheelScaleRear);
#endif
			}else if(vmi->m_vehicleType == VEHICLETYPE_BIKE){
				printf(",\t%d,\t%.3f", (int)vmi->m_bikeSteerAngle, vmi->m_wheelScale);
#ifdef VCS
				printf(",%.3f", vmi->m_wheelScaleRear);
#endif
			}else if(vmi->m_vehicleType == VEHICLETYPE_PLANE){
				printf(",\t%d", vmi->m_wheelId);
			}
			printf(", %.2f", vmi->m_normalSplay);
		}

		if(mi->type == MODELINFO_WEAPON)
			printf(", %s, 1, %.0f", getAnimBlockName(wmi->animFileIndex), wmi->drawDistances[0]);

		printf("\n");
	}
	if(currentSection != SECTION_NONE) printf("end\n");
}

void
dumpPedStats(void)
{
	int i;
	PedStats **pedstats = gamedata->pedStats;
	for(i = 0; i < 42; i++)	// LCS
		printf("%s\t%.1f\t%.1f\t%d\t%d\t%d\t%d\t%.1f\t%.1f\t%d\n", pedstats[i]->name,
			pedstats[i]->fleeDistance, pedstats[i]->headingChangeRate,
			pedstats[i]->fear, pedstats[i]->temper,
			pedstats[i]->lawfulness, pedstats[i]->sexiness,
			pedstats[i]->attackStrength, pedstats[i]->defendWeakness,
			pedstats[i]->flags);
}

#ifdef VCS

void
dumpVCSObjects(void)
{
	int i;
	CBaseModelInfo *bmi;
	const char *texname;
	for(i = 0; i < gamedata->numModelInfos; i++){
		bmi = gamedata->modelInfoPtrs[i];
		if(bmi == nil)
			continue;

		CSimpleModelInfo *smi = (CSimpleModelInfo*)bmi;
		if(bmi->type == MODELINFO_SIMPLE || bmi->type == MODELINFO_TIME)
			if(smi->relatedObject && smi->relatedObject->relatedObject == nil)
				smi->relatedObject->relatedObject = smi;

		bmi->name = lookupHashKey(bmi->hashKey);
		if(bmi->hashKey == 0)
			bmi->name = "NULL";

//		texname = nil;
//		if(bmi->txdSlot >= 0)
//			texname = res->texlistPool->items[bmi->txdSlot].name;

//		try txd name
//		if(bmi->name == nil && texname){
//			uint32 txdhash = GetUppercaseKey(texname, strlen(texname));
//			if(txdhash == bmi->hashKey)
//				bmi->name = texname;
//		}
	}

//	char tmpbuffer[30];
	for(i = 0; i < gamedata->numModelInfos; i++){
		bmi = gamedata->modelInfoPtrs[i];
		if(bmi == nil)
			continue;
		CSimpleModelInfo *smi = (CSimpleModelInfo*)bmi;

//		const char *relname = nil;
//		if(bmi->name == nil && (bmi->type == MODELINFO_SIMPLE || bmi->type == MODELINFO_TIME)){
//			if(smi->relatedObject)
//				relname = smi->relatedObject->name;
//			if(relname){
//				if((smi->flags & 0x10) == 0){
//					//fprintf(stderr, "%d has LOD %s\n", i, relname);
//				}else{
//					strcpy(tmpbuffer, relname);
//					tmpbuffer[0] = 'L';
//					tmpbuffer[1] = 'O';
//					tmpbuffer[2] = 'D';
//					uint32 hash = GetUppercaseKey(tmpbuffer, strlen(tmpbuffer));
//					if(hash == bmi->hashKey)
//						bmi->name = strdup(tmpbuffer);
//				}
//			}
//		}

		texname = nil;
		if(bmi->txdSlot >= 0)
			texname = gamedata->texlistPool->items[bmi->txdSlot].name;
		else
			texname = "NULL";

		if(bmi->name)
			printf("%d %s %s ", i, bmi->name, texname);
		else
			printf("%d hash:%x %s ", i, bmi->hashKey, texname);

		if(bmi->type == MODELINFO_SIMPLE || bmi->type == MODELINFO_TIME){
			printf("%x %.0f ", smi->flags, smi->drawDistances[0]);
			if(smi->flags & 0x10)
				printf("BIGBUILD ");
			if(smi->relatedObject)
				if(smi->relatedObject->name)
					printf("REL:%s ", smi->relatedObject->name);
				else
					printf("REL:%x ", smi->relatedObject->hashKey);
		}

		printf("\n");
	}
}

#endif

void
extractMarkers(void)
{
	int i;
	char tempname[128];
	RslElementGroup **C3dMarkers__m_pRslElementGroupArray;
	C3dMarkers__m_pRslElementGroupArray = gamedata->markers;
	for(i = 0; i < 32; i++){
		if(C3dMarkers__m_pRslElementGroupArray[i]){
			snprintf(tempname, 128, "marker%d.dff", i);
			printf("extracting %s\n", tempname);
			Clump *c = convertClump(C3dMarkers__m_pRslElementGroupArray[i]);
			if(c == nil)
				panic("couldn't convert marker %d", i);
			writeClump(tempname, c);
			c->destroy();
		}
	}
}

void
dumpAnimations(void)
{
	CAnimManager *mgr = gamedata->animManagerInst;
	CAnimBlock *block;
	int i, j;
	for(i = 0; i < mgr->m_numAnimBlocks; i++){
		block = &mgr->m_aAnimBlocks[i];
		printf("%s\n", block->name);
		for(j = 0; j < block->numAnims; j++)
			printf("  %s\n", mgr->m_aAnimations[j + block->animBase].name);
	}
}

void
dumpVehicleData(void)
{
#ifdef LCS
	CVehicleModelInfo__inst *vmi = gamedata->vehicleModelInfo_Info;
	int i;
	for(i = 0; i < 256; i++)
		printf("%d %d %d\n",
			vmi->m_vehicleColourTable[i].red,
			vmi->m_vehicleColourTable[i].green,
			vmi->m_vehicleColourTable[i].blue);
#else
	int i;
	// 0-127 are ped colours
	for(i = 128; i < 256; i++)
		printf("%d %d %d\n",
			gamedata->colourTable[i*3],
			gamedata->colourTable[i*3+1],
			gamedata->colourTable[i*3+2]);
#endif
}

#ifdef VCS
void
dumpVCSanimData(CAnimManager *mgr)
{
	int i, j;
	CAnimAssocGroup_vcs *a = mgr->assocGroups;
	for(i = 0; i < mgr->numAssocGroups; i++){
		printf("%s %s %x %x\n", a->groupname, a->blockname, a->animBase, a->numAnims);
		for(j = 0; j < a->numAnims; j++)
			printf("  %s\n", mgr->associations[j+a->animBase].name);
		a++;
	}
}
#endif

void
writeWaterpro(void)
{
	FILE *f = fopen("waterpro.dat", "wb");
	if(f == nil)
		panic("can't open waterpro.dat");
	CWaterLevel *wl = gamedata->waterLevelInst;

	fwrite(&wl->ms_nNoOfWaterLevels, 1, sizeof(wl->ms_nNoOfWaterLevels), f);
	fwrite(wl->ms_aWaterZs, 1, sizeof(float)*48, f);
	fwrite(wl->ms_aWaterRects, 1, sizeof(CRect)*48, f);
	fwrite(&wl->aWaterBlockList, 1, sizeof(wl->aWaterBlockList), f);
	fwrite(&wl->aWaterFineBlockList, 1, sizeof(wl->aWaterFineBlockList), f);

	fclose(f);
}

void
extractResource(void)
{
#ifdef VCS
//	dumpVCSanimData();
#else
//	dumpAnimations();
#endif
//	dumpPedStats();
//	writeAllModelInfo();
//	dumpVehicleData();
//	extractMarkers();
//	writeWaterpro();
}

char *arg1;

void
dumpLevel(sLevelChunk *level)
{
	rw::StreamFile stream;

	int len = strlen(arg1)+1;
	char filename[1024];
	strncpy(filename, arg1, len);
	filename[len-3] = 'i';
	filename[len-2] = 'm';
	filename[len-1] = 'g';
	filename[len] = '\0';
	if(stream.open(filename, "rb") == nil)
		panic("couldn't open %s", filename);
	filename[len-4] = '\\';
	filename[len-3] = '\0';

	char name[1024];
	uint8 *data;
	StreamFile outf;
	sChunkHeader *h;
	uint32 i = 0;
	for(h = level->sectorRows[0].header; h->ident == WRLD_IDENT; h++){
		sprintf(name, "world%04d.wrld", i++);
		strcat(filename, name);
		if(outf.open(filename, "wb") == 0)
			panic("couldn't open %s", filename);
		data = new uint8[h->fileSize];
		memcpy(data, h, sizeof(sChunkHeader));
		stream.seek(h->globalTab, 0);	// this is the offset for some reason
		stream.read(data+0x20, h->fileSize-sizeof(sChunkHeader));
		outf.write(data, h->fileSize);
		outf.close();
		filename[len-3] = '\0';
	}
/*
	// radar textures
	h = world->textures;
	for(i = 0; i < world->numTextures; i++){
		sprintf(name, "txd%04d.chk", i);
		strcat(filename, name);
		if(outf.open(filename, "wb") == nil)
			panic("couldn't open %s", filename);
		data = new uint8[h->fileEnd];
		memcpy(data, h, 0x20);
		stream.seek(h->root, 0);
		stream.read(data+0x20, h->fileEnd-0x20);
		outf.write(data, h->fileEnd);
		outf.close();
		filename[len-3] = '\0';
		h++;
	}
*/
	stream.close();
}

void
dumpSector(Sector *sector)
{
	CVector min = { 10000000.0f, 10000000.0f, 10000000.0f };
	CVector max = { -10000000.0f, -10000000.0f, -10000000.0f };
	sGeomInstance *inst;

	printf("%x %s %p %p %p %p %p %p %p %p\n", sector->unk1, arg1,
		sector->sectionA, sector->sectionB, sector->sectionC, sector->sectionD,
		sector->sectionE, sector->sectionF, sector->sectionG, sector->sectionEnd);
	fflush(stdout);
	// what is this?
//	if((sector->unk1 & 2) == 0)	// this is wrong
		return;

	for(inst = sector->sectionA; inst != sector->sectionEnd; inst++){
		// these are in worldspace, and per object not per model placement
		float x = halfFloatToFloat(inst->bound[0]);
		float y = halfFloatToFloat(inst->bound[1]);
		float z = halfFloatToFloat(inst->bound[2]);
		float r = halfFloatToFloat(inst->bound[3]);


//		printf("%d %f %f %f %f\n", inst->id & 0x7FFF, x, y, z, r);
		RslV3 v = inst->matrix.pos;
//		printf("%d %8.2f %8.2f %8.2f %8.2f\n", inst->id & 0x7FFF, inst->matrix.right.x, inst->matrix.up.x, inst->matrix.at.x, inst->matrix.pos.x);
//		printf("%d %8.2f %8.2f %8.2f %8.2f\n", inst->id & 0x7FFF, inst->matrix.right.y, inst->matrix.up.y, inst->matrix.at.y, inst->matrix.pos.y);
//		printf("%d %8.2f %8.2f %8.2f %8.2f\n", inst->id & 0x7FFF, inst->matrix.right.z, inst->matrix.up.z, inst->matrix.at.z, inst->matrix.pos.z);
//		printf("%d %8.2f %8.2f %8.2f %8.2f\n", inst->id & 0x7FFF, inst->matrix.rightw, inst->matrix.upw, inst->matrix.atw, inst->matrix.posw);
		// ignore placement, just get center
		v.x = x;
		v.y = y;
		v.z = z;
		if(v.x < min.x) min.x = v.x;
		if(v.y < min.y) min.y = v.y;
		if(v.z < min.z) min.z = v.z;
		if(v.x > max.x) max.x = v.x;
		if(v.y > max.y) max.y = v.y;
		if(v.z > max.z) max.z = v.z;
//		printf("point pos:[%f, %f, %f] scale:[0.5, 0.5, 0.5]\n", inst->matrix.pos.x, inst->matrix.pos.y, inst->matrix.pos.z);
//		printf("%d %d\n", inst->id, inst->resId);
	}
	CVector pos = { (min.x+max.x)/2.0f, (min.y+max.y)/2.0f, min.z };
	float w = max.x-min.x;
	float l = max.y-min.y;
	float h = max.z-min.z;
//	printf("box width:%f length:%f height:%f pos:[%f,%f,%f] name:\"%s\"\n", w, l, h, pos.x, pos.y, pos.z, arg1);
}

void
dumpVifCmd(uint32 *data, int32 size)
{
	int32 i;
	uint32 vifcode;
	uint32 op, num, imm;
	uint32 unpacksz;
	for(i = 0; i*4 < size;){
		vifcode = data[i++];
		op = (vifcode>>24) & 0xFF;
		num = (vifcode>>16) & 0xFF;
		imm = vifcode & 0xFFFF;

		printf(" %08X\n", vifcode);
		switch(op & 0x7F){
		case 0x20:	// STMASK
			i++;
			break;
		case 0x30:	// STROW
			i += 4;
			break;
		case 0x31:	// STCOL
			i += 4;
			break;
		case 0x4A:	// MPG
			i += num*2;
			break;
		case 0x50:	// DIRECT
			i += imm*4;
			break;
		case 0x51:	// DIRECTHL
			i += imm*4;
			break;
		unpack:
			unpacksz = 0;
			switch(op&0xF){
			case 0x0:	// S-32
				unpacksz = 4;
				break;
			case 0x1:	// S-16
				unpacksz = 2;
				break;
			case 0x2:	// S-8
				unpacksz = 1;
				break;

			case 0x4:	// V2-32
				unpacksz = 2*4;
				break;
			case 0x5:	// V2-16
				unpacksz = 2*2;
				break;
			case 0x6:	// V2-8
				unpacksz = 2*1;
				break;

			case 0x8:	// V3-32
				unpacksz = 3*4;
				break;
			case 0x9:	// V3-16
				unpacksz = 3*2;
				break;
			case 0xA:	// V3-8
				unpacksz = 3*1;
				break;

			case 0xC:	// V4-32
				unpacksz = 4*4;
				break;
			case 0xD:	// V4-16
				unpacksz = 4*2;
				break;
			case 0xE:	// V4-8
				unpacksz = 4*1;
				break;

			case 0xF:	// V4-5
				unpacksz = 2;
				break;
			}
			// unpacksz in bytes
			unpacksz *= num;
			unpacksz = (unpacksz+3)>>2;
			i += unpacksz;
			break;
		default:
			if((op & 0x60) == 0x60)
				goto unpack;
			break;
		}
	}
}

void
dumpBuildingGeom(sBuildingGeometry *building)
{
	uint8 *vifData;
	sClippableBuildingMesh *mesh;
	int i;

	printf("building: %x %x\n", building->numMeshes, building->size);
	mesh = (sClippableBuildingMesh*)(building+1);
	vifData = (uint8*)(building+1) + building->size;
	for(i = 0; i < building->numMeshes; i++){
		printf("%x %x\n", mesh->texID, mesh->packetSize);
		dumpVifCmd((uint32*)vifData, mesh->packetSize);
		vifData += mesh->packetSize;
		mesh++;
	}
}

void
usage(void)
{
	fprintf(stderr, "%s [-v version] [-x] [-s] input [output.{txd|dff}]\n", argv0);
	fprintf(stderr, "\t-v RW version, e.g. 33004 for 3.3.0.4\n");
	fprintf(stderr, "\t-x extract textures to tga\n");
	fprintf(stderr, "\t-s don't unswizzle textures\n");
	fprintf(stderr, "\t-m dump data to find missing names in VCS\n");
	exit(1);
}

enum MdlType {
	MDL_SIMPLE,
	MDL_ELEMENTGROUP,
	MDL_VEHICLE,
	MDL_PED,
	MDL_ANY
};

int
main(int argc, char *argv[])
{
	rw::Engine::init();
	gta::attachPlugins();
	rw::Engine::open();
	rw::Engine::start(nil);

	atmOffset = Atomic::registerPlugin(sizeof(void*), 0x1000000, NULL, NULL, NULL);
	rw::version = 0x34003;
	rw::platform = PLATFORM_D3D8;

	assert(sizeof(void*) == 4);
	int extract = 0;
	int missing = 0;

	int mdltype = MDL_ANY;

	ARGBEGIN{
	case 'v':
		sscanf(EARGF(usage()), "%x", &rw::version);
		break;
	case 's':
		unswizzle = 0;
		break;
	case 'x':
		extract++;
		break;
	case 'm':
		missing++;
		break;
	default:
		usage();
	}ARGEND;

	if(argc < 1)
		usage();

	RslElementGroup *clump = NULL;
	RslElement *atomic = NULL;
	RslTexList *txd = NULL;
	Clump *rwc;

	StreamFile stream;
	if(stream.open(argv[0], "rb") == nil)
		panic("couldn't open %s", argv[0]);
	arg1 = argv[0];

	uint32 ident = stream.readU32();
	stream.seek(0, 0);

	if(ident == ID_TEXDICTIONARY){
		findChunk(&stream, ID_TEXDICTIONARY, NULL, NULL);
		txd = RslTexListStreamRead(&stream);
		stream.close();
		assert(txd);
		goto writeTxd;
	}
	if(ident == ID_CLUMP){
		findChunk(&stream, ID_CLUMP, NULL, NULL);
		clump = RslElementGroupStreamRead(&stream);
		stream.close();
		assert(clump);
		rwc = convertClump(clump);
		goto writeDff;
	}

	assert(sizeof(sChunkHeader) == 0x20);

	sChunkHeader header;
	stream.read(&header, sizeof(sChunkHeader));
	uint8 *data = (uint8*)malloc(header.fileSize-sizeof(sChunkHeader));
	stream.read(data, header.fileSize-sizeof(sChunkHeader));
	stream.close();
	cReloctableChunk(header.ident, header.shrink).Fixup(header, data);

	bool32 largefile;
	largefile = header.dataSize > 4*1024*1024;

	if(header.ident == WRLD_IDENT && largefile){	// hack
		dumpLevel((sLevelChunk*)data);

	//	dumpBuildingGeom(((sLevelChunk*)data)->resourceTable[0xbe1].geometry);

	}else if(header.ident == WRLD_IDENT){	// sector
		dumpSector((Sector*)data);
#if 0
		sector = (Sector*)rslstr->data;
		fprintf(stderr, "%d\n",sector->unk1);
		//printf("resources\n");
		//for(uint32 i = 0; i < sector->numResources; i++){
		//	OverlayResource *r = &sector->resources[i];
		//	printf(" %d %p\n", r->id, r->raw);
		//}
		//printf("placement\n");
		if(sector->unk1 == 0)
			return 0;
		Placement *p;
		//for(p = sector->sectionA; p < sector->sectionEnd; p++){
		//	printf(" %d, %d, %f %f %f\n", p->id &0x7FFF, p->resId, p->matrix[12], p->matrix[13], p->matrix[14]);
		//}
		for(p = sector->sectionA; p < sector->sectionEnd; p++)
			printf("%f %f %f\n", p->matrix[12], p->matrix[13], p->matrix[14]);
#endif
	}else if(header.ident == MDL_IDENT){
		switch(mdltype){
		case MDL_SIMPLE:
			rwc = LoadSimple(data, "object", header.numFuncs);
			break;
		case MDL_ELEMENTGROUP:
			rwc = LoadElementGroup(data);
			break;
		case MDL_VEHICLE:
			rwc = LoadVehicle(data);
			break;
		case MDL_PED:
			rwc = LoadPed(data);
			break;
		case MDL_ANY:
			rwc = LoadAny(header, data, "object");
			break;
		default:
			panic("unknown model type");
		}
	writeDff:
		if(rwc == nil)
			panic("couldn't load object");
		if(stream.open(argc > 1 ? argv[1] : "out.dff", "wb") == nil)
			panic("couldn't open file %s", argc > 1 ? argv[1] : "out.dff");
		rwc->streamWrite(&stream);
		stream.close();		
	}else if(header.ident == TEX_IDENT){
		txd = (RslTexList*)data;
	writeTxd:
		if(extract)
			RslTexListForAllTextures(txd, dumpTextureCB, NULL);
		TexDictionary *rwtxd = convertTXD(txd);
		if(stream.open(argc > 1 ? argv[1] : "out.txd", "wb") == nil)
			panic("couldn't open file %s", argc > 1 ? argv[1] : "out.txd");
		rwtxd->streamWrite(&stream);
		stream.close();
	}else if(header.ident == GTAG_IDENT){
		gamedata = (ResourceImage*)data;
#ifdef VCS
		if(missing)
			dumpVCSObjects();
		else
#endif
			extractResource();
	}else
		printf("unknown file type %X\n", header.ident);

	return 0;
}
