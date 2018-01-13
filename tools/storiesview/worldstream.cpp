#include "storiesview.h"

// sectors in III: -2000 - 2000: 100 sectors of size 40 in both directions
// sectors in VC: x: -2400 - 1600: 80 sectors of size 50
//                y: -2000 - 2000: 80 sectors of size 50

#ifdef LCS
const char *lvznames[] = {
	nil,
	"MODELS/INDUST.LVZ",
	"MODELS/COMMER.LVZ",
	"MODELS/SUBURB.LVZ",
	"MODELS/UNDERG.LVZ",
};
const char *imgnames[] = {
	nil,
	"MODELS/INDUST.IMG",
	"MODELS/COMMER.IMG",
	"MODELS/SUBURB.IMG",
	"MODELS/UNDERG.IMG",
};
#define XINC 100.0f
#define YINC 86.6f
#define XSTART (-2000.0f)
#define YSTART (-2000.0f)
#else
const char *lvznames[] = {
	nil,
	"BEACH.LVZ",
	"MAINLA.LVZ",
	"MALL.LVZ",
};
const char *imgnames[] = {
	nil,
	"BEACH.IMG",
	"MAINLA.IMG",
	"MALL.IMG",
};
#define XINC 125.0f
#define YINC 108.25f
#define XSTART (-2400.0f)
#define YSTART (-2000.0f)
#endif

LevelExt *gLevel;
rw::V3d worldSectorPositions[NUMSECTORSX][NUMSECTORSY];
SectorExt *worldSectors[NUMSECTORSX][NUMSECTORSY];

Resource*
GetResource(int id)
{
	return &gLevel->chunk->resourceTable[id];
}

BuildingExt*
GetBuildingExt(int id)
{
	BuildingExt *be;
	if(gLevel->buildings[id])
		return gLevel->buildings[id];
	be = (BuildingExt*)malloc(sizeof(BuildingExt));
	memset(be, 0, sizeof(BuildingExt));
	gLevel->buildings[id] = be;
	return be;
}

void
LoadLevel(eLevel lev)
{
	int i, j;
	int n;
	const char *imgname, *lvzname;
	imgname = imgnames[lev];
	lvzname = lvznames[lev];

	Zfile *zfile;
	zfile = zopen(lvzname, "rb");
	if(zfile == nil){
		sk::globals.quit = 1;
		return;
	}
	sChunkHeader header;
	zread(zfile, &header, sizeof(sChunkHeader));
	uint8 *data = (uint8*)malloc(header.fileSize-sizeof(sChunkHeader));
	zread(zfile, data, header.fileSize-sizeof(sChunkHeader));
	zclose(zfile);
	cReloctableChunk(header.ident, header.shrink).Fixup(header, data);
	gLevel = new LevelExt;
	memset(gLevel, 0, sizeof(LevelExt));
	gLevel->chunk = (sLevelChunk*)data;
	gLevel->imgfile = fopen(imgname, "rb");
	if(gLevel->imgfile == nil){
		sk::globals.quit = 1;
		return;
	}
	sChunkHeader *pheader = gLevel->chunk->sectorRows[0].header;
	gLevel->numSectors = 0;
	gLevel->numWorldSectors = 0;
	for(i = 0; pheader->ident == WRLD_IDENT ; i++){
		gLevel->numSectors++;
		if(pheader < gLevel->chunk->sectorEnd.header){
			gLevel->numWorldSectors++;
		}
		pheader++;
	}

//	printf("%d %d\n", gLevel->numStreamingSectors, gLevel->numSectors);

	// Allocate extension data
	gLevel->sectors = (SectorExt*)malloc(gLevel->numSectors*sizeof(SectorExt));
	gLevel->res = (ResourceExt*)malloc(gLevel->chunk->numResources*sizeof(ResourceExt));
	memset(gLevel->sectors, 0, gLevel->numSectors*sizeof(SectorExt));
	memset(gLevel->res, 0, gLevel->chunk->numResources*sizeof(ResourceExt));

	SectorExt *se = gLevel->sectors;

	// Set up world sectors
	memset(worldSectors, 0, sizeof(worldSectors));
	for(i = 0; i < NUMSECTORSY; i++){
		// Get origin
		for(j = 0; j < NUMSECTORSX; j++){
			worldSectorPositions[j][i].x = XSTART + XINC/2 +
				XINC*j - (i&1)*XINC/2;
			worldSectorPositions[j][i].y = YSTART + YINC/2 +
				YINC*i;
			worldSectorPositions[j][i].z = 0.0f;
		}
		// Store sector x and y coords
		n = gLevel->chunk->sectorRows[i+1].header - gLevel->chunk->sectorRows[i].header;
		for(j = 0; j < n; j++){
			se->secx = gLevel->chunk->sectorRows[i].startOff + j;
			se->secy = i;
			se->origin = worldSectorPositions[se->secx][se->secy];
			se->type = SECTOR_WORLD;
			se++;
		}
	}

	// Set up timed objects and hide building swaps, whatever that is exactly
	TriggerInfo *trig = gLevel->chunk->triggeredObjects;
	for(i = 0; i < gLevel->chunk->numTriggeredObjects; i++){
		BuildingExt *be = GetBuildingExt(trig->id);
		if(trig->timeOff & 0x80){
			be->isTimed = true;
			be->timeOff = trig->timeOff & 0x7F;
			be->timeOn = trig->timeOn;
		}else
			be->hidden = true;
//		printf("%d %d %d\t%d\n", time->timeOff & 0x80, time->timeOn, time->timeOff & 0x7F, time->id);
		trig++;
	}

	// Set up interior sectors
	Interior *intr = gLevel->chunk->interiors;
	for(i = 0; i < gLevel->chunk->numInteriors; i++){
		se = &gLevel->sectors[intr->sectorId];
		se->secx = intr->secx;
		se->secy = intr->secy;
		se->origin = worldSectorPositions[se->secx][se->secy];
		se->type = SECTOR_INTERIOR;
//		printf("%d %d %d %d %d\n", intr->listIndex, intr->listOffset, intr->c, intr->d, intr->streamWorldId);
		intr++;
	}

#ifdef VCS
	gLevel->areas = (Area**)malloc(gLevel->chunk->numAreas*sizeof(void*));
	memset(gLevel->areas, 0, gLevel->chunk->numAreas*sizeof(void*));

//	AreaInfo *ai = gLevel->chunk->areas;
//	for(i = 0; i < gLevel->chunk->numAreas; i++){
//		printf("%x %x\n", ai->a, ai->b);
//		ai++;
//	}
#endif
}

