#include "storiesconv.h"

int32
mapID(int32 id)
{
	if(id == 255) return -1;
	if(id > 0x80) id |= 0x1300;
	return id;
}

static RslNode*
findNode(RslNode *f, int32 id)
{
	if(f == NULL) return NULL;
	if((f->nodeId & 0xFF) == (id & 0xFF))
		return f;
	RslNode *ff = findNode(f->next, id);
	if(ff) return ff;
	return findNode(f->child, id);
}

static RslNode*
findChild(RslNode *f)
{
	for(RslNode *c = f->child; c; c = c->next)
		if(c->nodeId < 0)
			return c;
	return NULL;
}

int32 nextId;

struct Node {
	int32 id;
	int32 parent;
};

Frame*
convertFrame(RslNode *f)
{
	Frame *rwf = Frame::create();
	rwf->matrix.right.x =  f->modelling.right.x;
	rwf->matrix.right.y =  f->modelling.right.y;
	rwf->matrix.right.z =  f->modelling.right.z;
	rwf->matrix.up.x =  f->modelling.up.x;
	rwf->matrix.up.y =  f->modelling.up.y;
	rwf->matrix.up.z =  f->modelling.up.z;
	rwf->matrix.at.x =  f->modelling.at.x;
	rwf->matrix.at.y =  f->modelling.at.y;
	rwf->matrix.at.z = f->modelling.at.z;
	rwf->matrix.pos.x = f->modelling.pos.x;
	rwf->matrix.pos.y = f->modelling.pos.y;
	rwf->matrix.pos.z = f->modelling.pos.z;

	if(f->name)
		strncpy(gta::getNodeName(rwf), f->name, 24);

#ifdef LCS
	HAnimData *hanim = PLUGINOFFSET(HAnimData, rwf, hAnimOffset);
	hanim->id = f->nodeId;
	if(f->hier){
		int32 numNodes = f->hier->numNodes;
		int32 *nodeFlags = new int32[numNodes];
		int32 *nodeIDs = new int32[numNodes];

		nextId = 2000;
		Node *nodehier = new Node[numNodes];
		int32 stack[100];
		int32 sp = 0;
		stack[sp] = -1;
		// Match up nodes with frames to fix and assign IDs
		// NOTE: assignment can only work reliably when not more
		//       than one child node needs an ID
		for(int32 i = 0; i < numNodes; i++){
			RslTAnimNodeInfo *ni = &f->hier->pNodeInfo[i];
			Node *n = &nodehier[i];
			n->parent = stack[sp];
			if(ni->flags & HAnimHierarchy::PUSH)
				sp++;
			stack[sp] = i;
			RslNode *ff = findNode(f, (uint8)ni->id);
			n->id = ff->nodeId;
			if(n->id < 0){
				ff = findNode(f, nodehier[n->parent].id);
				ff = findChild(ff);
				n->id = ff->nodeId = nextId++;
			}
			//printf("%d %s %d %d\n", i, ff->name, n->id, n->parent);
			if(ni->flags & HAnimHierarchy::POP)
				sp--;

			nodeFlags[i] = ni->flags;
			nodeIDs[i] = n->id;
		}

		HAnimHierarchy *hier = HAnimHierarchy::create(numNodes, nodeFlags, nodeIDs, f->hier->flags, f->hier->maxKeyFrameSize);
		hanim->hierarchy = hier;

		delete[] nodeFlags;
		delete[] nodeIDs;
		delete[] nodehier;
	}
#else
	HAnimData *hanim = PLUGINOFFSET(HAnimData, rwf, hAnimOffset);
	hanim->id = -1;
	if(f->hier){
		int32 numNodes = f->hier->numNodes;
		int32 *nodeFlags = new int32[numNodes];
		int32 *nodeIDs = new int32[numNodes];

		nextId = 2000;
		Node *nodehier = new Node[numNodes];
		int32 stack[100];
		int32 sp = 0;
		stack[sp] = -1;
		// Match up nodes with frames to fix and assign IDs
		// NOTE: assignment can only work reliably when not more
		//       than one child node needs an ID
		for(int32 i = 0; i < numNodes; i++){
			RslTAnimNodeInfo *ni = &f->hier->pNodeInfo[i];
			printf("%d %d %d %p\n", ni->id, ni->index, ni->flags, ni->frame);
			//Node *n = &nodehier[i];
			//n->parent = stack[sp];
			if(ni->flags & HAnimHierarchy::PUSH)
				sp++;
			stack[sp] = i;
			//RslNode *ff = findNode(f, (uint8)ni->id);
			//n->id = ff->nodeId;
			//if(n->id < 0){
			//	ff = findNode(f, nodehier[n->parent].id);
			//	ff = findChild(ff);
			//	n->id = ff->nodeId = nextId++;
			//}
			//printf("%d %s %d %d\n", i, ff->name, n->id, n->parent);
			if(ni->flags & HAnimHierarchy::POP)
				sp--;

			nodeFlags[i] = ni->flags;
			nodeIDs[i] = ni->id; //n->id;
		}

#ifdef LCS
		int kfsz = f->hier->maxKeyFrameSize
#else
		int kfsz = 0x24;
#endif
		HAnimHierarchy *hier = HAnimHierarchy::create(numNodes, nodeFlags, nodeIDs, f->hier->flags, kfsz);
		hanim->hierarchy = hier;

		delete[] nodeFlags;
		delete[] nodeIDs;
		delete[] nodehier;
	}
#endif
	return rwf;
}

