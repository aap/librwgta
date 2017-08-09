#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <stdarg.h>

#include <args.h>
#include <rw.h>

#include <rwgta.h>

using namespace std;
using namespace rw;
#include "rsl.h"

char *argv0;
int32 atmOffset;

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
RslStream::relocate(void)
{
	uint32 off = (uint32)(uintptr)this->data;
	off -= 0x20;
	this->relocTab += off;
	this->globalTab += off;

	uint8 **rel = (uint8**)this->relocTab;
	uint8 ***tab = (uint8***)this->relocTab;
	for(uint32 i = 0; i < this->numRelocs; i++){
		rel[i] += off;
		*tab[i] += off;
	}
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
	printf("%08x %08x %s %d\n", frame->nodeId, frame->hier, frame->name, frame->hierId);
#else
	printf("%08x %08x %08x %s %d\n", frame->nodeId, frame->nodeId2, frame->hier, frame->name, frame->hierId);
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
	if(tex->refCount == 1)
		tex->filterAddressing = (Texture::WRAP << 12) | (Texture::WRAP << 8) | Texture::LINEAR;
	return tex;
}

Material*
convertMaterial(RslMaterial *m)
{
	Material *rwm;
	rwm = Material::create();

	rwm->color = m->color;
	if(m->texture)
		rwm->texture = convertTexture(m->texture);

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
	Mesh *m = &rwg->meshHeader->mesh[inst->matID];
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
	rwg->matList.materials = new Material*[rwg->matList.numMaterials];
	for(int32 i = 0; i < rwg->matList.numMaterials; i++)
		rwg->matList.materials[i] = convertMaterial(g->matList.materials[i]);

	sPs2Geometry *resHeader = (sPs2Geometry*)(g+1);
	sPs2GeometryMesh *inst = (sPs2GeometryMesh*)(resHeader+1);
	int32 numInst = resHeader->size >> 20;

	rwg->meshHeader = new MeshHeader;
	rwg->meshHeader->flags = 1;
	rwg->meshHeader->numMeshes = rwg->matList.numMaterials;
	rwg->meshHeader->mesh = new Mesh[rwg->meshHeader->numMeshes];
	rwg->meshHeader->totalIndices = 0;
	Mesh *meshes = rwg->meshHeader->mesh;
	for(uint32 i = 0; i < rwg->meshHeader->numMeshes; i++)
		meshes[i].numIndices = 0;


	for(int32 i = 0; i < numInst; i++){
		Mesh *m = &meshes[inst[i].matID];
		rwg->numVertices += inst[i].numTriangles+2;
		m->numIndices += inst[i].numTriangles+2 +3;
	}
	for(uint32 i = 0; i < rwg->meshHeader->numMeshes; i++){
		rwg->meshHeader->mesh[i].material = rwg->matList.materials[i];
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

	rwg->allocateData();
	rwg->meshHeader->allocateIndices();

	Skin *skin = NULL;
	if(resHeader->flags & 0x10)
		assert(g->skin);
	if(g->skin){
		skin = new Skin;
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
	dumpNodeCB((RslNode*)eg->object.parent, NULL);
	return convertClump(eg);
}

#ifdef LCS
#define NUMPRIM 25
#define NUMSEC 25
#else
#define NUMPRIM 30
#define NUMSEC 27
#endif

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
LoadAny(RslStream *rslstr, const char *name)
{
	Clump *c;
	c = LoadSimple(rslstr->data, "object", rslstr->numFuncs);
	if(c) return c;
	c = LoadElementGroup(rslstr->data);
	if(c) return c;
	c = LoadVehicle(rslstr->data);
	if(c) return c;
	c = LoadPed(rslstr->data);
	if(c) return c;
	return nil;
}

void
extractResourceVCS(RslStream *rslstr)
{
	int i;
	char tempname[128];
	RslElementGroup **C3dMarkers__m_pRslElementGroupArray;
	struct ResourceImage {
		char xxx[0xa8];
		RslElementGroup **markers;	// [32] in LCS
	} *res;

	res = (ResourceImage*)rslstr->data;
	C3dMarkers__m_pRslElementGroupArray = res->markers;

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
usage(void)
{
	fprintf(stderr, "%s [-v version] [-x] [-s] input [output.{txd|dff}]\n", argv0);
	fprintf(stderr, "\t-v RW version, e.g. 33004 for 3.3.0.4\n");
	fprintf(stderr, "\t-x extract textures to tga\n");
	fprintf(stderr, "\t-s don't unswizzle textures\n");
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
	default:
		usage();
	}ARGEND;

	if(argc < 1)
		usage();

	::World *world = NULL;
	Sector *sector = NULL;
	RslElementGroup *clump = NULL;
	RslElement *atomic = NULL;
	RslTexList *txd = NULL;
	Clump *rwc;

	StreamFile stream;
	if(stream.open(argv[0], "rb") == nil)
		panic("couldn't open %s", argv[0]);

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

	RslStream *rslstr;
	rslstr = new RslStream;
	stream.read(rslstr, 0x20);
	rslstr->data = new uint8[rslstr->fileSize-0x20];
	stream.read(rslstr->data, rslstr->fileSize-0x20);
	stream.close();
	rslstr->relocate();

	bool32 largefile;
	largefile = rslstr->dataSize > 0x1000000;

	if(rslstr->ident == WRLD_IDENT && largefile){	// hack
		world = (::World*)rslstr->data;

		int len = strlen(argv[0])+1;
		char filename[1024];
		strncpy(filename, argv[0], len);
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
		RslStreamHeader *h;
		uint32 i = 0;
		for(h = world->sectors->sector; h->ident == WRLD_IDENT; h++){
			sprintf(name, "world%04d.wrld", i++);
			strcat(filename, name);
			if(outf.open(filename, "wb") == 0)
				panic("couldn't open %s", filename);
			data = new uint8[h->fileEnd];
			memcpy(data, h, 0x20);
			stream.seek(h->root, 0);
			stream.read(data+0x20, h->fileEnd-0x20);
			outf.write(data, h->fileEnd);
			outf.close();
			filename[len-3] = '\0';
		}
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
		stream.close();
	}else if(rslstr->ident == WRLD_IDENT){	// sector
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
	}else if(rslstr->ident == MDL_IDENT){
		switch(mdltype){
		case MDL_SIMPLE:
			rwc = LoadSimple(rslstr->data, "object", rslstr->numFuncs);
			break;
		case MDL_ELEMENTGROUP:
			rwc = LoadElementGroup(rslstr->data);
			break;
		case MDL_VEHICLE:
			rwc = LoadVehicle(rslstr->data);
			break;
		case MDL_PED:
			rwc = LoadPed(rslstr->data);
			break;
		case MDL_ANY:
			rwc = LoadAny(rslstr, "object");
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
	}else if(rslstr->ident == TEX_IDENT){
		txd = (RslTexList*)rslstr->data;
	writeTxd:
		if(extract)
			RslTexListForAllTextures(txd, dumpTextureCB, NULL);
		TexDictionary *rwtxd = convertTXD(txd);
		if(stream.open(argc > 1 ? argv[1] : "out.txd", "wb") == nil)
			panic("couldn't open file %s", argc > 1 ? argv[1] : "out.txd");
		rwtxd->streamWrite(&stream);
		stream.close();
	}else if(rslstr->ident == GTAG_IDENT){
#ifdef VCS
		extractResourceVCS(rslstr);
#endif
	}else
		printf("unknown file type %X\n", rslstr->ident);

	return 0;
}
