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
CColSphere::Set(float radius, rw::V3d *center, rw::uint8 mat, rw::uint8 flag)
{
	this->radius = radius;
	this->center = *center;
	this->mat = mat;
	this->flag = flag;
}

void
CColBox::Set(rw::V3d *min, rw::V3d *max, rw::uint8 mat, rw::uint8 flag)
{
	this->min = *min;
	this->max = *max;
	this->mat = mat;
	this->flag = flag;
}

void
CColLine::Set(rw::V3d *p0, rw::V3d *p1)
{
	this->p0 = *p0;
	this->p1 = *p1;
}

void
CColTriangle::Set(int a, int b, int c, rw::uint8 mat)
{
	this->a = a;
	this->b = b;
	this->c = c;
	this->mat = mat;
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