void
LoadSector(int n)
{
	int i;

	if(n >= gLevel->numSectors)
		return;
	sChunkHeader *pheader = gLevel->chunk->sectorRows[0].header + n;
	uint8 *s = (uint8*)malloc(pheader->fileSize-sizeof(sChunkHeader));
	fseek(gLevel->imgfile, pheader->globalTab, SEEK_SET);
	fread(s, 1, pheader->fileSize-sizeof(sChunkHeader), gLevel->imgfile);
	cReloctableChunk(pheader->ident, pheader->shrink).Fixup(*pheader, s);

	SectorExt *se = &gLevel->sectors[n];
	se->sect = (Sector*)s;
	if(se->type == SECTOR_WORLD)
		worldSectors[se->secx][se->secy] = se;

	se->numInstances = 0;
	se->instances = nil;
	se->dummies = nil;
//	printf("%d %x, %p %p %p %p %p %p %p %p\n", n, se->sect->unk1,
//		se->sect->sectionA, se->sect->sectionB, se->sect->sectionC, se->sect->sectionD,
//		se->sect->sectionE, se->sect->sectionF, se->sect->sectionG, se->sect->sectionEnd);

	// Add new resources to global resource table
	for(i = 0; i < se->sect->numResources; i++){
		OverlayResource *or = &se->sect->resources[i];
		Resource *res = &gLevel->chunk->resourceTable[or->id];
		ResourceExt *re = &gLevel->res[or->id];
		res->raw = or->raw;
		re->sector = se;
	}

	// Make some room for our RW data
	if(se->type == SECTOR_WORLD || se->type == SECTOR_INTERIOR){
		sGeomInstance *inst;
		for(inst = se->sect->sectionA; inst != se->sect->sectionEnd; inst++)
			se->numInstances++;
		se->instances = (rw::Atomic**)malloc(se->numInstances*sizeof(void*));
		memset(se->instances, 0, se->numInstances*sizeof(void*));
		se->dummies = (rw::Atomic**)malloc(se->numInstances*sizeof(void*));
		memset(se->dummies, 0, se->numInstances*sizeof(void*));

		TriggerInfo *trig = se->sect->triggeredObjects;
		for(i = 0; i < se->sect->numTriggeredObjects; i++){
			SectorExt *se = &gLevel->sectors[trig->id];

			if(trig->timeOff & 0x80){
				se->isTimed = true;
				se->timeOff = trig->timeOff & 0x7F;
				se->timeOn = trig->timeOn;
			}else
				se->hidden = true;
			LoadSector(trig->id);

//			printf("%d %d %d\t%d\n", trig->timeOff & 0x80, trig->timeOn, trig->timeOff & 0x7F, trig->id);
			trig++;
		}

	}
}

