#include "euryopa.h"

CColModel::CColModel(void)
{
	this->numSpheres = 0;
	this->spheres = nil;
	this->numLines = 0;
	this->lines = nil;
	this->numBoxes = 0;
	this->boxes = nil;
	this->numTriangles = 0;
	this->vertices = nil;
	this->triangles = nil;

	this->flags = 0;
	this->allocFlag = 0;	// SA does something strange here
	this->rawdata = nil;
}

CColModel::~CColModel(void)
{
	if(this->allocFlag & 2)
		rwFree(this->rawdata);
	else{
		rwFree(this->spheres);
		rwFree(this->lines);
		rwFree(this->boxes);
		rwFree(this->vertices);
		rwFree(this->triangles);
	}
}

void
ReadColModel(CColModel *colmodel, rw::uint8 *buf, int size)
{
	float *fp = (float*)buf;
	colmodel->boundingSphere.radius = *fp++;
	colmodel->boundingSphere.center.x = *fp++;
	colmodel->boundingSphere.center.y = *fp++;
	colmodel->boundingSphere.center.z = *fp++;
	colmodel->boundingBox.min.x = *fp++;
	colmodel->boundingBox.min.y = *fp++;
	colmodel->boundingBox.min.z = *fp++;
	colmodel->boundingBox.max.x = *fp++;
	colmodel->boundingBox.max.y = *fp++;
	colmodel->boundingBox.max.z = *fp++;
	buf = (rw::uint8*)fp;
	colmodel->numSpheres = *(int16*)buf;
	buf += 4;
	if(colmodel->numSpheres){
		colmodel->spheres = rwNewT(CColSphere, colmodel->numSpheres, 0);
		for(int i = 0; i < colmodel->numSpheres; i++){
			colmodel->spheres[i].Set(*(float*)buf, (rw::V3d*)(buf+4), buf[16], buf[17], buf[19]);
			buf += 20;
		}
	}

	colmodel->numLines = *(int16*)buf;
	buf += 4;
	if(colmodel->numLines){
		colmodel->lines = rwNewT(CColLine, colmodel->numLines, 0);
		for(int i = 0; i < colmodel->numLines; i++){
			colmodel->lines[i].Set((rw::V3d*)buf, (rw::V3d*)(buf+12));
			buf += 24;
		}
	}

	colmodel->numBoxes = *(int16*)buf;
	buf += 4;
	if(colmodel->numBoxes){
		colmodel->boxes = rwNewT(CColBox, colmodel->numBoxes, 0);
		for(int i = 0; i < colmodel->numBoxes; i++){
			colmodel->boxes[i].Set((rw::V3d*)buf, (rw::V3d*)(buf+12), buf[24], buf[25], buf[27]);
			buf += 28;
		}
	}

	int32 numVertices = *(int16*)buf;
	buf += 4;
	if(numVertices){
		colmodel->vertices = rwNewT(rw::V3d, numVertices, 0);
		for(int i = 0; i < numVertices; i++){
			colmodel->vertices[i] = *(rw::V3d*)buf;
			buf += 12;
		}
	}

	colmodel->numTriangles = *(int16*)buf;
	buf += 4;
	if(colmodel->numTriangles){
		colmodel->triangles = rwNewT(CColTriangle, colmodel->numTriangles, 0);
		for(int i = 0; i < colmodel->numTriangles; i++){
			colmodel->triangles[i].Set(*(int32*)buf, *(int32*)(buf+4), *(int32*)(buf+8), buf[12], buf[15]);
			buf += 16;
		}
	}

	colmodel->allocFlag = 0;
	if(colmodel->numSpheres || colmodel->numLines || colmodel->numBoxes || colmodel->numTriangles)
		colmodel->allocFlag |= 1;
}

struct Col4Header
{
	CBox boundingBox;
	CSphere boundingSphere;
	int16 numSpheres;
	int16 numBoxes;
	int16 numTriangles;
	int16 numLines;
	uint8 flags;
	uint32 sphereOffset;
	uint32 boxOffset;
	uint32 lineOffset;
	uint32 vertexOffset;
	uint32 triangleOffset;
	uint32 unused;	// triangle planes

