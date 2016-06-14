#include "III.h"

char*
CFileLoader::LoadLine(FILE *f)
{
	static char linebuf[1024];
again:
	if(fgets(linebuf, 1024, f) == NULL)
		return NULL;
	// remove leading whitespace
	char *s = skipWhite(linebuf);
	// remove trailing whitespace
	int end = strlen(s);
	char c;
	while(c = s[--end], isspace(c))
		s[end] = '\0';
	// convert ',' -> ' '
	for(char *t = s; *t; t++)
		if(*t == ',') *t = ' ';
	// don't return empty lines
	if(*s == '\0')
		goto again;
	return s;
}

void
CFileLoader::LoadLevel(char *filename)
{
	FILE *file;
	char *line;
	if(file = fopen_ci(filename, "rb"), file == NULL)
		return;
	rw::TexDictionary *savedTxd = rw::currentTexDictionary;
	if(rw::currentTexDictionary == NULL)
		rw::currentTexDictionary = rw::TexDictionary::create();
	while(line = CFileLoader::LoadLine(file)){
		if(line[0] == '#')
			continue;
		if(strncmp(line, "EXIT", 4) == 0)
			break;
		if(strncmp(line, "IMAGEPATH", 9) == 0){
			printf("imagepath ");
			puts(line+19);
		}
		if(strncmp(line, "TEXDICTION", 10) == 0){
			printf("txd ");
			puts(line+11);
		}
		if(strncmp(line, "COLFILE", 7) == 0){
			int currlevel = CGame::currLevel;
			sscanf(line+8, "%d", &CGame::currLevel);
			CFileLoader::LoadCollisionFile(line+10);
			CGame::currLevel = currlevel;
		}
		if(strncmp(line, "MODELFILE", 9) == 0){
			printf("model ");
			puts(line+10);
		}
		if(strncmp(line, "HIERFILE", 8) == 0){
			printf("hier ");
			puts(line+9);
		}
		if(strncmp(line, "IDE", 3) == 0)
			CFileLoader::LoadObjectTypes(line+4);
		if(strncmp(line, "IPL", 3) == 0)
			CFileLoader::LoadScene(line+4);
		if(strncmp(line, "MAPZONE", 7) == 0)
			CFileLoader::LoadMapZone(line+8);
		if(strncmp(line, "SPLASH", 6) == 0){
			printf("splash ");
			puts(line+7);
		}
		if(strncmp(line, "CDIMAGE", 7) == 0)
			CdStream::addImage(line+8);
	}
	fclose(file);
	rw::currentTexDictionary = savedTxd;
}

DatDesc CFileLoader::zoneDesc[] = {
	{ "end", CFileLoader::LoadNothing },
	{ "zone", CFileLoader::LoadMapZone },
	{ "", NULL }
};

DatDesc CFileLoader::ideDesc[] = {
	{ "end", CFileLoader::LoadNothing },
	{ "objs", CFileLoader::LoadObject },
	{ "tobj", CFileLoader::LoadTimeObject },
	{ "hier", CFileLoader::LoadClumpObject },
	{ "cars", CFileLoader::LoadVehicleObject },
	{ "peds", CFileLoader::LoadPedObject },
	{ "path", CFileLoader::LoadPathLine },
	{ "2dfx", CFileLoader::Load2dEffect },
	{ "", NULL }
};

DatDesc CFileLoader::iplDesc[] = {
	{ "end", CFileLoader::LoadNothing },
	{ "inst", CFileLoader::LoadObjectInstance },
	{ "zone", CFileLoader::LoadZone },
	{ "cull", CFileLoader::LoadCullZone },
	{ "pick", CFileLoader::LoadNothing },
	{ "path", CFileLoader::LoadNothing },
	{ "", NULL }
};

void
CFileLoader::LoadDataFile(char *filename, DatDesc *desc)
{
	FILE *file;
	char *line;
	void (*handler)(char*) = NULL;

	if(file = fopen_ci(filename, "rb"), file == NULL)
		return;
	while(line = CFileLoader::LoadLine(file)){
		if(line[0] == '#')
			continue;
		void *tmp = DatDesc::get(desc, line);
		if(tmp){
			handler = (void(*)(char*))tmp;
			continue;
		}
		if(handler)
			handler(line);
	}
	fclose(file);
}