#ifdef VCS
void
LoadArea(int n)
{
	if(n >= gLevel->chunk->numAreas)
		return;
	AreaInfo *ai = &gLevel->chunk->areas[n];

	uint8 *data = (uint8*)malloc(ai->fileSize);
	fseek(gLevel->imgfile, ai->fileOffset, SEEK_SET);
	fread(data, 1, ai->fileSize, gLevel->imgfile);
	sChunkHeader *header = (sChunkHeader*)data;
	cReloctableChunk(header->ident, header->shrink).Fixup(data);

	Area *a = (Area*)(header+1);
	gLevel->areas[n] = a;

	int i;
	AreaResource *ar = a->resources;
	for(i = 0; i < a->numResources; i++){
//		printf("%x %x \n", ar->id1, ar->id2);
		gLevel->chunk->resourceTable[ar->id1].raw = (uint8*)ar->data;
		ar++;
	}
}
#endif


static uint32
unpackSize(uint32 unpack)
{
	if((unpack&0x6F000000) == 0x6F000000)
		return 2;
	static uint32 size[] = { 32, 16, 8, 16 };
	return ((unpack>>26 & 3)+1)*size[unpack>>24 & 3]/8;
}

static uint32*
skipUnpack(uint32 *p)
{
	int32 n = (p[0] >> 16) & 0xFF;
	return p + (n*unpackSize(p[0])+3 >> 2) + 1;
}

int
countStripVertices(uint32 *data, int32 size)
{
	uint32 *end;
	uint32 *w;
	uint32 nvert;
	uint32 totalVerts;
	bool firstBatch;

	end = &data[size/4];
	w = data;
	totalVerts = 0;
	firstBatch = true;
	while(w < end){
		// GIFtag probably
		assert(w[0] == 0x6C018000);	// UNPACK
		nvert = w[4] & 0x7FFF;
		if(!firstBatch) nvert -=2;
		w += 5;

		// positions
		assert(w[0] == 0x20000000);	// STMASK
		w += 2;
		assert(w[0] == 0x30000000);	// STROW
		w += 5;
		assert((w[0] & 0xFF004000) == 0x79000000);
		w = skipUnpack(w);

		// tex coords
		assert(w[0] == 0x20000000);	// STMASK
		w += 2;
		assert(w[0] == 0x30000000);	// STROW
		w += 5;
		assert((w[0] & 0xFF004000) == 0x76004000);
		w = skipUnpack(w);

		// prelight
		assert((w[0] & 0xFF004000) == 0x6F000000);
		w = skipUnpack(w);

		assert(w[0] == 0x14000006);	// MSCAL
		w++;
		while(w[0] == 0) w++;

		totalVerts += nvert;
	}
	return totalVerts;
}

