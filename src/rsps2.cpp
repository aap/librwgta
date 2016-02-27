#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>
#include "rwgta.h"

namespace gta {

using namespace ps2;

PipeAttribute saXYZADC = {
	"saXYZADC",
	AT_V4_16 | AT_RW
};

PipeAttribute saUV = {
	"saUV",
	AT_V2_16 | AT_RW
};

PipeAttribute saUV2 = {
	"saUV2",
	AT_V4_16 | AT_RW
};

PipeAttribute saRGBA = {
	"saRGBA",
	AT_V4_8 | AT_UNSGN | AT_RW
};

PipeAttribute saRGBA2 = {
	"saRGBA2",
	AT_V4_16 | AT_UNSGN | AT_RW
};

PipeAttribute saNormal = {
	"saNormal",
	AT_V4_8 | AT_RW
};

PipeAttribute saWeights = {
	"saWeights",
	AT_V4_32 | AT_RW
};

static bool hasTex2(uint32 id)
{
	return id == 0x53f2008b;
}
static bool hasNormals(uint32 id)
{
	return id == 0x53f20085 || id == 0x53f20087 || id == 0x53f20089 ||
		id == 0x53f2008b || id == 0x53f2008d || id == 0x53f2008f;
}
static bool hasColors(uint32 id)
{
	return id == 0x53f20081 || id == 0x53f20083 || id == 0x53f2008d ||
	       id == 0x53f2008f || id == 0x53f20091 || id == 0x53f20093;
}
static bool hasColors2(uint32 id)
{
	return id == 0x53f20083 || id == 0x53f2008f || id == 0x53f20093;
}

static void
saPreCB(MatPipeline *p, Geometry *geo)
{
	allocateADC(geo);
	if(hasColors2(p->pluginData) && extraVertColorOffset)
		allocateExtraVertColors(geo);
	if(p->pluginData == 0x53f20089)
		skinPreCB(p, geo);
}

static void
saPostCB(MatPipeline *p, Geometry *geo)
{
	skinPostCB(p, geo);
}

int32
findSAVertex(Geometry *g, uint32 flags[], uint32 mask, SaVert *v)
{
	Skin *skin = *PLUGINOFFSET(Skin*, g, skinGlobals.offset);
	float32 *wghts = NULL;
	uint8 *inds    = NULL;
	if(skin){
		wghts = skin->weights;
		inds = skin->indices;
	}
	float32 *verts = g->morphTargets[0].vertices;
	float32 *tex0  = g->texCoords[0];
	float32 *tex1  = g->texCoords[1];
	float32 *norms = g->morphTargets[0].normals;
	uint8 *cols0   = g->colors;
	uint8 *cols1   = NULL;
	if(extraVertColorOffset)
		cols1 = PLUGINOFFSET(ExtraVertColors, g, extraVertColorOffset)->nightColors;

	for(int32 i = 0; i < g->numVertices; i++){
		uint32 flag = flags ? flags[i] : ~0;
		if(mask & flag & 0x1 &&
		   !(verts[0] == v->p[0] && verts[1] == v->p[1] && verts[2] == v->p[2]))
			goto cont;
		if(mask & flag & 0x10 &&
		   !(norms[0] == v->n[0] && norms[1] == v->n[1] && norms[2] == v->n[2]))
			goto cont;
		if(mask & flag & 0x100 &&
		   !(cols0[0] == v->c[0] && cols0[1] == v->c[1] &&
		     cols0[2] == v->c[2] && cols0[3] == v->c[3]))
			goto cont;
		if(mask & flag & 0x200 &&
		   !(cols1[0] == v->c1[0] && cols1[1] == v->c1[1] &&
		     cols1[2] == v->c1[2] && cols1[3] == v->c1[3]))
			goto cont;
		if(mask & flag & 0x1000 &&
		   !(tex0[0] == v->t[0] && tex0[1] == v->t[1]))
			goto cont;
		if(mask & flag & 0x2000 &&
		   !(tex1[0] == v->t1[0] && tex1[1] == v->t1[1]))
			goto cont;
		if(mask & flag & 0x10000 &&
		   !(wghts[0] == v->w[0] && wghts[1] == v->w[1] &&
		     wghts[2] == v->w[2] && wghts[3] == v->w[3] &&
		     inds[0] == v->i[0] && inds[1] == v->i[1] &&
		     inds[2] == v->i[2] && inds[3] == v->i[3]))
			goto cont;
		return i;
	cont:
		verts += 3;
		tex0 += 2;
		tex1 += 2;
		norms += 3;
		cols0 += 4;
		cols1 += 4;
		wghts += 4;
		inds += 4;
	}
	return -1;
}

void
insertSAVertex(Geometry *geo, int32 i, uint32 mask, SaVert *v)
{
	insertVertex(geo, i, mask, v);
	if(mask & 0x200 && extraVertColorOffset){
		uint8 *cols1 =
		 &PLUGINOFFSET(ExtraVertColors, geo, extraVertColorOffset)->nightColors[i*4];
		cols1[0] = v->c1[0];
		cols1[1] = v->c1[1];
		cols1[2] = v->c1[2];
		cols1[3] = v->c1[3];
	}
	if(mask & 0x10000 && skinGlobals.offset){
		Skin *skin = *PLUGINOFFSET(Skin*, geo, skinGlobals.offset);
		memcpy(&skin->weights[i*4], v->w, 16);
		memcpy(&skin->indices[i*4], v->i, 4);
	}
}

static void
saUninstanceCB(ps2::MatPipeline *pipe, Geometry *geo, uint32 flags[], Mesh *mesh, uint8 *data[])
{
	uint32 id = pipe->pluginData;
	int16 *verts       = (int16*)data[0];
	int16 *texcoords   = (int16*)data[1];
	uint8 *colors      = (uint8*)data[2];
	int8 *norms        = (int8*)data[id == 0x53f20089 ? 2 : 3];
	uint32 *wghts      = (uint32*)data[3];
	float vertScale = 1.0f/128.0f;
	if(id == 0x53f20085 || id == 0x53f20087 ||
	   id == 0x53f20089 || id == 0x53f2008b)
		vertScale = 1.0f/1024.0f;
	uint32 mask = 0x1;	// vertices
	int cinc = 4;
	int tinc = 2;
	if((geo->geoflags & Geometry::NORMALS) && hasNormals(id))
		mask |= 0x10;
	if((geo->geoflags & Geometry::PRELIT) && hasColors(id))
		mask |= 0x100;
	if(hasColors2(id)){
		mask |= 0x200;
		cinc *= 2;
	}
	if(geo->numTexCoordSets > 0)
		mask |= 0x1000;
	if(geo->numTexCoordSets > 0 && hasTex2(id)){
		mask |= 0x2000;
		tinc *= 2;
	}
	if(id == 0x53f20089)
		mask |= 0x10000;
	SaVert v;
	int32 idxstart = 0;
	for(Mesh *m = geo->meshHeader->mesh; m < mesh; m++)
		idxstart += m->numIndices;
	int8 *adc = getADCbitsForMesh(geo, mesh);
	for(uint32 i = 0; i < mesh->numIndices; i++){
		v.p[0] = verts[0]*vertScale;
		v.p[1] = verts[1]*vertScale;
		v.p[2] = verts[2]*vertScale;
		if(mask & 0x10){
			v.n[0] = norms[0]/127.0f;
			v.n[1] = norms[1]/127.0f;
			v.n[2] = norms[2]/127.0f;
		}
		if(mask & 0x200){
			v.c[0] = colors[0];
			v.c[1] = colors[2];
			v.c[2] = colors[4];
			v.c[3] = colors[6];
			v.c1[0] = colors[1];
			v.c1[1] = colors[3];
			v.c1[2] = colors[5];
			v.c1[3] = colors[7];
		}else if(mask & 0x100){
			v.c[0] = colors[0];
			v.c[1] = colors[1];
			v.c[2] = colors[2];
			v.c[3] = colors[3];
		}
		if(mask & 0x1000){
			v.t[0] = texcoords[0]/4096.0f;
			v.t[1] = texcoords[1]/4096.0f;
		}
		if(mask & 0x2000){
			v.t1[0] = texcoords[2]/4096.0f;
			v.t1[1] = texcoords[3]/4096.0f;
		}
		if(mask & 0x10000){
			for(int j = 0; j < 4; j++){
				((uint32*)v.w)[j] = wghts[j] & ~0x3FF;
				v.i[j] = (wghts[j] & 0x3FF) >> 2;
				if(v.i[j]) v.i[j]--;
				if(v.w[j] == 0.0f) v.i[j] = 0;
			}
		}
		int32 idx = findSAVertex(geo, flags, mask, &v);
		if(idx < 0)
			idx = geo->numVertices++;
		mesh->indices[i] = idx;
		adc[i] = !!verts[3];
		flags[idx] = mask;
		insertSAVertex(geo, idx, mask, &v);

		verts += 4;
		texcoords += tinc;
		colors += cinc;
		norms += 4;
		wghts += 4;
	}
}

static void
instanceSAPositions(Geometry *g, Mesh *m, int8 *adc, int16 *dst, float32 scale)
{
	float32 *verts = g->morphTargets[0].vertices;
	uint16 j;
	for(uint32 i = 0; i < m->numIndices; i++){
		j = m->indices[i];
		dst[0] = verts[j*3+0]*scale;
		dst[1] = verts[j*3+1]*scale;
		dst[2] = verts[j*3+2]*scale;
		dst[3] = adc ? 0x8000*adc[i] : 0;
		dst += 4;
	}
}

static void
instanceSATex(Geometry *g, Mesh *m, int16 *dst)
{
	float32 *tex = g->texCoords[0];
	uint16 j;
	for(uint32 i = 0; i < m->numIndices; i++){
		j = m->indices[i];
		if(tex){
			dst[0] = tex[j*2+0]*4096.0f;
			dst[1] = tex[j*2+1]*4096.0f;
		}else{
			dst[0] = 0;
			dst[1] = 0;
		}
		dst += 2;
	}
}

static void
instanceSADualTex(Geometry *g, Mesh *m, int16 *dst)
{
	float32 *tex0 = g->texCoords[0];
	float32 *tex1 = g->texCoords[1];
	uint16 j;
	for(uint32 i = 0; i < m->numIndices; i++){
		j = m->indices[i];
		if(tex0){
			dst[0] = tex0[j*2+0]*4096.0f;
			dst[1] = tex0[j*2+1]*4096.0f;
		}else{
			dst[0] = 0;
			dst[1] = 0;
		}
		if(tex1){
			dst[2] = tex1[j*2+0]*4096.0f;
			dst[3] = tex1[j*2+1]*4096.0f;
		}else{
			dst[2] = 0;
			dst[3] = 0;
		}
		dst += 4;
	}
}

static void
instanceSAColors(Geometry *g, Mesh *m, uint8 *dst)
{
	uint8 *c = g->colors;
	uint16 j;
	for(uint32 i = 0; i < m->numIndices; i++){
		j = m->indices[i];
		if(c)
			memcpy(dst, &c[j*4], 4);
		else
			memset(dst, 0xFF, 4);
		dst += 4;
	}
}

static void
instanceSADualColors(Geometry *g, Mesh *m, uint8 *dst)
{
	uint8 *c0 = g->colors;
	uint8 *c1 =
	 PLUGINOFFSET(ExtraVertColors, g, extraVertColorOffset)->nightColors;
	uint16 j;
	for(uint32 i = 0; i < m->numIndices; i++){
		j = m->indices[i];
		if(c0){
			dst[0] = c0[j*4+0];
			dst[2] = c0[j*4+1];
			dst[4] = c0[j*4+2];
			dst[6] = c0[j*4+3];
		}else{
			dst[0] = 0xFF;
			dst[2] = 0xFF;
			dst[4] = 0xFF;
			dst[6] = 0xFF;
		}
		if(c1){
			dst[1] = c1[j*4+0];
			dst[3] = c1[j*4+1];
			dst[6] = c1[j*4+2];
			dst[7] = c1[j*4+3];
		}else{
			dst[1] = 0xFF;
			dst[3] = 0xFF;
			dst[6] = 0xFF;
			dst[7] = 0xFF;
		}
		dst += 8;
	}
}

static void
instanceSANormals(Geometry *g, Mesh *m, int8 *dst)
{
	float32 *norms = g->morphTargets[0].normals;
	uint16 j;
	for(uint32 i = 0; i < m->numIndices; i++){
		j = m->indices[i];
		if(norms){
			dst[0] = norms[j*3+0]*127.0f;
			dst[1] = norms[j*3+1]*127.0f;
			dst[2] = norms[j*3+2]*127.0f;
			dst[3] = 0;
		}else
			memset(dst, 0, 4);
		dst += 4;
	}
}

static void
saInstanceCB(MatPipeline *pipe, Geometry *g, Mesh *m, uint8 **data)
{
	uint32 id = pipe->pluginData;
	float vertScale = 128.0f;
	if(id == 0x53f20085 || id == 0x53f20087 ||
	   id == 0x53f20089 || id == 0x53f2008b)
		vertScale = 1024.0f;
	ADCData *adc = PLUGINOFFSET(ADCData, g, adcOffset);

	for(uint32 i = 0; i < nelem(pipe->attribs); i++){
		PipeAttribute *a = pipe->attribs[i];
		if(a == &saXYZADC)
			instanceSAPositions(g, m, adc->adcFormatted ? adc->adcBits : NULL,
			                    (int16*)data[i], vertScale);
		if(a == &saUV)
			instanceSATex(g, m, (int16*)data[i]);
		if(a == &saUV2)
			instanceSADualTex(g, m, (int16*)data[i]);
		if(a == &saRGBA)
			instanceSAColors(g, m, (uint8*)data[i]);
		if(a == &saRGBA2)
			instanceSADualColors(g, m, (uint8*)data[i]);
		if(a == &saNormal)
			instanceSANormals(g, m, (int8*)data[i]);
		if(a == &saWeights){
			Skin *skin = *PLUGINOFFSET(Skin*, g, skinGlobals.offset);
			instanceSkinData(g, m, skin, (uint32*)data[i]);
		}
	}
}

void
registerPS2BuildingPipes(void)
{
	Pipeline *pipe;
	MatPipeline *mpipe;
	uint32 vertCount;

	pipe = new ps2::ObjPipeline(PLATFORM_PS2);
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20080;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2);
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20081;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saRGBA;
	vertCount = MatPipeline::getVertCount(VU_Lights, 3, 3, 2);
	mpipe->setTriBufferSizes(3, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	pipe = new ps2::ObjPipeline(PLATFORM_PS2);
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20082;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2);
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20083;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saRGBA2;
	vertCount = MatPipeline::getVertCount(VU_Lights, 3, 3, 2);
	mpipe->setTriBufferSizes(3, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	pipe = new ps2::ObjPipeline(PLATFORM_PS2); //unused in DFFs
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f2008C;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2);	// use with 0x53f20080
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f2008D;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saRGBA;
	mpipe->attribs[3] = &saNormal;
	vertCount = MatPipeline::getVertCount(0x3BD, 4, 3, 3);
	mpipe->setTriBufferSizes(4, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	pipe = new ps2::ObjPipeline(PLATFORM_PS2); //unused in DFFs
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f2008E;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2);	// use with 0x53f20082
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f2008F;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saRGBA2;
	mpipe->attribs[3] = &saNormal;
	vertCount = MatPipeline::getVertCount(0x3BD, 4, 3, 3);
	mpipe->setTriBufferSizes(4, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	pipe = new ps2::ObjPipeline(PLATFORM_PS2); //unused in DFFs
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20090;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2); // unused in DFFs
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20091;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saRGBA;
	vertCount = 0x50; //MatPipeline::getVertCount(VU_Lights, 3, 3, 2);
	mpipe->setTriBufferSizes(3, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	pipe = new ps2::ObjPipeline(PLATFORM_PS2); // unused in DFFs
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20092;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2); // unused in DFFs
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20093;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saRGBA2;
	vertCount = 0x50; //MatPipeline::getVertCount(VU_Lights, 3, 3, 2);
	mpipe->setTriBufferSizes(3, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);
}