Texture*
convertTexture(RslTexture *t)
{
	Texture *tex = Texture::read(t->name, t->mask);
	//tex->refCount++;	// ??
	if(tex && tex->refCount == 1)
		tex->filterAddressing = (Texture::WRAP << 12) | (Texture::WRAP << 8) | Texture::LINEAR;
	return tex;
}

Material*
convertMaterial(RslMaterial *m)
{
	Material *rwm;
	rwm = Material::create();

	rwm->color = m->color;
	if(m->texture){
		rwm->texture = convertTexture(m->texture);
		rwm->refCount++;
	}

	if(m->matfx){
		MatFX::setEffects(rwm, m->matfx->effectType);
		MatFX *matfx = MatFX::get(rwm);
		matfx->setEnvCoefficient(m->matfx->env.intensity);
		if(m->matfx->env.texture)
			matfx->setEnvTexture(convertTexture(m->matfx->env.texture));
		*PLUGINOFFSET(MatFX*, rwm, matFXGlobals.materialOffset) = matfx;
	}
	return rwm;
}

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

void
convertMesh(Geometry *rwg, RslGeometry *g, int32 ii)
{
	sPs2Geometry *resHeader = (sPs2Geometry*)(g+1);
	sPs2GeometryMesh *inst = (sPs2GeometryMesh*)(resHeader+1);
	int32 numInst = resHeader->size >> 20;
	uint8 *p = (uint8*)(inst+numInst);
	inst += ii;
	p += inst->dmaPacket;
	Mesh *m = &rwg->meshHeader->getMeshes()[inst->matID];
	ps2::Vertex v;
	uint32 mask = 0x1001;	// tex coords, vertices
	if(rwg->flags & Geometry::NORMALS)
		mask |= 0x10;
	if(rwg->flags & Geometry::PRELIT)
		mask |= 0x100;
	Skin *skin = *PLUGINOFFSET(Skin*, rwg, skinGlobals.geoOffset);
	if(skin)
		mask |= 0x10000;

	int16 *vuVerts = NULL;
	int8 *vuNorms = NULL;
	uint8 *vuTex = NULL;
	uint16 *vuCols = NULL;
	uint32 *vuSkin = NULL;

	uint32 *w = (uint32*)p;
	uint32 *end = (uint32*)(p + ((w[0] & 0xFFFF) + 1)*0x10);
	w += 4;
	int32 nvert;
	bool firstInst = m->numIndices == 0;
	bool firstBatch = 1;
	while(w < end){
		/* Get data pointers */
//if(firstInst)
//printf("$$$$$$$$$$\n");
//if(firstBatch)
//printf("##########\n");

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

		if(rwg->flags & Geometry::PRELIT){
			assert((w[0] & 0xFF004000) == 0x6F000000);
			vuCols = (uint16*)(w+1);
			if(!firstBatch) vuCols += 2;
			w = skipUnpack(w);
		}

		if(rwg->flags & Geometry::NORMALS){
			assert((w[0] & 0xFF004000) == 0x6A000000);
			vuNorms = (int8*)(w+1);
			if(!firstBatch) vuNorms += 2*3;
			w = skipUnpack(w);
		}

		if(skin){
			assert((w[0] & 0xFF004000) == 0x6C000000);
			vuSkin = w+1;
			if(!firstBatch) vuSkin += 2*4;
			w = skipUnpack(w);
		}

		assert(w[0] == 0x14000006);	// MSCAL
		w++;
		while(w[0] == 0) w++;

		/* Insert Data */
		for(int32 i = 0; i < nvert; i++){
			v.p.x = vuVerts[0]/32768.0f*resHeader->scale[0] + resHeader->pos[0];
			v.p.y = vuVerts[1]/32768.0f*resHeader->scale[1] + resHeader->pos[1];
			v.p.z = vuVerts[2]/32768.0f*resHeader->scale[2] + resHeader->pos[2];
			v.t.u = vuTex[0]/128.0f*inst->uvScale[0];
			v.t.v = vuTex[1]/128.0f*inst->uvScale[1];
			if(mask & 0x10){
				v.n.x = vuNorms[0]/127.0f;
				v.n.y = vuNorms[1]/127.0f;
				v.n.z = vuNorms[2]/127.0f;
			}
			if(mask & 0x100){
				v.c.red   = (vuCols[0] & 0x1f) * 255 / 0x1F;
				v.c.green = (vuCols[0]>>5 & 0x1f) * 255 / 0x1F;
				v.c.blue  = (vuCols[0]>>10 & 0x1f) * 255 / 0x1F;
				v.c.alpha = vuCols[0]&0x8000 ? 0xFF : 0;
			}
			if(mask & 0x10000){
				for(int j = 0; j < 4; j++){
					((uint32*)v.w)[j] = vuSkin[j] & ~0x3FF;
					v.i[j] = vuSkin[j] >> 2;
					//if(v.i[j]) v.i[j]--;
					if(v.w[j] == 0.0f) v.i[j] = 0;
				}
			}

			int32 idx = ps2::findVertexSkin(rwg, NULL, mask, &v);
			if(idx < 0)
				idx = rwg->numVertices++;

			// For some geometry
			// Insts normally overlap by two vertices (same as batches).
			// For some reason each inst is assumed to end in an odd vertex.
			// This is enforced by doubling the last vertex if necessary,
			// in which case we have one more overlapping vertex.
//			if(i == 0 && firstBatch && !firstInst){
//				m->numIndices -= 2;
//				if(inst->numTriangles % 2)
//					m->numIndices--;
//			}

//printf("%d %d\n", m->numIndices % 2, idx);

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
			ps2::insertVertexSkin(rwg, idx, mask, &v);

			vuVerts += 3;
			vuTex += 2;
			vuNorms += 3;
			vuCols++;
			vuSkin += 4;
		}
		firstInst = 0;
		firstBatch = 0;
	}
}

