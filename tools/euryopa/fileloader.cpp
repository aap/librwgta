#include "euryopa.h"

GameFile*
NewGameFile(char *path)
{
	GameFile *f = new GameFile;
	f->name = strdup(path);
	return f;
}

namespace FileLoader {

GameFile *currentFile;

void*
DatDesc::get(DatDesc *desc, const char *name)
{
	for(; desc->name[0] != '\0'; desc++)
		if(strcmp(desc->name, name) == 0)
			return (void*)desc->handler;
	return (void*)desc->handler;
}

static char*
skipWhite(char *s)
{
	while(isspace(*s))
		s++;
	return s;
}

char*
LoadLine(FILE *f)
{
	static char linebuf[1024];
again:
	if(fgets(linebuf, 1024, f) == nil)
		return nil;
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
LoadDataFile(const char *filename, DatDesc *desc)
{
	FILE *file;
	char *line;
	void (*handler)(char*) = nil;

	if(file = fopen_ci(filename, "rb"), file == nil)
		return;
	while(line = LoadLine(file)){
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

void LoadNothing(char *line) { }

static int firstID, lastID;

void
LoadObject(char *line)
{
	int id;
	char model[MODELNAMELEN];
	char txd[MODELNAMELEN];
	int numAtomics;
	float dist[3];
	int flags;
	int n;

	// SA format
	numAtomics = 1;
	n = sscanf(line, "%d %s %s %f %d", &id, model, txd, dist, &flags);
	if(gameversion != GAME_SA || n != 5 || dist[0] < 4){
		// III and VC format
		sscanf(line, "%d %s %s %d", &id, model, txd, &numAtomics);
		switch(numAtomics){
		case 1:
			sscanf(line, "%d %s %s %d %f %d",
			       &id, model, txd, &numAtomics, dist, &flags);
			break;
		case 2:
			sscanf(line, "%d %s %s %d %f %f %d",
			       &id, model, txd, &numAtomics, dist, dist+1, &flags);
			break;
		case 3:
			sscanf(line, "%d %s %s %d %f %f %f %d",
			       &id, model, txd, &numAtomics, dist, dist+1, dist+2, &flags);
			break;
		}
	}

	ObjectDef *obj = AddObjectDef(id);
	obj->m_type = ObjectDef::ATOMIC;
	strncpy(obj->m_name, model, MODELNAMELEN);
	obj->m_txdSlot = AddTxdSlot(txd);
	obj->m_numAtomics = numAtomics;
	for(n = 0; n < numAtomics; n++)
		obj->m_drawDist[n] = dist[n];
	obj->SetFlags(flags);
	obj->m_isTimed = false;
	if(id > lastID) lastID = id;
	if(id < firstID) firstID = id;

	obj->m_file = currentFile;
}

void
LoadTimeObject(char *line)
{
	int id;
	char model[MODELNAMELEN];
	char txd[MODELNAMELEN];
	int numAtomics;
	float dist[3];
	int flags;
	int timeOn, timeOff;
	int n;

	// SA format
	numAtomics = 1;
	timeOn = 0;
	timeOff = 0;
	n = sscanf(line, "%d %s %s %f %d %d %d", &id, model, txd, dist, &flags, &timeOn, &timeOff);
	if(gameversion != GAME_SA || n != 7 || dist[0] < 4){
		// III and VC format
		sscanf(line, "%d %s %s %d", &id, model, txd, &numAtomics);
		switch(numAtomics){
		case 1:
			sscanf(line, "%d %s %s %d %f %d %d %d",
			       &id, model, txd, &numAtomics, dist, &flags,
			       &timeOn, &timeOff);
			break;
		case 2:
			sscanf(line, "%d %s %s %d %f %f %d %d %d",
			       &id, model, txd, &numAtomics, dist, dist+1, &flags,
			       &timeOn, &timeOff);
			break;
		case 3:
			sscanf(line, "%d %s %s %d %f %f %f %d %d %d",
			       &id, model, txd, &numAtomics, dist, dist+1, dist+2, &flags,
			       &timeOn, &timeOff);
			break;
		}
	}

	ObjectDef *obj = AddObjectDef(id);
	obj->m_type = ObjectDef::ATOMIC;
	strncpy(obj->m_name, model, MODELNAMELEN);
	obj->m_txdSlot = AddTxdSlot(txd);
	obj->m_numAtomics = numAtomics;
	for(n = 0; n < numAtomics; n++)
		obj->m_drawDist[n] = dist[n];
	obj->SetFlags(flags);
	obj->m_isTimed = true;
	obj->m_timeOn = timeOn;
	obj->m_timeOff = timeOff;
	if(id > lastID) lastID = id;
	if(id < firstID) firstID = id;

	obj->m_file = currentFile;
}

void
LoadAnimatedObject(char *line)
{
	int id;
	char model[MODELNAMELEN];
	char txd[MODELNAMELEN];
	char anim[MODELNAMELEN];
	float dist;
	int flags;

	sscanf(line, "%d %s %s %s %f %d", &id, model, txd, anim, &dist, &flags);

	ObjectDef *obj = AddObjectDef(id);
	obj->m_type = ObjectDef::CLUMP;
	strncpy(obj->m_name, model, MODELNAMELEN);
	obj->m_txdSlot = AddTxdSlot(txd);
	strncpy(obj->m_animname, anim, MODELNAMELEN);
	obj->m_numAtomics = 1;	// to make the distance code simpler
	obj->m_drawDist[0] = dist;
	obj->SetFlags(flags);

	obj->m_file = currentFile;
}

void LoadPathLine(char *line) { }
void Load2dEffect(char *line) { }

void
LoadTXDParent(char *line)
{
	char child[MODELNAMELEN], parent[MODELNAMELEN];

	sscanf(line, "%s %s", child, parent);
	TxdSetParent(child, parent);
}


static ObjectInst *tmpInsts[8096];
static int numTmpInsts;

void
LoadObjectInstance(char *line)
{
	using namespace rw;

	FileObjectInstance fi;

	char model[MODELNAMELEN];
	float areaf;
	float sx, sy, sz;
	int n;

	if(isSA()){
		sscanf(line, "%d %s %d  %f %f %f  %f %f %f %f  %d",
		       &fi.objectId, model, &fi.area,
		       &fi.position.x, &fi.position.y, &fi.position.z,
		       &fi.rotation.x, &fi.rotation.y, &fi.rotation.z, &fi.rotation.w,
		       &fi.lod);
	}else{
		n = sscanf(line, "%d %s %f  %f %f %f  %f %f %f  %f %f %f %f",
		       &fi.objectId, model, &areaf,
		       &fi.position.x, &fi.position.y, &fi.position.z,
		       &sx, &sy, &sz,
		       &fi.rotation.x, &fi.rotation.y, &fi.rotation.z, &fi.rotation.w);
		if(n != 13){
			sscanf(line, "%d %s  %f %f %f  %f %f %f  %f %f %f %f",
			       &fi.objectId, model,
			       &fi.position.x, &fi.position.y, &fi.position.z,
			       &sx, &sy, &sz,
			       &fi.rotation.x, &fi.rotation.y, &fi.rotation.z, &fi.rotation.w);
			areaf = 0.0f;
		}
		fi.area = areaf;
		fi.lod = -1;
	}

	ObjectDef *obj = GetObjectDef(fi.objectId);
	if(obj == nil){
		log("warning: object %d was never defined\n", fi.objectId);
		tmpInsts[numTmpInsts++] = nil;
		return;
	}

	ObjectInst *inst = AddInstance();
	inst->Init(&fi);

	if(!isSA() && obj->m_isBigBuilding)
		inst->SetupBigBuilding();

	if(isSA())
		tmpInsts[numTmpInsts++] = inst;

	inst->m_file = currentFile;
}

void
LoadZone(char *line)
{
	char name[24];
	char text[24];
	int type;
	CBox box;
	int level;
	int n;
	n = sscanf(line, "%s %d %f %f %f %f %f %f %d %s",
			name, &type,
			&box.min.x, &box.min.y, &box.min.z,
			&box.max.x, &box.max.y, &box.max.z,
			&level, text);
	if(n == 9)
		// in III we add all zones here to the zones array
		Zones::CreateZone(name, type, box, level, nil);
	else if(n == 10)
		Zones::CreateZone(name, type, box, level, text);
}

void
LoadMapZone(char *line)
{
	// Should only be called from III
	// zones always added to mapzone array
	LoadZone(line);
}

void
LoadCullZone(char *line)
{
	rw::V3d center;
	int flags;
	int wantedLevelDrop;

	wantedLevelDrop = 0;
	if(isSA()){
		float s1x, s1y;
		float s2x, s2y;
		float zmin, zmax;
		rw::Plane mirror;
		if(sscanf(line, "%f %f %f  %f %f %f  %f %f %f  %d  %f %f %f %f",
				&center.x, &center.y, &center.z,
				&s1x, &s1y, &zmin,
				&s2x, &s2y, &zmax,
				&flags,
				&mirror.normal.x, &mirror.normal.y, &mirror.normal.z, &mirror.distance) == 14)
			Zones::AddMirrorAttribZone(center, s1x, s1y, s2x, s2y, zmin, zmax, flags, mirror);
		else{
			sscanf(line, "%f %f %f  %f %f %f  %f %f %f  %d %d",
				&center.x, &center.y, &center.z,
				&s1x, &s1y, &zmin,
				&s2x, &s2y, &zmax,
				&flags, &wantedLevelDrop);
			Zones::AddAttribZone(center, s1x, s1y, s2x, s2y, zmin, zmax, flags);
		}
	}else{
		CBox box;
		sscanf(line, "%f %f %f  %f %f %f  %f %f %f  %d %d",
			&center.x, &center.y, &center.z,
			&box.min.x, &box.min.y, &box.min.z,
			&box.max.x, &box.max.y, &box.max.z,
			&flags, &wantedLevelDrop);
		Zones::AddAttribZone(box, flags, wantedLevelDrop);
	}
}

void
LoadTimeCycleModifier(char *line)
{
	CBox box;
	int farclp;
	int extraCol;
	float extraColIntensity;
	float falloffDist;
	float unused;
	float lodDistMult;

	falloffDist = 100.0f;
	unused = 1.0;
	lodDistMult = 1.0f;
	if(sscanf(line, "%f %f %f  %f %f %f  %d  %d %f  %f %f %f",
			&box.min.x, &box.min.y, &box.min.z,
			&box.max.x, &box.max.y, &box.max.z,
			&farclp,
			&extraCol, &extraColIntensity,
			&falloffDist, &unused, &lodDistMult) < 12)
		lodDistMult = unused;
	Timecycle::AddBox(box, farclp, extraCol, extraColIntensity, falloffDist, lodDistMult);
}

DatDesc zoneDesc[] = {
	{ "end", LoadNothing },
	{ "zone", LoadMapZone },
	{ "", nil }
};

DatDesc ideDesc[] = {
	{ "end", LoadNothing },
	{ "objs", LoadObject },
	{ "tobj", LoadTimeObject },
	{ "hier", LoadNothing },
	{ "cars", LoadNothing },
	{ "peds", LoadNothing },
	{ "path", LoadPathLine },
	{ "2dfx", Load2dEffect },
// VC
	{ "weap", LoadNothing },
// SA
	{ "anim", LoadAnimatedObject },
	{ "txdp", LoadTXDParent },
	{ "", nil }
};

DatDesc iplDesc[] = {
	{ "end", LoadNothing },
	{ "inst", LoadObjectInstance },
	{ "zone", LoadZone },
	{ "cull", LoadCullZone },
	{ "pick", LoadNothing },
	{ "path", LoadNothing },

	{ "occl", LoadNothing },
	{ "mult", LoadNothing },	// actually no-op
	{ "grge", LoadNothing },
	{ "enex", LoadNothing },
	{ "cars", LoadNothing },
	{ "jump", LoadNothing },
	{ "tcyc", LoadTimeCycleModifier },
	{ "auzo", LoadNothing },
	{ "", nil }
};

void
LoadObjectTypes(const char *filename)
{
	int i;
	firstID = 0x7FFFFFFF;
	lastID = -1;
	LoadDataFile(filename, ideDesc);

	/* Finding related models is done per file
	 * but not yet in III */
	if(isIII()){
		firstID = 0;
		lastID = NUMOBJECTDEFS;
	}
	for(i = firstID; i < lastID; i++)
		if(GetObjectDef(i))
			GetObjectDef(i)->SetupBigBuilding(firstID, lastID);
}

static void
SetupRelatedIPLs(const char *path, int instArraySlot)
{
	const char *filename, *ext, *s;
	char *t, scenename[256];	// maximum is way less anyway....
	int len;
	int i;
	IplDef *ipl;

	filename = strrchr(path, '\\');
	assert(filename);
	ext = strchr(filename, '.');
	assert(ext);
	t = scenename;
	for(s = filename+1; s != ext; s++)
		*t++ = *s;
	*t++ = '\0';
	strcat(scenename, "_stream");
	len = strlen(scenename);

	for(i = 0; i < NUMIPLS; i++){
		ipl = GetIplDef(i);
		if(ipl == nil)
			continue;
		if(rw::strncmp_ci(scenename, ipl->name, len) == 0){
			ipl->instArraySlot = instArraySlot;
			LoadIpl(i);
		}
	}
}

// SA only
void
SetupBigBuildings(void)
{
	int i;
	ObjectInst *inst, *lodinst;
	ObjectDef *obj, *lodobj;

	for(i = 0; i < numTmpInsts; i++){
		inst = tmpInsts[i];
		if(inst->m_lodId < 0)
			inst->m_lod = nil;
		else{
			lodinst = tmpInsts[inst->m_lodId];
			inst->m_lod = lodinst;
			lodinst->m_numChildren++;
		}
	}

	for(i = 0; i < numTmpInsts; i++){
		inst = tmpInsts[i];
		obj = GetObjectDef(inst->m_objectId);
		if(obj->m_isBigBuilding || inst->m_numChildren)
			inst->SetupBigBuilding();
		lodinst = inst->m_lod;
		if(lodinst == nil)
			continue;
		lodobj = GetObjectDef(lodinst->m_objectId);
		if(lodinst->m_numChildren == 1 && obj->m_colModel){
			lodobj->m_colModel = obj->m_colModel;
			lodobj->m_gotChildCol = true;
		}
		assert(lodobj->m_colModel);
		assert(obj->m_colModel);
	}
}

void
LoadScene(const char *filename)
{
	numTmpInsts = 0;
	LoadDataFile(filename, iplDesc);

	if(isSA()){
		int i = -1;
		if(numTmpInsts){
			i = AddInstArraySlot(numTmpInsts);
			ObjectInst **ia = GetInstArray(i);
			memcpy(ia, tmpInsts, numTmpInsts*sizeof(ObjectInst*));
		}

		SetupRelatedIPLs(filename, i);
		SetupBigBuildings();
	}
}

void LoadMapZones(const char *filename) { LoadDataFile(filename, zoneDesc); }


rw::TexDictionary*
LoadTexDictionary(const char *path)
{
	using namespace rw;

	StreamFile stream;
	TexDictionary *txd = nil;
	if(stream.open(getPath(path), "rb")){
		if(findChunk(&stream, rw::ID_TEXDICTIONARY, nil, nil)){
			txd = TexDictionary::streamRead(&stream);
			ConvertTxd(txd);
		}
		stream.close();
	}
	if(txd == nil)
		txd = TexDictionary::create();
	return txd;
}

static void
AddTexDictionaries(rw::TexDictionary *dst, rw::TexDictionary *src)
{
	FORLIST(lnk, src->textures)
		dst->addFront(rw::Texture::fromDict(lnk));
}

static void
LoadCollisionFile(const char *path)
{
	FILE *f;
	ColFileHeader colfile;
	int version;
	uint8 *buffer;
	ObjectDef *obj;
 
	f = fopen_ci(path, "rb");
	if(f == nil)
		return;
	while(fread(&colfile, 1, 8, f)){
		version = 0;
		switch(colfile.fourcc){
		case 0x4C4C4F43:	// COLL
			version = 1;
			break;
		case 0x324C4F43:	// COL2
			version = 2;
			break;
		case 0x334C4F43:	// COL3
			version = 3;
			break;
		case 0x344C4F43:	// COL4
			version = 4;
			break;
		default:
			fclose(f);
			return;
		}
		fread(colfile.name, 1, 24, f);
		buffer = rwNewT(uint8, colfile.modelsize-24, 0);
		fread(buffer, 1, colfile.modelsize-24, f);

		obj = GetObjectDef(colfile.name, nil);
		if(obj){
			CColModel *col = new CColModel;
			strncpy(col->name, colfile.name, 24);
			col->file = currentFile;
			obj->m_colModel = col;
			switch(version){
			case 1: ReadColModel(col, buffer, colfile.modelsize-24); break;
			case 2: ReadColModelVer2(col, buffer, colfile.modelsize-24); break;
			case 3: ReadColModelVer3(col, buffer, colfile.modelsize-24); break;
			case 4: ReadColModelVer4(col, buffer, colfile.modelsize-24); break;
			default:
				printf("unknown COL version %d\n", version);
				obj->m_colModel = nil;
			}
		}else
			printf("Couldn't find object %s for collision\n", colfile.name);

		rwFree(buffer);
	}
	fclose(f);
}

static bool haveFinishedDefinitions;

void
LoadLevel(const char *filename)
{
	FILE *file;
	char *line;
	char path[256];
	rw::TexDictionary *curTxd;


	if(file = fopen_ci(filename, "rb"), file == nil)
		return;

	curTxd = rw::TexDictionary::getCurrent();
	while(line = LoadLine(file)){
		if(line[0] == '#')
			continue;
		if(strncmp(line, "EXIT", 4) == 0)
			break;
		else if(strncmp(line, "IMAGEPATH", 9) == 0){
			strcpy(path, line+10);
			strcat(path, "/");
			rw::Image::setSearchPath(path);
		}else if(strncmp(line, "TEXDICTION", 10) == 0){
			rw::TexDictionary *txd;
			txd = LoadTexDictionary(line+11);
			AddTexDictionaries(curTxd, txd);
			txd->destroy();
		}else if(strncmp(line, "COLFILE", 7) == 0){
//			eLevelName currlevel = CGame::currLevel;
//			sscanf(line+8, "%d", (int*)&CGame::currLevel);
			strncpy(path, line+10, 256);
			currentFile = NewGameFile(path);
			LoadCollisionFile(path);
//			CGame::currLevel = currlevel;
		}else if(strncmp(line, "MODELFILE", 9) == 0){
			// TODO
//			CFileLoader::LoadModelFile(line+10);
//			debug("MODELFILE\n");
		}else if(strncmp(line, "HIERFILE", 8) == 0){
			// TODO
//			CFileLoader::LoadClumpFile(line+9);
//			debug("HIERFILE\n");
		}else if(strncmp(line, "IDE", 3) == 0){
			strncpy(path, line+4, 256);
			currentFile = NewGameFile(path);
			LoadObjectTypes(path);
		}else if(strncmp(line, "IPL", 3) == 0){
			if(!haveFinishedDefinitions){
				InitCdImages();
				LoadAllCollisions();
				haveFinishedDefinitions = true;
			}

			strncpy(path, line+4, 256);
			currentFile = NewGameFile(path);
			LoadScene(path);
		}else if(strncmp(line, "MAPZONE", 7) == 0){
//			debug("MAPZONE\n");
			strncpy(path, line+8, 256);
			currentFile = NewGameFile(path);
			LoadMapZones(path);
		}else if(strncmp(line, "SPLASH", 6) == 0){
//			printf("[SPLASH %s]\n", line+7);
		}else if(strncmp(line, "CDIMAGE", 7) == 0){
			AddCdImage(line+8);
		}else if(strncmp(line, "IMG", 3) == 0){
			AddCdImage(line+4);
		}
	}
	fclose(file);
}

}