void
registerPS2VehiclePipes(void)
{
	Pipeline *pipe;
	MatPipeline *mpipe;
	uint32 vertCount;

	pipe = new ps2::ObjPipeline(PLATFORM_PS2);
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20084;
	ps2::registerPDSPipe(pipe);

	// No effects whatsoever
	mpipe = new MatPipeline(PLATFORM_PS2);
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20085;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[3] = &saNormal;
	vertCount = MatPipeline::getVertCount(VU_Lights, 4, 3, 2);
	mpipe->setTriBufferSizes(4, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	pipe = new ps2::ObjPipeline(PLATFORM_PS2); // unused in DFFs
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20086;
	ps2::registerPDSPipe(pipe);

	// Environment map (+ Specular map)
	mpipe = new MatPipeline(PLATFORM_PS2);	// use with 0x53f20084
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20087;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[3] = &saNormal;
	vertCount = MatPipeline::getVertCount(0x3BD, 4, 3, 3);
	mpipe->setTriBufferSizes(4, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);

	// x-Environment map (+ Specular map)
	mpipe = new MatPipeline(PLATFORM_PS2);	// use with 0x53f20084
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f2008B;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV2;
	mpipe->attribs[3] = &saNormal;
	vertCount = MatPipeline::getVertCount(0x3BD, 4, 3, 3);
	mpipe->setTriBufferSizes(4, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	ps2::registerPDSPipe(mpipe);
}

void
registerPS2SkinPipe(void)
{
	Pipeline *pipe;
	MatPipeline *mpipe;
	uint32 vertCount;

	pipe = new ps2::ObjPipeline(PLATFORM_PS2);
	pipe->pluginID = ID_PDS;
	pipe->pluginData = 0x53f20088;
	ps2::registerPDSPipe(pipe);

	mpipe = new MatPipeline(PLATFORM_PS2);	// use with 0x53f20088
	mpipe->pluginID = ID_PDS;
	mpipe->pluginData = 0x53f20089;
	mpipe->attribs[0] = &saXYZADC;
	mpipe->attribs[1] = &saUV;
	mpipe->attribs[2] = &saNormal;
	mpipe->attribs[3] = &saWeights;
//	these values give vertCount = 0x33 :/
//	vertCount = MatPipeline::getVertCount(0x2D0, 4, 3, 2);
	vertCount = 0x30;
	mpipe->setTriBufferSizes(4, vertCount);
	mpipe->vifOffset = mpipe->inputStride*vertCount;
	mpipe->instanceCB = saInstanceCB;
	mpipe->preUninstCB = saPreCB;
	mpipe->uninstanceCB = saUninstanceCB;
	mpipe->postUninstCB = saPostCB;
	ps2::registerPDSPipe(mpipe);
}

void
registerPDSPipes(void)
{
	// 19 pipelines in total
	registerPS2BuildingPipes();
	registerPS2VehiclePipes();
	registerPS2SkinPipe();
}

}