Atomic*
convertAtomic(RslElement *atomic)
{
	Atomic *rwa = Atomic::create();
	RslGeometry *g = atomic->geometry;
	Geometry *rwg = Geometry::create(0, 0, 0);
	rwa->geometry = rwg;

	*PLUGINOFFSET(RslElement*, rwa, atmOffset) = atomic;

	rwg->matList.numMaterials = g->matList.numMaterials;
	rwg->matList.space = rwg->matList.numMaterials;
	rwg->matList.materials = rwMallocT(Material*, rwg->matList.numMaterials, MEMDUR_EVENT | ID_MATERIAL);
	for(int32 i = 0; i < rwg->matList.numMaterials; i++)
		rwg->matList.materials[i] = convertMaterial(g->matList.materials[i]);

	sPs2Geometry *resHeader = (sPs2Geometry*)(g+1);
	sPs2GeometryMesh *inst = (sPs2GeometryMesh*)(resHeader+1);
	int32 numInst = resHeader->size >> 20;

	// allocate empty meshes
	rwg->allocateMeshes(rwg->matList.numMaterials, 0, 1);
	rwg->meshHeader->flags = MeshHeader::TRISTRIP;
	Mesh *meshes = rwg->meshHeader->getMeshes();

	for(int32 i = 0; i < numInst; i++){
		rwg->numVertices += inst[i].numTriangles+2;
		meshes[inst[i].matID].numIndices += inst[i].numTriangles+2 +3;
	}
	for(uint32 i = 0; i < rwg->meshHeader->numMeshes; i++){
		meshes[i].material = rwg->matList.materials[i];
		rwg->meshHeader->totalIndices += meshes[i].numIndices;
	}
	rwg->flags = Geometry::TRISTRIP |
	                Geometry::LIGHT;
	if(rwg->hasColoredMaterial())
		rwg->flags |= Geometry::MODULATE;
	if(resHeader->flags & 0x1)
		rwg->flags |= Geometry::POSITIONS;
	if(resHeader->flags & 0x2)
		rwg->flags |= Geometry::NORMALS;
	if(resHeader->flags & 0x4)
		rwg->flags |= Geometry::TEXTURED;
	if(resHeader->flags & 0x8)
		rwg->flags |= Geometry::PRELIT;
	rwg->numTexCoordSets = 1;

	rwg->numTriangles = rwg->meshHeader->guessNumTriangles();
	rwg->allocateData();
	rwg->allocateMeshes(rwg->meshHeader->numMeshes, rwg->meshHeader->totalIndices, 0);
	meshes = rwg->meshHeader->getMeshes();

	Skin *skin = NULL;
	if(resHeader->flags & 0x10)
		assert(g->skin);
	if(g->skin){
		skin = rwNewT(Skin, 1, MEMDUR_EVENT | ID_SKIN);
		*PLUGINOFFSET(Skin*, rwg, skinGlobals.geoOffset) = skin;
		skin->init(g->skin->numBones, g->skin->numBones, rwg->numVertices);
		memcpy(skin->inverseMatrices, g->skin->invMatrices, skin->numBones*64);
	}

	for(uint32 i = 0; i < rwg->meshHeader->numMeshes; i++)
		meshes[i].numIndices = 0;
	rwg->meshHeader->totalIndices = rwg->numVertices = 0;
	for(int32 i = 0; i < numInst; i++)
		convertMesh(rwg, g, i);
	for(uint32 i = 0; i < rwg->meshHeader->numMeshes; i++)
		rwg->meshHeader->totalIndices += meshes[i].numIndices;
	if(skin){
		skin->findNumWeights(rwg->numVertices);
		skin->findUsedBones(rwg->numVertices);
	}
	rwg->calculateBoundingSphere();
	rwg->generateTriangles();
	rwg->removeUnusedMaterials();
	return rwa;
}