void
convertBuildingMesh(rw::Geometry *geo, rw::Mesh *m, sClippableBuildingMesh *mesh, uint32 *data, int32 size)
{
	uint32 *end;
	uint32 *w;
	uint32 nvert;
	float uvscale[2];
	rw::ps2::Vertex v;

	int16 *vuVerts = nil;
	uint8 *vuTex = nil;
	uint16 *vuCols = nil;

	uvscale[0] = halfFloatToFloat(mesh->uvScale[0]);
	uvscale[1] = halfFloatToFloat(mesh->uvScale[1]);

	uint32 mask = 0x1101;	// tex coords, prelight, vertices

	end = &data[size/4];
	w = data;
	bool firstInst = m->numIndices == 0;
	bool firstBatch = true;
	while(w < end){
		// GIFtag probably
		assert(w[0] == 0x6C018000);	// UNPACK
		nvert = w[4] & 0x7FFF;
		if(!firstBatch) nvert -=2;
		w += 5;

		// positions
		assert(w[0] == 0x20000000);	// STMASK
		w += 2;
		assert(w[0] == 0x30000000);	// STROW
		w += 5;
		assert((w[0] & 0xFF004000) == 0x79000000);
		vuVerts = (int16*)(w+1);
		if(!firstBatch) vuVerts += 2*3;
		w = skipUnpack(w);

		// tex coords
		assert(w[0] == 0x20000000);	// STMASK
		w += 2;
		assert(w[0] == 0x30000000);	// STROW
		w += 5;
		assert((w[0] & 0xFF004000) == 0x76004000);
		vuTex = (uint8*)(w+1);
		if(!firstBatch) vuTex += 2*2;
		w = skipUnpack(w);

		// prelight
		assert((w[0] & 0xFF004000) == 0x6F000000);
		vuCols = (uint16*)(w+1);
		if(!firstBatch) vuCols += 2;
		w = skipUnpack(w);

		assert(w[0] == 0x14000006);	// MSCAL
		w++;
		while(w[0] == 0) w++;

		/* Insert Data */
		for(uint32 i = 0; i < nvert; i++){
			v.p.x = vuVerts[0]/32767.0f;
			v.p.y = vuVerts[1]/32767.0f;
			v.p.z = vuVerts[2]/32767.0f;
			v.t.u = vuTex[0]/128.0f*uvscale[0];
			v.t.v = vuTex[1]/128.0f*uvscale[1];
			v.c.red   = (vuCols[0] & 0x1f) * 255 / 0x1F;
			v.c.green = (vuCols[0]>>5 & 0x1f) * 255 / 0x1F;
			v.c.blue  = (vuCols[0]>>10 & 0x1f) * 255 / 0x1F;
			v.c.alpha = vuCols[0]&0x8000 ? 0xFF : 0;

			int32 idx = rw::ps2::findVertexSkin(geo, nil, mask, &v);
			if(idx < 0)
				idx = geo->numVertices++;

			if(i == 0 && firstBatch && !firstInst){
				m->indices[m->numIndices++] = m->indices[m->numIndices-1];
				m->indices[m->numIndices++] = idx;
	//			if(inst->numTriangles % 2)
	//				m->indices[m->numIndices++] = idx;
				if(m->numIndices % 2)
					m->indices[m->numIndices++] = idx;
	//			if(inst[-1].numTriangles % 2)
	//				m->indices[m->numIndices++] = idx;
			}

			m->indices[m->numIndices++] = idx;
			rw::ps2::insertVertexSkin(geo, idx, mask, &v);

			vuVerts += 3;
			vuTex += 2;
			vuCols++;
		}
		firstInst = false;
		firstBatch = false;
	}
}

bool
isTextureTransparent(rw::Texture *tex)
{
	if(tex == nil)
		return false;
	return (tex->raster->format & 0xF00) == rw::Raster::C8888 ||
		(tex->raster->format & 0xF00) == rw::Raster::C1555;
}

bool
isGeoTransparent(rw::Geometry *geo)
{
	int i;
	rw::MaterialList *mlist = &geo->matList;
	for(i = 0; i < mlist->numMaterials; i++)
		if(isTextureTransparent(mlist->materials[i]->texture))
			return true;
	return false;
}

void
setGeoMaterialColor(rw::Geometry *geo, rw::RGBA col)
{
	int i;
	rw::MaterialList *mlist = &geo->matList;
	for(i = 0; i < mlist->numMaterials; i++)
		mlist->materials[i]->color = col;
}

int
lookupint(int *haystack, int n, int needle)
{
	int i;
	for(i = 0; i < n; i++)
		if(haystack[i] == needle)
			return i;
	return -1;
}