//
// IDE
//

void
CFileLoader::LoadObject(char *line)
{
	int id;
	char model[24], txd[24];
	int numObjs;
	float dist[3];
	int flags;
	sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs);
	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d",
		       &id, model, txd, &numObjs, dist, &flags);
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d",
		       &id, model, txd, &numObjs, dist, dist+1, &flags);
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d",
		       &id, model, txd, &numObjs, dist, dist+1, dist+2, &flags);
		break;
	}
	CSimpleModelInfo *modelinfo = CModelInfo::AddSimpleModel(id);
	strncpy(modelinfo->name, model, 24);
	modelinfo->SetTexDictionary(txd);
	modelinfo->numAtomics = numObjs;
	for(int i = 0; i < numObjs; i++)
		modelinfo->lodDistances[i] = dist[i];
	modelinfo->flags = flags;

	//printf("objs | %d %s %s %d ", id, model, txd, numObjs);
	//for(int i = 0; i < numObjs; i++)
	//	printf("%f ", dist[i]);
	//printf("%d\n", flags);
}

void
CFileLoader::LoadTimeObject(char *line)
{
	int id;
	char model[24], txd[24];
	int numObjs;
	float dist[3];
	int flags;
	int timeon, timeoff;
	sscanf(line, "%d %s %s %d", &id, model, txd, &numObjs);
	switch(numObjs){
	case 1:
		sscanf(line, "%d %s %s %d %f %d %d %d",
		       &id, model, txd, &numObjs, dist, &flags,
		       &timeon, &timeoff);
		break;
	case 2:
		sscanf(line, "%d %s %s %d %f %f %d %d %d",
		       &id, model, txd, &numObjs, dist, dist+1, &flags,
		       &timeon, &timeoff);
		break;
	case 3:
		sscanf(line, "%d %s %s %d %f %f %f %d %d %d",
		       &id, model, txd, &numObjs, dist, dist+1, dist+2, &flags,
		       &timeon, &timeoff);
		break;
	}
	CTimeModelInfo *modelinfo = CModelInfo::AddTimeModel(id);
	strncpy(modelinfo->name, model, 24);
	modelinfo->SetTexDictionary(txd);
	modelinfo->numAtomics = numObjs;
	for(int i = 0; i < numObjs; i++)
		modelinfo->lodDistances[i] = dist[i];
	modelinfo->flags = flags;
	modelinfo->timeOn = timeon;
	modelinfo->timeOff = timeoff;
	modelinfo->otherTimeModelID = -1;	// TODO

	//printf("tobj | %d %s %s %d ", id, model, txd, numObjs);
	//for(int i = 0; i < numObjs; i++)
	//	printf("%f ", dist[i]);
	//printf("%d %d %d\n", flags, timeon, timeoff);
}

void
CFileLoader::LoadClumpObject(char *line)
{
	int id;
	char model[24], txd[24];
	sscanf(line, "%d %s %s", &id, model, txd);
	//printf("hier | %d %s %s\n", id, model, txd);

	CClumpModelInfo *modelinfo = CModelInfo::AddClumpModel(id);
	strncpy(modelinfo->name, model, 24);
	modelinfo->SetTexDictionary(txd);
	modelinfo->colModel = NULL;	// TODO
	modelinfo->freeCol = false;
}