RslElement*
collectElements(RslElement *atomic, void *data)
{
	RslElement ***alist = (RslElement***)data;
	*(*alist)++ = atomic;
	return atomic;
}

Clump*
convertClump(RslElementGroup *c)
{
	Clump *rwc;
	Frame *rwf;
	Atomic *rwa;
	rslNodeList frameList;

	rwc = Clump::create();
	rslNodeListInitialize(&frameList, (RslNode*)c->object.parent);
	Frame **rwframes = new Frame*[frameList.numNodes];
	for(int32 i = 0; i < frameList.numNodes; i++){
		rwf = convertFrame(frameList.frames[i]);
		rwframes[i] = rwf;
		void *par = frameList.frames[i]->object.parent;
		int32 parent = findPointer(par, (void**)frameList.frames, frameList.numNodes);
		if(parent >= 0)
			rwframes[parent]->addChild(rwf);
	}
	rwc->object.parent = rwframes[0];

	int32 numElements = RslElementGroupGetNumElements(c);
	RslElement **alist = new RslElement*[numElements];
	RslElement **ap = &alist[0];
	RslElementGroupForAllElements(c, collectElements, &ap);
	for(int32 i = 0; i < numElements; i++){
		rwa = convertAtomic(alist[i]);
		int32 fi = findPointer(alist[i]->object.object.parent, (void**)frameList.frames, frameList.numNodes);
		rwa->setFrame(rwframes[fi]);
		rwc->addAtomic(rwa);
	}

	delete[] alist;
	delete[] rwframes;
	delete[] frameList.frames;
	return rwc;
}


