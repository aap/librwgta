#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <args.h>
#include <rw.h>

#include "rwgta.h"
#include "collision.h"

using namespace rw;

void
CColSphere::Set(float radius, rw::V3d *center, rw::uint8 surf, rw::uint8 piece)
{
	this->radius = radius;
	this->center = *center;
	this->surface = surf;
	this->piece = piece;
}

void
CColBox::Set(rw::V3d *min, rw::V3d *max, rw::uint8 surf, rw::uint8 piece)
{
	this->min = *min;
	this->max = *max;
	this->surface = surf;
	this->piece = piece;
}

void
CColLine::Set(rw::V3d *p0, rw::V3d *p1)
{
	this->p0 = *p0;
	this->p1 = *p1;
}

void
CColTriangle::Set(int a, int b, int c, rw::uint8 surf)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->surface = surf;
}

CColModel::CColModel(void)
{
	this->numSpheres = 0;
	this->spheres = NULL;
	this->numLines = 0;
	this->lines = NULL;
	this->numBoxes = 0;
	this->boxes = NULL;
	this->numTriangles = 0;
	this->vertices = NULL;
	this->triangles = NULL;
}

CColModel::~CColModel(void)
{
	delete[] this->spheres;
	delete[] this->lines;
	delete[] this->boxes;
	delete[] this->vertices;
	delete[] this->triangles;
}

void
readColModel(CColModel *colmodel, uint8 *buf)
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
	buf = (uint8*)fp;
	colmodel->numSpheres = *(int32*)buf;
	buf += 4;
	if(colmodel->numSpheres){
		colmodel->spheres = new CColSphere[colmodel->numSpheres];
		for(int i = 0; i < colmodel->numSpheres; i++){
			colmodel->spheres[i].Set(*(float*)buf, (V3d*)(buf+4), buf[16], buf[17]); // buf[19] light
			buf += 20;
		}
	}

	colmodel->numLines = *(int32*)buf;
	buf += 4;
	if(colmodel->numLines){
		colmodel->lines = new CColLine[colmodel->numLines];
		for(int i = 0; i < colmodel->numLines; i++){
			colmodel->lines[i].Set((V3d*)buf, (V3d*)(buf+12));
			buf += 24;
		}
	}

	colmodel->numBoxes = *(int32*)buf;
	buf += 4;
	if(colmodel->numBoxes){
		colmodel->boxes = new CColBox[colmodel->numBoxes];
		for(int i = 0; i < colmodel->numBoxes; i++){
			colmodel->boxes[i].Set((V3d*)buf, (V3d*)(buf+12), buf[24], buf[25]); // buf[27] light
			buf += 28;
		}
	}

	int32 numVertices = *(int32*)buf;
	buf += 4;
	if(numVertices){
		colmodel->vertices = new V3d[numVertices];
		for(int i = 0; i < numVertices; i++){
			colmodel->vertices[i] = *(V3d*)buf;
			buf += 12;
		}
	}

	colmodel->numTriangles = *(int32*)buf;
	buf += 4;
	if(colmodel->numTriangles){
		colmodel->triangles = new CColTriangle[colmodel->numTriangles];
		for(int i = 0; i < colmodel->numTriangles; i++){
			colmodel->triangles[i].Set(*(int32*)buf, *(int32*)(buf+4), *(int32*)(buf+8), buf[12]); // buf[15] light
			buf += 16;
		}
	}
}

uint32
writeColModel(CColModel *colmodel, rw::uint8 **bufp)
{
	uint8 *buf;
	uint32 size;
	int32 numVertices = -1;
	for(int i = 0; i < colmodel->numTriangles; i++){
		if(colmodel->triangles[i].a > numVertices)
			numVertices = colmodel->triangles[i].a;
		if(colmodel->triangles[i].b > numVertices)
			numVertices = colmodel->triangles[i].b;
		if(colmodel->triangles[i].c > numVertices)
			numVertices = colmodel->triangles[i].c;
	}
	numVertices++;
	size = 10*sizeof(float32) + 5*sizeof(int32) +
		colmodel->numSpheres*20 + colmodel->numLines*24 +
		colmodel->numBoxes*28 + numVertices*12 + colmodel->numTriangles*16;
	*bufp = buf = new uint8[size];
	memset(buf, 0, size);
	uint8 *end = buf + size;

	float *fp = (float*)buf;
	*fp++ = colmodel->boundingSphere.radius;
	*fp++ = colmodel->boundingSphere.center.x;
	*fp++ = colmodel->boundingSphere.center.y;
	*fp++ = colmodel->boundingSphere.center.z;
	*fp++ = colmodel->boundingBox.min.x;
	*fp++ = colmodel->boundingBox.min.y;
	*fp++ = colmodel->boundingBox.min.z;
	*fp++ = colmodel->boundingBox.max.x;
	*fp++ = colmodel->boundingBox.max.y;
	*fp++ = colmodel->boundingBox.max.z;
	buf = (uint8*)fp;

	*(int32*)buf = colmodel->numSpheres;
	buf += 4;
	for(int i = 0; i < colmodel->numSpheres; i++){
		*(float*)(buf+0) = colmodel->spheres[i].radius;
		*(V3d*)(buf+4) = colmodel->spheres[i].center;
		buf[16] = colmodel->spheres[i].surface;
		buf[17] = colmodel->spheres[i].piece;
		buf[19] = 0;	// light
		buf += 20;
	}

	*(int32*)buf = colmodel->numLines;
	buf += 4;
	for(int i = 0; i < colmodel->numLines; i++){
		*(V3d*)(buf+0) = colmodel->lines[i].p0;
		*(V3d*)(buf+12) = colmodel->lines[i].p1;
		buf += 24;
	}

	*(int32*)buf = colmodel->numBoxes;
	buf += 4;
	for(int i = 0; i < colmodel->numBoxes; i++){
		*(V3d*)(buf+0) = colmodel->boxes[i].min;
		*(V3d*)(buf+12) = colmodel->boxes[i].max;
		buf[24] = colmodel->boxes[i].surface;
		buf[25] = colmodel->boxes[i].piece;
		buf[27] = 0; // light
		buf += 28;
	}

	*(int32*)buf = numVertices;
	buf += 4;
	for(int i = 0; i < numVertices; i++){
		*(V3d*)buf = colmodel->vertices[i];
		buf += 12;
	}

	*(int32*)buf = colmodel->numTriangles;
	buf += 4;
	for(int i = 0; i < colmodel->numTriangles; i++){
		*(int32*)(buf+0) = colmodel->triangles[i].a;
		*(int32*)(buf+4) = colmodel->triangles[i].b;
		*(int32*)(buf+8) = colmodel->triangles[i].c;
		buf[12] = colmodel->triangles[i].surface;
		buf[15] = 0; // light
		buf += 16;
	}
	assert(buf == end);
	return size;
}