void
CFileLoader::LoadVehicleObject(char *line)
{
	static StrAssoc types[] = {
		{ "car",   CVehicleModelInfo::Car },
		{ "boat",  CVehicleModelInfo::Boat },
		{ "train", CVehicleModelInfo::Train },
		{ "heli",  CVehicleModelInfo::Heli },
		{ "plane", CVehicleModelInfo::Plane },
		{ "bike",  CVehicleModelInfo::Bike },
		{ "", 0 }
	};
	static StrAssoc classes[] = {
		{ "poorfamily", CVehicleModelInfo::Poorfamily },
		{ "richfamily", CVehicleModelInfo::Richfamily },
		{ "executive",  CVehicleModelInfo::Executive },
		{ "worker",     CVehicleModelInfo::Worker },
		{ "special",    CVehicleModelInfo::Special },
		{ "big",        CVehicleModelInfo::Big },
		{ "taxi",       CVehicleModelInfo::Taxi },
		{ "", 0 }
	};
	int id;
	char model[24], txd[24], vehType[8], handling[16], gameName[32], vehClass[12];
	int frq, lvl, compRules, secondId;
	float wheelScale;
	sscanf(line, "%d %s %s %s %s %s %s %d %d %x %d %f",
	       &id, model, txd, vehType, handling, gameName, vehClass,
	       &frq, &lvl, &compRules, &secondId, &wheelScale);

	CVehicleModelInfo *modelinfo = CModelInfo::AddVehicleModel(id);
	strncpy(modelinfo->name, model, 24);
	modelinfo->SetTexDictionary(txd);
	// TODO frq
	modelinfo->handlingId = CHandlingData::GetHandlingData(handling);
	modelinfo->vehicleType = StrAssoc::get(types, vehType);
	modelinfo->vehicleClass = StrAssoc::get(classes, vehClass);
	for(char *s = gameName; *s; s++)
		if(*s == '_') *s = ' ';
	strncpy(modelinfo->gameName, gameName, 32);
	modelinfo->lvl = lvl;
	modelinfo->compRules = compRules;
	if(modelinfo->vehicleType == CVehicleModelInfo::Car ||
	   modelinfo->vehicleType == CVehicleModelInfo::Plane){
		modelinfo->extraModelIndex = secondId;
		modelinfo->wheelScale = wheelScale;
	}
}

void
CFileLoader::LoadPedObject(char *line)
{
	int id;
	char model[24], txd[24], pedType[24], pedStats[24], animGroup[24];
	int carsCanDrive;
	sscanf(line, "%d %s %s %s %s %s %x",
	       &id, model, txd, pedType, pedStats, animGroup, &carsCanDrive);
	//printf("peds | %d %s %s %s %s %s %x\n", id, model, txd, pedType, pedStats, animGroup, carsCanDrive);

	CPedModelInfo *modelinfo = CModelInfo::AddPedModel(id);
	strncpy(modelinfo->name, model, 24);
	modelinfo->SetTexDictionary(txd);
	modelinfo->colModel = NULL;	// TODO
	modelinfo->freeCol = false;
	modelinfo->pedType = CPedType::FindPedType(pedType);
	modelinfo->pedStats = CPedStats::GetPedStatType(pedStats);
	for(int i = 0; i < CAnimManager::ms_numAnimAssocDefinitions; i++){
		char *grpname = CAnimManager::GetAnimGroupName(i);
		if(strcmp(grpname, animGroup) == 0){
			modelinfo->animGroup = i;
			goto animfound;
		}
	}
	modelinfo->animGroup = CAnimManager::ms_numAnimAssocDefinitions;	// invalid
animfound:
	modelinfo->carsCanDrive = carsCanDrive;
}

// used by the path functions below
static int pathId, pathItr;

int
CFileLoader::LoadPathHeader(char *line)
{
	char type[20], model[24];
	sscanf(line, "%s %d %s", type, &pathId, model);
	if(strcmp(type, "ped") == 0)
		return 1;
	if(strcmp(type, "car") == 0)
		return 2;
	return -1;
}

void
CFileLoader::LoadPedPathNode(char *line, int id, int i)
{
	int type, next, cross;
	float x, y, z, width;
	sscanf(line, "%d %d %d %f %f %f %f",
		&type, &next, &cross, &x, &y, &z, &width);
	CPathFind::StoreNodeInfoPed(id, i, type, next, (short)x, (short)y, (short)z, 0, !!cross);
}