RslElement*
makeTextures(RslElement *atomic, void*)
{
	RslGeometry *g = atomic->geometry;
	RslMaterial *m;
	for(int32 i = 0; i < g->matList.numMaterials; i++){
		m = g->matList.materials[i];
		if(m->texname){
			RslTexture *tex = RslTextureCreate(NULL);
			strncpy(tex->name, m->texname, 32);
			strncpy(tex->mask, m->texname, 32);
			m->texture = tex;
		}
		if(m->matfx && m->matfx->effectType == MatFX::ENVMAP &&
		   m->matfx->env.texname){
			RslTexture *tex = RslTextureCreate(NULL);
			strncpy(tex->name, m->matfx->env.texname, 32);
			strncpy(tex->mask, m->matfx->env.texname, 32);
			m->matfx->env.texture = tex;
		}
	}
	return atomic;
}

void
moveAtomics(Frame *f)
{
	static char *names[] = { "", "hi_ok", "hi_dam" };
	if(f == NULL) return;
	int n = f->objectList.count();
	if(n > 1){
		char *oldname = gta::getNodeName(f);
		ObjectWithFrame **objs = new ObjectWithFrame*[n];
		int i = 0;
		FORLIST(lnk, f->objectList){
			ObjectWithFrame *obj = ObjectWithFrame::fromFrame(lnk);
			assert(obj->object.type == Atomic::ID);
			objs[i] = obj;
			obj->setFrame(NULL);
			i++;
		}
		for(i = 0; i < n; i++){
			Frame *ff = Frame::create();
			RslElement *rsla = *PLUGINOFFSET(RslElement*, objs[i], atmOffset);
			char *name = gta::getNodeName(ff);
			strncpy(name, oldname, 24);
			char *end = strrchr(name, '_');
			if(end){
				*(++end) = '\0';
				strcat(end, names[rsla->visIdFlag&3]);
			}
			f->addChild(ff);
			objs[i]->setFrame(ff);
		}
		delete[] objs;
	}
	moveAtomics(f->next);
	moveAtomics(f->child);
}

uint8*
getPalettePS2(RslRaster *raster)
{
	uint32 f = raster->ps2.flags;
	uint32 w = 1 << (f & 0x3F);
	uint32 h = 1 << (f>>6 & 0x3F);
	uint32 d = f>>12 & 0x3F;
	uint32 mip = f>>20 & 0xF;
	uint8 *data = raster->ps2.data;
	if(d > 8)
		return NULL;
	while(mip--){
		data += w*h*d/8;
		w /= 2;
		h /= 2;
	}
	return data;
}

uint8*
getTexelPS2(RslRaster *raster, int32 n)
{
	uint32 f = raster->ps2.flags;
	uint32 w = 1 << (f & 0x3F);
	uint32 h = 1 << (f>>6 & 0x3F);
	uint32 d = f>>12 & 0x3F;
	uint8 *data = raster->ps2.data;
	for(int32 i = 0; i < n; i++){
		data += w*h*d/8;
		w /= 2;
		h /= 2;
	}
	return data;
}

void
convertCLUT(uint8 *texels, uint32 w, uint32 h)
{
	static uint8 map[4] = { 0x00, 0x10, 0x08, 0x18 };
	for (uint32 i = 0; i < w*h; i++)
		texels[i] = (texels[i] & ~0x18) | map[(texels[i] & 0x18) >> 3];
}