	// Ver3
	int32 numShadowTriangles;
	uint32 shadowVertexOffset;
	uint32 shadowTriangleOffset;

	// Ver4
	uint32 unused2;
};

void
ReadColModelVer2(CColModel *colmodel, uint8 *buf, int32 size)
{
#define COLHEADERSIZE 0x4C
	Col4Header *header = (Col4Header*)buf;
	int datasize = size - COLHEADERSIZE;
	colmodel->boundingBox = header->boundingBox;
	colmodel->boundingSphere = header->boundingSphere;
	// flag 2
	colmodel->allocFlag = (colmodel->allocFlag&~1) | (header->flags>>1)&1;
	if(datasize <= 0)
		return;
	colmodel->rawdata = rwNewT(uint8, datasize, 0);
	memcpy(colmodel->rawdata, buf+COLHEADERSIZE, datasize);
	colmodel->numSpheres = header->numSpheres;
	colmodel->numBoxes = header->numBoxes;
	colmodel->numLines = header->numLines;
	colmodel->numTriangles = header->numTriangles;
	colmodel->flags &= ~1;
	colmodel->flags &= ~4;
	// flag 8
	colmodel->flags = (colmodel->flags&~2) | (header->flags>>2)&2;

	colmodel->spheres = header->sphereOffset ?
		(CColSphere*)(colmodel->rawdata + header->sphereOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->boxes = header->boxOffset ?
		(CColBox*)(colmodel->rawdata + header->boxOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->lines = header->lineOffset ?
		(CColLine*)(colmodel->rawdata + header->lineOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->compVertices = header->vertexOffset ?
		(CompressedVector*)(colmodel->rawdata + header->vertexOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->triangles = header->triangleOffset ?
		(CColTriangle*)(colmodel->rawdata + header->triangleOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->allocFlag |= 2;

	colmodel->flags = 0x80;	// compressed vertices
#undef COLHEADERSIZE
}

void
ReadColModelVer3(CColModel *colmodel, uint8 *buf, int32 size)
{
#define COLHEADERSIZE 0x58
	Col4Header *header = (Col4Header*)buf;
	int datasize = size - COLHEADERSIZE;
	colmodel->boundingBox = header->boundingBox;
	colmodel->boundingSphere = header->boundingSphere;
	// flag 2
	colmodel->allocFlag = (colmodel->allocFlag&~1) | (header->flags>>1)&1;
	if(datasize <= 0)
		return;
	colmodel->rawdata = rwNewT(uint8, datasize, 0);
	memcpy(colmodel->rawdata, buf+COLHEADERSIZE, datasize);
	colmodel->numSpheres = header->numSpheres;
	colmodel->numBoxes = header->numBoxes;
	colmodel->numLines = header->numLines;
	colmodel->numTriangles = header->numTriangles;
	colmodel->flags &= ~1;
	colmodel->flags &= ~4;
	// flag 8
	colmodel->flags = (colmodel->flags&~2) | (header->flags>>2)&2;

	colmodel->spheres = header->sphereOffset ?
		(CColSphere*)(colmodel->rawdata + header->sphereOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->boxes = header->boxOffset ?
		(CColBox*)(colmodel->rawdata + header->boxOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->lines = header->lineOffset ?
		(CColLine*)(colmodel->rawdata + header->lineOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->compVertices = header->vertexOffset ?
		(CompressedVector*)(colmodel->rawdata + header->vertexOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->triangles = header->triangleOffset ?
		(CColTriangle*)(colmodel->rawdata + header->triangleOffset - COLHEADERSIZE - 0x1C) :
		nil;
	colmodel->allocFlag |= 2;

	// TODO: read shadow mesh

	colmodel->flags = 0x80;	// compressed vertices
#undef COLHEADERSIZE
}