rw::Geometry*
makeWorldGeometry(sBuildingGeometry *bgeom)
{
	using namespace rw;

	int i;
	int matid;
	sClippableBuildingMesh *mesh;
	uint8 *dmaData;
	Geometry *geo;
	int *matmap;
	int nmat;

	geo = Geometry::create(0, 0, Geometry::TRISTRIP | Geometry::LIGHT |
		Geometry::PRELIT | Geometry::TEXTURED);

	mesh = (sClippableBuildingMesh*)(bgeom+1);
	dmaData = (uint8*)mesh + bgeom->size;

	matmap = (int*)malloc(bgeom->numMeshes*sizeof(int));
	nmat = 0;
	// Figure out a material ID for each mesh.
	// Maybe this is overkill, but better be safe.
	for(i = 0; i < bgeom->numMeshes; i++){
//printf("%x %x %x %x\n", i, mesh[i].texID, mesh[i].packetSize, mesh[i].unk1);
		matid = lookupint(matmap, nmat, mesh[i].texID);
		if(matid < 0){
			matid = nmat++;
			matmap[matid] = mesh[i].texID;
		}
	}

	// Manually create material list
	geo->matList.numMaterials = nmat;
	geo->matList.space = nmat;
	geo->matList.materials = rwMallocT(Material*, nmat, MEMDUR_EVENT | ID_MATERIAL);
	for(int32 i = 0; i < nmat; i++){
		geo->matList.materials[i] = Material::create();
		Resource *texres = GetResource(matmap[i]);
		if(texres->raw == nil)
			continue;
		if(texres->dmaChain == nil){
			RslRaster *ras = texres->raster;
			rw::Raster *raster = convertRasterPS2(&ras->ps2);
			if(raster == nil)
				continue;
			rw::Texture *tex = rw::Texture::create(raster);
			tex->setFilter(rw::Texture::LINEAR);
			texres->dmaChain = tex;
		}
		geo->matList.materials[i]->setTexture((rw::Texture*)texres->dmaChain);
	}

	// allocate empty meshes
	geo->allocateMeshes(geo->matList.numMaterials, 0, 1);
	geo->meshHeader->flags = MeshHeader::TRISTRIP;
	Mesh *meshes = geo->meshHeader->getMeshes();

	// Find out number of indices for every mesh
	for(i = 0; i < bgeom->numMeshes; i++){
		matid = lookupint(matmap, nmat, mesh[i].texID);
		assert(matid >= 0);

		int size = mesh[i].GetPacketSize();
		int n = countStripVertices((uint32*)dmaData, size);
		assert(n != 0);
		geo->numVertices += n;
		meshes[matid].numIndices += n +3;
//		printf("%d %d\n", n, mesh[i].unk1);
		dmaData += size;
	}
	for(uint32 i = 0; i < geo->meshHeader->numMeshes; i++){
		meshes[i].material = geo->matList.materials[i];
		geo->meshHeader->totalIndices += meshes[i].numIndices;
	}

	// Allocate enough space for everything
	geo->numTriangles = geo->meshHeader->guessNumTriangles();
	geo->allocateData();
	geo->allocateMeshes(geo->meshHeader->numMeshes, geo->meshHeader->totalIndices, 0);
	meshes = geo->meshHeader->getMeshes();

	// Zero vertex counts again because the above were just estimates
	for(uint32 i = 0; i < geo->meshHeader->numMeshes; i++)
		meshes[i].numIndices = 0;
	geo->meshHeader->totalIndices = geo->numVertices = 0;

	// Now actually do the conversion
	dmaData = (uint8*)mesh + bgeom->size;
	for(i = 0; i < bgeom->numMeshes; i++){
		matid = lookupint(matmap, nmat, mesh[i].texID);

		int size = mesh[i].GetPacketSize();

		convertBuildingMesh(geo, &meshes[matid], &mesh[i], (uint32*)dmaData, size);
		dmaData += size;
	}

	for(uint32 i = 0; i < geo->meshHeader->numMeshes; i++)
		geo->meshHeader->totalIndices += meshes[i].numIndices;
	geo->calculateBoundingSphere();
	geo->generateTriangles();
//	geo->removeUnusedMaterials();	// this shouldn't be needed

	free(matmap);
	return geo;
};

BuildingExt::Model*
BuildingExt::GetResourceInfo(int id)
{
	Model *m;
	for(m = this->resources; m; m = m->next)
		if(m->resId == id)
			return m;
	m = (Model*)malloc(sizeof(Model));
	m->next = this->resources;
	this->resources = m;
	m->resId = id;
	m->lastFrame = 0;
	return m;
}