void
unswizzle8(uint8 *dst, uint8 *src, uint32 w, uint32 h)
{
	for (uint32 y = 0; y < h; y++)
		for (uint32 x = 0; x < w; x++) {
			int32 block_loc = (y&(~0xF))*w + (x&(~0xF))*2;
			uint32 swap_sel = (((y+2)>>2)&0x1)*4;
			int32 ypos = (((y&(~3))>>1) + (y&1))&0x7;
			int32 column_loc = ypos*w*2 + ((x+swap_sel)&0x7)*4;
			int32 byte_sum = ((y>>1)&1) + ((x>>2)&2);
			uint32 swizzled = block_loc + column_loc + byte_sum;
			dst[y*w+x] = src[swizzled];
		}
}

void
unswizzle16(uint16 *dst, uint16 *src, int32 w, int32 h)
{
	for(int y = 0; y < h; y++)
		for(int x = 0; x < w; x++){
			int32 pageX = x & (~0x3f);
			int32 pageY = y & (~0x3f);
			int32 pages_horz = (w+63)/64;
			int32 pages_vert = (h+63)/64;
			int32 page_number = (pageY/64)*pages_horz + (pageX/64);
			int32 page32Y = (page_number/pages_vert)*32;
			int32 page32X = (page_number%pages_vert)*64;
			int32 page_location = (page32Y*h + page32X)*2;
			int32 locX = x & 0x3f;
			int32 locY = y & 0x3f;
			int32 block_location = (locX&(~0xf))*h + (locY&(~0x7))*2;
			int32 column_location = ((y&0x7)*h + (x&0x7))*2;
			int32 short_num = (x>>3)&1;       // 0,1
			uint32 swizzled = page_location + block_location +
			                  column_location + short_num;
			dst[y*w+x] = src[swizzled];
		}
}

bool32 unswizzle = 1;

void
convertTo32(uint8 *out, uint8 *pal, uint8 *tex,
            uint32 w, uint32 h, uint32 d, bool32 swiz)
{
	uint32 x;
	if(d == 32){
		//uint32 *dat = new uint32[w*h];
		//if(swiz && unswizzle)
		//	unswizzle8_hack(dat, (uint32*)tex, w, h);
		//else
		//	memcpy(dat, tex, w*h*4);
		//tex = (uint8*)dat;
		for(uint32 i = 0; i < w*h; i++){
			out[i*4+0] = tex[i*4+0];
			out[i*4+1] = tex[i*4+1];
			out[i*4+2] = tex[i*4+2];
			out[i*4+3] = tex[i*4+3]*255/128;
		}
		//delete[] dat;
	}
	if(d == 16) return;	// TODO
	if(d == 8){
		uint8 *dat = new uint8[w*h];
		if(swiz && unswizzle)
			unswizzle8(dat, tex, w, h);
		else
			memcpy(dat, tex, w*h);
		tex = dat;
		convertCLUT(tex, w, h);
		for(uint32 i = 0; i < h; i++)
			for(uint32 j = 0; j < w; j++){
				x = *tex++;
				*out++ = pal[x*4+0];
				*out++ = pal[x*4+1];
				*out++ = pal[x*4+2];
				*out++ = pal[x*4+3]*255/128;
			}
		delete[] dat;
	}
	if(d == 4){
		uint8 *dat = new uint8[w*h];
		for(uint32 i = 0; i < w*h/2; i++){
			dat[i*2+0] = tex[i] & 0xF;
			dat[i*2+1] = tex[i] >> 4;
		}
		if(swiz && unswizzle){
			uint8 *tmp = new uint8[w*h];
			unswizzle8(tmp, dat, w, h);
			delete[] dat;
			dat = tmp;
		}
		tex = dat;
		for(uint32 i = 0; i < h; i++)
			for(uint32 j = 0; j < w; j++){
				x = *tex++;
				*out++ = pal[x*4+0];
				*out++ = pal[x*4+1];
				*out++ = pal[x*4+2];
				*out++ = pal[x*4+3]*255/128;
			}
		delete[] dat;
	}
}