void
CFileLoader::LoadCarPathNode(char *line, int id, int i)
{
	int type, next, cross;
	float x, y, z, width;
	int left, right;
	sscanf(line, "%d %d %d %f %f %f %f %d %d",
		&type, &next, &cross, &x, &y, &z, &width, &left, &right);
	CPathFind::StoreNodeInfoCar(id, i, type, next, (short)x, (short)y, (short)z, 0, left, right);
}

void
CFileLoader::LoadPathLine(char *line)
{
	static int pathType;
	switch(pathType){
	case 0:
		pathType = CFileLoader::LoadPathHeader(line);
		break;
	case 1:
		CFileLoader::LoadPedPathNode(line, pathId, pathItr);
		pathItr++;
		break;
	case 2:
		CFileLoader::LoadCarPathNode(line, pathId, pathItr);
		pathItr++;
		break;
	}
	// path is done after 12 nodes
	if(pathItr == 12){
		pathType = 0;
		pathItr = 0;
	}
}

void
CFileLoader::Load2dEffect(char *line)
{
	int id, r, g, b, a, type;
	float x, y, z;
	int flag, probability;
	char corona[32], shadow[32], *s;
	int shadowIntens, flash, wet, flare;
	sscanf(line, "%d %f %f %f %d %d %d %d %d",
	       &id, &x, &y, &z, &r, &g, &b, &a, &type);

	CTxdStore::PushCurrentTxd();
	CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));
	CBaseModelInfo *modelinfo;
	C2dEffect *fx;
	modelinfo = CModelInfo::ms_modelInfoPtrs[id];
	fx = &CModelInfo::ms_2dEffectStore.store[CModelInfo::ms_2dEffectStore.allocPtr++];
	modelinfo->Add2dEffect(fx);
	fx->pos.x = x;
	fx->pos.y = y;
	fx->pos.z = z;
	fx->col.red = r;
	fx->col.green = g;
	fx->col.blue = b;
	fx->col.alpha = a;
	fx->type = type;

	switch(type){
	case 0:	// light
		while(*line++ != '"');
		s = corona;
		while((*s++ = *line++) != '"');
		s[-1] = '\0';
		while(*line++ != '"');
		s = shadow;
		while((*s++ = *line++) != '"');
		s[-1] = '\0';
		sscanf(line, "%f %f %f %f %d %d %d %d %d",
		       &fx->light.dist, &fx->light.outerRange,
		       &fx->light.size, &fx->light.innerRange,
		       &shadowIntens, &flash, &wet, &flare, &flag);
		fx->light.corona = rw::Texture::read(corona, NULL);
		fx->light.shadow = rw::Texture::read(shadow, NULL);
		fx->light.shadowIntens = shadowIntens;
		fx->light.flash = flash;
		fx->light.wet = wet;
		fx->light.flare = flare;
		fx->light.flag = flag;
		if(flag & 4)
			fx->light.flag &= ~2;
		break;
	case 1: // particles
		sscanf(line, "%d %f %f %f %d %d %d %d %d"
		             " %d %f %f %f %f",
		       &id, &x, &y, &z, &r, &g, &b, &a, &type,
		       &fx->particle.particleType,
		       &fx->particle.dir[0], &fx->particle.dir[1],
		       &fx->particle.dir[2], &fx->particle.scale);
		break;
	case 2: // ped attractor?
		sscanf(line, "%d %f %f %f %d %d %d %d %d"
		             " %d %f %f %f %d",
		       &id, &x, &y, &z, &r, &g, &b, &a, &type,
		       &flag, &fx->attractor.dir[0], &fx->attractor.dir[1],
		       &fx->attractor.dir[2], &probability);
		fx->attractor.flag = flag;
		fx->attractor.probability = probability;
		break;
	}
	CTxdStore::PopCurrentTxd();
}

//
// IPL
//

void
CFileLoader::LoadObjectInstance(char *line)
{
	int id;
	char model[24];
	float tx, ty, tz;
	float sx, sy, sz;
	float rx, ry, rz, rw;
	sscanf(line, "%d %s %f %f %f %f %f %f %f %f %f %f",
	       &id, model, &tx, &ty, &tz, &sx, &sy, &sz,
	       &rx, &ry, &rz, &rw);
}