void
renderSector(SectorExt *se)
{
	int i;

	cubeMat->color.red = 255;
	cubeMat->color.green = 255;
	cubeMat->color.blue = 0;
//	pAmbient->setColor(1.0f, 1.0f, 1.0f);

	if(se == nil)
		return;

	for(i = 0; i < se->numInstances; i++){
		sGeomInstance *inst = &se->sect->sectionA[i];
		BuildingExt *be = GetBuildingExt(inst->GetId());
		BuildingExt::Model *m = be->GetResourceInfo(inst->resId);

		if(m->lastFrame == frameCounter)
			continue;
		m->lastFrame = frameCounter;

//		if(!be->hidden)
//			continue;
		if(be->isTimed && !GetIsTimeInRange(be->timeOn, be->timeOff))
			continue;
		if(inst->IsLOD() != drawLOD)
			continue;

		ResourceExt *re = &gLevel->res[inst->resId];
		if(re->sector && re->sector->isTimed && !GetIsTimeInRange(re->sector->timeOn, re->sector->timeOff))
			continue;

		if(se->instances[i] == nil){
			Resource *res = GetResource(inst->resId);

			rw::Geometry *geo = nil;
			rw::Matrix m = *(rw::Matrix*)&inst->matrix;
			if(res->raw == nil){
printf("missing 0x%X %x\n", inst->resId, inst->GetId());
				// This shouldn't happen eventually probably
				m.right.set(1.0f, 0.0f, 0.0f);
				m.up.set(0.0f, 1.0f, 0.0f);
				m.at.set(0.0f, 0.0f, 1.0f);
				geo = cubeGeo;
			}else if(res->dmaChain){
				geo = (rw::Geometry*)res->dmaChain;
			}else{
				geo = makeWorldGeometry(res->geometry);
				res->dmaChain = geo;
			}
			be->isTransparent = isGeoTransparent(geo);

			rw::Atomic *a = rw::Atomic::create();
			rw::Frame *f = rw::Frame::create();
			a->setGeometry(geo, 0);
			a->setFrame(f);
			a->pipeline = Renderer::buildingPipe;
			m.pos = add(m.pos, se->origin);

			m.optimize();
			f->transform(&m, rw::COMBINEREPLACE);
			se->instances[i] = a;
		}

		float x = halfFloatToFloat(inst->bound[0]);
		float y = halfFloatToFloat(inst->bound[1]);
		float z = halfFloatToFloat(inst->bound[2]);
		float r = halfFloatToFloat(inst->bound[3]);
		rw::Sphere sph = { { x, y, z }, r };
		if(TheCamera.m_rwcam->frustumTestSphere(&sph) == rw::Camera::SPHEREOUTSIDE)
			continue;
		if(TheCamera.distanceTo(sph.center) < 400.0f){
			CSphere s = { { x, y, z }, r };
			static rw::RGBA c = { 0, 0, 255, 255 };
			static rw::Matrix ident;
			ident.setIdentity();
			RenderWireSphere(&s, c, &ident);
		}

		if(be->isTransparent)
			Renderer::addToTransparentRenderList(se->instances[i]);
		else
			Renderer::addToOpaqueRenderList(se->instances[i]);
	}
}

void
renderCubesSector(SectorExt *se)
{
	int i;
	if(se == nil)
		return;

	cubeMat->color.red = 255;
	cubeMat->color.green = 0;
	cubeMat->color.blue = 0;
	pAmbient->setColor(1.0f, 1.0f, 1.0f);

	for(i = 0; i < se->numInstances; i++){
		sGeomInstance *inst = &se->sect->sectionA[i];
		float x = halfFloatToFloat(inst->bound[0]);
		float y = halfFloatToFloat(inst->bound[1]);
		float z = halfFloatToFloat(inst->bound[2]);
		rw::V3d pos = { x, y, z };
		if(se->dummies[i] == nil){
			float r = halfFloatToFloat(inst->bound[3]);

			rw::Atomic *a = rw::Atomic::create();
			rw::Frame *f = rw::Frame::create();
			a->setFrame(f);
			a->setGeometry(cubeGeo, 0);

			f->translate(&pos, rw::COMBINEREPLACE);
			se->dummies[i] = a;
		}
		se->dummies[i]->render();
	}
}

/*
void
dumpInstances(void)
{
	int i, j;
	SectorExt *se;

	for(i = 0; i < gLevel->numStreamingSectors; i++){
		se = gLevel->sectors[i];
		for(j = 0; j < se->numInstances; j++){
			sGeomInstance *inst = &se->sect->sectionA[j];
			float x = halfFloatToFloat(inst->bound[0]);
			float y = halfFloatToFloat(inst->bound[1]);
			float z = halfFloatToFloat(inst->bound[2]);
			printf("%d %d %f %f %f : %f %f %f : %d\n", inst->id & 0x7FFF, inst->resId, x, y, z,
				inst->matrix.pos.x, inst->matrix.pos.y, inst->matrix.pos.z,
				i);
		}
	}
}
*/