RslTexture *dumpTextureCB(RslTexture *texture, void*)
{
	uint32 f = texture->raster->ps2.flags;
	uint32 w = 1 << (f & 0x3F);
	uint32 h = 1 << (f>>6 & 0x3F);
	uint32 d = f>>12 & 0x3F;
	uint32 mip = f>>20 & 0xF;
	uint32 swizmask = f>>24;
	uint8 *palette = getPalettePS2(texture->raster);
	uint8 *texels = getTexelPS2(texture->raster, 0);
	printf(" %x %x %x %x %x %s\n", w, h, d, mip, swizmask, texture->name);
	Image *img = Image::create(w, h, 32);
	img->allocate();
	convertTo32(img->pixels, palette, texels, w, h, d, swizmask&1);
	char *name = new char[strlen(texture->name)+5];
	strcpy(name, texture->name);
	strcat(name, ".tga");
	writeTGA(img, name);
	img->destroy();
	delete[] name;
	return texture;
}

rw::Raster*
convertRasterPS2(RslRasterPS2 *ras)
{
	uint32 f = ras->flags;
	uint32 logw = f & 0x3F;
	uint32 logh = f>>6 & 0x3F;
	uint32 w = 1 << logw;
	uint32 h = 1 << logh;
	uint32 d = f>>12 & 0x3F;
	//uint32 mip = f>>20 & 0xF;
	uint32 swizmask = f>>24;

	uint8 *palette = getPalettePS2((RslRaster*)ras);
	uint8 *texels = getTexelPS2((RslRaster*)ras, 0);


	uint8 *convtex = NULL;
	if(d == 4){
		convtex = rwNewT(uint8, w*h, 0);
		for(uint32 i = 0; i < w*h/2; i++){
			int32 a = texels[i] & 0xF;
			int32 b = texels[i] >> 4;
			convtex[i*2+0] = a;
			convtex[i*2+1] = b;
		}
		if(swizmask & 1 && unswizzle){
			uint8 *tmp = rwNewT(uint8, w*h, 0);
			unswizzle8(tmp, convtex, w, h);
			rwFree(convtex);
			convtex = tmp;
		}
	}else if(d == 8){
		convtex = rwNewT(uint8, w*h, 0);
		if(swizmask & 1 && unswizzle)
			unswizzle8(convtex, texels, w, h);
		else
			memcpy(convtex, texels, w*h);
		convertCLUT(convtex, w, h);
	}

	rw::Image *img = rw::Image::create(w, h, d);
	img->allocate();
	int32 pallen = d == 4 ? 16 :
	               d == 8 ? 256 : 0;
	if(pallen){
		uint8 *p = img->palette;
		for(int32 i = 0; i < pallen; i++){
			p[i*4+0] = palette[i*4+0];
			p[i*4+1] = palette[i*4+1];
			p[i*4+2] = palette[i*4+2];
			p[i*4+3] = palette[i*4+3]*255/128;
		}
		memcpy(img->pixels, convtex, w*h);
	}else
		memcpy(img->pixels, texels, w*h*d/8);

	if(d == 32){
		// texture is fucked, but pretend it isn't
		uint8 *p = img->pixels;
		uint8 c[3];
		for(uint32 i = 0; i < w*h; i++){
			c[0] = p[i*4+0];
			c[1] = p[i*4+1];
			c[2] = p[i*4+2];
			p[i*4+2] = c[0];
			p[i*4+1] = c[1];
			p[i*4+0] = c[2];
			p[i*4+3] = p[i*4+3]*255/128;
		}
	}
	img->unindex();
	rw::Raster *rwras = rw::Raster::createFromImage(img);
	img->destroy();
	rwFree(convtex);
	return rwras;
}