void
CFileLoader::LoadZone(char *line)
{
	int type;
	char name[24];
	float minx, miny, minz;
	float maxx, maxy, maxz;
	int level;
	sscanf(line, "%s %d %f %f %f %f %f %f %d",
	       name, &type,
               &minx, &miny, &minz,
               &maxx, &maxy, &maxz, &level);
}

void
CFileLoader::LoadCullZone(char *line)
{
	float cx, cy, cz;
	float x1, y1, z1;
	float x2, y2, z2;
	int flag, wanted;
	sscanf(line, "%f %f %f %f %f %f %f %f %f %d %d",
	       &cx, &cy, &cz, &x1, &y1, &z1, &x2, &y2, &z2,
	       &flag, &wanted);
}

//
// ZON
//

void
CFileLoader::LoadMapZone(char *line)
{
	int type;
	char name[24];
	float minx, miny, minz;
	float maxx, maxy, maxz;
	int level;
	sscanf(line, "%s %d %f %f %f %f %f %f %d",
	       name, &type,
               &minx, &miny, &minz,
               &maxx, &maxy, &maxz, &level);
}

//
// Collision
//

#define COLL 0x4C4C4F43

void
CFileLoader::LoadCollisionFile(char *filename)
{
	struct {
		uint32 ident;
		uint32 size;
	} header;
	FILE *file;
	static uchar buf[55000];
	char name[24];
	CBaseModelInfo *modelinfo;
	if(file = fopen_ci(filename, "rb"), file == NULL)
		return;
	while(1){
		if(fread(&header, 8, 1, file) == 0 ||
		   header.ident != COLL)
			return;
		fread(buf, header.size, 1, file);
		memcpy(name, buf, 24);
		modelinfo = CModelInfo::GetModelInfo(name, NULL);
		if(modelinfo){
			if(modelinfo->colModel == NULL)
				modelinfo->colModel = new CColModel;
			readColModel(modelinfo->colModel, buf+24);
			modelinfo->colModel->level = CGame::currLevel;
			modelinfo->freeCol = true;
		}
	}
	fclose(file);
}

static void
GetNameAndLOD(char *nodename, char *name, int *n)
{
	char *underscore = NULL;
	for(char *s = nodename; *s != '\0'; s++){
		if(s[0] == '_' && (s[1] == 'l' || s[1] == 'L'))
			underscore = s;
	}
	if(underscore){
		strncpy(name, nodename, underscore - nodename);
		name[underscore - nodename] = '\0';
		*n = atoi(underscore + 2);
	}else{
		strncpy(name, nodename, 24);
		*n = 0;
	}
}

bool
CFileLoader::LoadAtomicFile(rw::Stream *stream, int id)
{
	CSimpleModelInfo *modelinfo;
	rw::Clump *clump;
	rw::Atomic *atomic;
	char *nodename, name[24];
	int n;
	if(rw::findChunk(stream, rw::ID_CLUMP, NULL, NULL)){
		clump = rw::Clump::streamRead(stream);
		if(clump == NULL)
			return false;
		modelinfo = (CSimpleModelInfo*)CModelInfo::ms_modelInfoPtrs[id];
		FORLIST(lnk, clump->atomics){
			atomic = rw::Atomic::fromClump(lnk);
			nodename = gta::getNodeName(atomic->getFrame());
			GetNameAndLOD(nodename, name, &n);
			modelinfo->SetAtomic(n, atomic);
			atomic->removeFromClump();
			atomic->setFrame(rw::Frame::create());
		}
		clump->destroy();
	}
	return true;
}

bool
CFileLoader::LoadClumpFile(rw::Stream *stream, int id)
{
	CClumpModelInfo *modelinfo;
	rw::Clump *clump;
	if(rw::findChunk(stream, rw::ID_CLUMP, NULL, NULL)){
		clump = rw::Clump::streamRead(stream);
		if(clump == NULL)
			return false;
		modelinfo = (CClumpModelInfo*)CModelInfo::ms_modelInfoPtrs[id];
		modelinfo->SetClump(clump);
		// TODO: ped low detail clump
	}
	return true;

}