/* LCS sectors:
...
483 12, 0DA60C20 0DA61170 0DA622F0 0DA622F0 0DA62570 0DA62570 0DA62570
484 4e, 0E401A20 0E401FC0 0E406840 0E409450 0E40DEB0 0E40DF50 0E40E540
485 67a, 0E511380 0E511920 0E516560 0E517C30 0E51B1F0 0E51B2E0 0E51B470
486 67a, 0E683120 0E6836C0 0E6885D0 0E68BAF0 0E68FD30 0E68FE70 0E6905F0
487 67a, 0E810D00 0E8112A0 0E815BC0 0E816DE0 0E819A40 0E819B30 0E819C20
488 67a, 0E87C830 0E87CDD0 0E881BF0 0E882640 0E886740 0E8867E0 0E886A60
489 0, 000002D9 0DA62B70 001C0001 81F002D7 3C003C00 90EA001C A3B083D9
...
*/
/* VCS sectors:
...
622 0, 0CB46BF8 0CB478C8 0CB47AA8 0CB48E08 0CB48E08 0CB48E08 0CB48E08
623 1916, 0D2D31F0 0D2D3F60 0D2D4690 0D2DD650 0D2DDA60 0D2DF090 0D2DF090
624 12, 0D328450 0D3291C0 0D329850 0D331B90 0D331F00 0D332F90 0D332F90
625 12, 0D438B30 0D439850 0D439F80 0D43EFD0 0D43F070 0D4433F0 0D4433F0
626 12, 0D49EBB0 0D49F920 0D49FEC0 0D4A6B80 0D4A70D0 0D4A8110 0D4A8110
627 12, 0D57ADC0 0D57BB30 0D57BF90 0D583240 0D583650 0D584910 0D584910
628 12, 0D65FF40 0D660CB0 0D661020 0D667BA0 0D667E70 0D669040 0D669040
629 12, 0D6E9AE0 0D6EA710 0D6EAF80 0D6F39F0 0D6F3DB0 0D6FBFB0 0D6FBFB0
630 12, 0D7CDB90 0D7CE900 0D7CF300 0D7D8EA0 0D7D9300 0D7DA390 0D7DA390
631 12, 0D840080 0D840DA0 0D8412A0 0D844EA0 0D844EA0 0D845FD0 0D845FD0
632 12, 0D8F5C60 0D8F69D0 0D8F7100 0D8FF2B0 0D8FF580 0D901560 0D901560
633 12, 0D9D8900 0D9D9670 0D9D9850 0D9E06A0 0D9E0880 0D9E0E70 0D9E0E70
634 12, 0DA658D0 0DA665F0 0DA66B90 0DA6ADD0 0DA6AE20 0DA6EF20 0DA6EF20
635 12, 0DB1EC50 0DB1F970 0DB1FF10 0DB24150 0DB241A0 0DB282A0 0DB282A0
636 12, 0DC21230 0DC21FA0 0DC22810 0DC2B7D0 0DC2B9B0 0DC2C4A0 0DC2C4A0
637 12, 0CB66270 0CB66FE0 0CB67990 0CB70810 0CB70810 0CB70860 0CB70860
638 12, 0CB722B0 0CB73020 0CB739D0 0CB7C850 0CB7C850 0CB7C8A0 0CB7C8A0
639 12, 0CB90AB0 0CB91820 0CB921D0 0CB9B050 0CB9B050 0CB9B0A0 0CB9B0A0
640 12, 0CBB2868 0CBB35D8 0CBB3F88 0CBBCE08 0CBBCE08 0CBBCE58 0CBBCE58
641 12, 0CBD8C50 0CBD99C0 0CBDA370 0CBE31F0 0CBE31F0 0CBE3240 0CBE3240
642 12, 0CBFB818 0CBFC588 0CBFCF38 0CC05DB8 0CC05DB8 0CC05E08 0CC05E08
643 12, 0CC356D8 0CC36448 0CC36DF8 0CC3FC78 0CC3FC78 0CC3FD18 0CC3FD18
644 12, 0CC69D58 0CC6AAC8 0CC6B478 0CC742F8 0CC742F8 0CC74398 0CC74398
645 12, 0CC7F6C8 0CC80438 0CC80DE8 0CC89C68 0CC89C68 0CC89D08 0CC89D08
646 12, 0CC9A1A8 0CC9AF18 0CC9B8C8 0CCA4748 0CCA4748 0CCA47E8 0CCA47E8
647 12, 0CCC2CA0 0CCC3A10 0CCC43C0 0CCCD240 0CCCD240 0CCCD2E0 0CCCD2E0
648 12, 0CD03860 0CD045D0 0CD04F80 0CD0DE00 0CD0DE00 0CD0DEA0 0CD0DEA0
649 12, 0CD1A5F0 0CD1B360 0CD1BD10 0CD24B90 0CD24B90 0CD24C30 0CD24C30
650 12, 0CD39AB0 0CD3A820 0CD3B1D0 0CD44050 0CD44050 0CD440F0 0CD440F0
651 12, 0CD73828 0CD74598 0CD74F48 0CD7DDC8 0CD7DDC8 0CD7DE68 0CD7DE68
652 12, 0CD8E168 0CD8EED8 0CD8F888 0CD98708 0CD98708 0CD98758 0CD98758
653 12, 0CDD27B0 0CDD3520 0CDD3ED0 0CDDCD50 0CDDCD50 0CDDCDA0 0CDDCDA0
654 12, 0CDE1928 0CDE2698 0CDE3048 0CDEBEC8 0CDEBEC8 0CDEBF18 0CDEBF18
655 12, 0CE142F8 0CE15068 0CE15A18 0CE1E898 0CE1E898 0CE1E8E8 0CE1E8E8
656 12, 0CE377F8 0CE38568 0CE38F18 0CE41D98 0CE41D98 0CE41DE8 0CE41DE8
657 12, 0CE6EBD8 0CE6F948 0CE702F8 0CE79178 0CE79178 0CE791C8 0CE791C8
658 12, 0CE7B540 0CE7C2B0 0CE7CC60 0CE85AE0 0CE85AE0 0CE85B80 0CE85B80
659 12, 0CEFF430 0CF001A0 0CF00B50 0CF099D0 0CF099D0 0CF09A70 0CF09A70
660 12, 0CF0B980 0CF0C6F0 0CF0D0A0 0CF15F20 0CF15F20 0CF15FC0 0CF15FC0
661 12, 0CF27660 0CF283D0 0CF28D80 0CF31C00 0CF31C00 0CF31CA0 0CF31CA0
662 12, 0CF424E0 0CF43250 0CF43C00 0CF4CA80 0CF4CA80 0CF4CB20 0CF4CB20
663 12, 0CF607D0 0CF61540 0CF61EF0 0CF6AD70 0CF6AD70 0CF6AE10 0CF6AE10
664 12, 0CFC19F0 0CFC2760 0CFC3110 0CFCBF90 0CFCBF90 0CFCC030 0CFCC030
665 12, 0CFEB4A8 0CFEC218 0CFECBC8 0CFF5A48 0CFF5A48 0CFF5AE8 0CFF5AE8
666 12, 0D04BFE8 0D04CD58 0D04D708 0D056588 0D056588 0D056628 0D056628
667 12, 0D0B40C0 0D0B4E30 0D0B57E0 0D0BE660 0D0BE660 0D0BE6B0 0D0BE6B0
668 12, 0D0F3900 0D0F4670 0D0F5020 0D0FDEA0 0D0FDEA0 0D0FDEF0 0D0FDEF0
669 12, 0DC700A8 0DC70E18 0DC717C8 0DC7A648 0DC7A648 0DC7A698 0DC7A698
670 12, 0DCBEF38 0DCBFCA8 0DCC0658 0DCC94D8 0DCC94D8 0DCC9528 0DCC9528
671 12, 0DCDEB00 0DCDF870 0DCE0220 0DCE90A0 0DCE90A0 0DCE90F0 0DCE90F0
672 12, 0DD34DC8 0DD35B38 0DD364E8 0DD3F368 0DD3F368 0DD3F3B8 0DD3F3B8
673 12, 0DD41B00 0DD42870 0DD43220 0DD4C0A0 0DD4C0A0 0DD4C140 0DD4C140
674 12, 0DD59050 0DD59DC0 0DD5A770 0DD635F0 0DD635F0 0DD63690 0DD63690
675 12, 0DD98080 0DD98DF0 0DD997A0 0DDA2620 0DDA2620 0DDA26C0 0DDA26C0
676 12, 0DDFC2E0 0DDFD050 0DDFDA00 0DE06880 0DE06880 0DE06920 0DE06920
677 12, 0EC402E0 0EC41050 0EC41A00 0EC4A880 0EC4A880 0EC4A920 0EC4A920
678 12, 0DE13E50 0DE14BC0 0DE15570 0DE1E3F0 0DE1E3F0 0DE1E490 0DE1E490
679 12, 0DE23A30 0DE247A0 0DE25150 0DE2DFD0 0DE2DFD0 0DE2E070 0DE2E070
680 12, 0DE5C7D0 0DE5D540 0DE5DEF0 0DE66D70 0DE66D70 0DE66E10 0DE66E10
681 12, 0ECD18D0 0ECD2640 0ECD2FF0 0ECDBE70 0ECDBE70 0ECDBF10 0ECDBF10
682 7ffd, 00000000 00000000 001C0001 00000B01 3C000149 002A3C00 B1F0BE10
...
"flags" change
*/