// This function is VERY BAD
// it was written when librw was younger. It should be
// replaced by something similar to the above.
RslTexture*
convertTexturePS2(RslTexture *texture, void *pData)
{
	TexDictionary *rwtxd = (TexDictionary*)pData;
	Texture *rwtex = Texture::create(NULL);
	RslRasterPS2 *ras = &texture->raster->ps2;

	strncpy(rwtex->name, texture->name, 32);
	strncpy(rwtex->mask, texture->mask, 32);
	rwtex->filterAddressing = 0x1102;

	uint32 f = ras->flags;
	uint32 w = 1 << (f & 0x3F);
	uint32 h = 1 << (f>>6 & 0x3F);
	uint32 d = f>>12 & 0x3F;
	//uint32 mip = f>>20 & 0xF;
	uint32 swizmask = f>>24;
	uint8 *palette = getPalettePS2(texture->raster);
	uint8 *texels = getTexelPS2(texture->raster, 0);

	int32 hasAlpha = 0;
	uint8 *convtex = NULL;
	if(d == 4){
		convtex = new uint8[w*h];
		for(uint32 i = 0; i < w*h/2; i++){
			int32 a = texels[i] & 0xF;
			int32 b = texels[i] >> 4;
			if(palette[a*4+3] != 0x80)
				hasAlpha = 1;
			if(palette[b*4+3] != 0x80)
				hasAlpha = 1;
			convtex[i*2+0] = a;
			convtex[i*2+1] = b;
		}
		if(swizmask & 1 && unswizzle){
			uint8 *tmp = new uint8[w*h];
			unswizzle8(tmp, convtex, w, h);
			delete[] convtex;
			convtex = tmp;
		}
	}else if(d == 8){
		convtex = new uint8[w*h];
		if(swizmask & 1 && unswizzle)
			unswizzle8(convtex, texels, w, h);
		else
			memcpy(convtex, texels, w*h);
		convertCLUT(convtex, w, h);
		for(uint32 i = 0; i < w*h; i++)
			if(palette[convtex[i]*4+3] != 0x80){
				hasAlpha = 1;
				break;
			}
	}

	int32 format = 0;
	switch(d){
	case 4:
	case 8:
		format |= Raster::PAL8;
		goto alpha32;

	case 32:
		for(uint32 i = 0; i < w*h; i++)
			if(texels[i*4+3] != 0x80){
				hasAlpha = 1;
				break;
			}
	alpha32:
		if(hasAlpha)
			format |= Raster::C8888;
		else
			format |= Raster::C888;
		break;
	default:
		fprintf(stderr, "unsupported depth %d\n", d);
		return NULL;
	}
	Raster *rwras = Raster::create(w, h, d == 4 ? 8 : d, format | 4, PLATFORM_D3D8);
	d3d::D3dRaster *d3dras = PLUGINOFFSET(d3d::D3dRaster, rwras, d3d::nativeRasterOffset);

	int32 pallen = d == 4 ? 16 :
	               d == 8 ? 256 : 0;
	if(pallen){
		uint8 *p = new uint8[256*4];
		for(int32 i = 0; i < pallen; i++){
			p[i*4+0] = palette[i*4+0];
			p[i*4+1] = palette[i*4+1];
			p[i*4+2] = palette[i*4+2];
			p[i*4+3] = palette[i*4+3]*255/128;
		}
		memcpy(d3dras->palette, p, 256*4);
		delete[] p;
	}

	uint8 *data = rwras->lock(0);
	if(d == 4 || d == 8)
		memcpy(data, convtex, w*h);
	else if(d == 32){
		// texture is fucked, but pretend it isn't
		for(uint32 i = 0; i < w*h; i++){
			data[i*4+2] = texels[i*4+0];
			data[i*4+1] = texels[i*4+1];
			data[i*4+0] = texels[i*4+2];
			data[i*4+3] = texels[i*4+3]*255/128;
		}
	}else
		memcpy(data, texels, w*h*d/8);
	rwras->unlock(0);
	rwtex->raster = rwras;
	delete[] convtex;

	rwtxd->add(rwtex);
	return texture;
}

TexDictionary*
convertTXD(RslTexList *txd)
{
	TexDictionary *rwtxd = TexDictionary::create();
	RslTexListForAllTextures(txd, convertTexturePS2, rwtxd);
	return rwtxd;
}

