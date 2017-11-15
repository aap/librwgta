#include "euryopa.h"



struct DebugLine
{
	rw::V3d v1;
	rw::V3d v2;
	rw::RGBA col1;
	rw::RGBA col2;
};
#define MAXDEBUGLINES 40000
static DebugLine debugLines[MAXDEBUGLINES];
static int numDebugLines;

#define TEMPBUFFERVERTSIZE 256
#define TEMPBUFFERINDEXSIZE 1024
int TempBufferIndicesStored;
int TempBufferVerticesStored;
rw::RWDEVICE::Im3DVertex TempVertexBuffer[TEMPBUFFERVERTSIZE];
uint16 TempIndexBuffer[TEMPBUFFERINDEXSIZE];

static void
RenderAndEmptyRenderBuffer(void)
{
	assert(TempBufferVerticesStored <= TEMPBUFFERVERTSIZE);
	assert(TempBufferIndicesStored <= TEMPBUFFERINDEXSIZE);
	if(TempBufferVerticesStored){
		rw::im3d::Transform(TempVertexBuffer, TempBufferVerticesStored, nil);
		rw::im3d::RenderIndexed(rw::PRIMTYPELINELIST, TempIndexBuffer, TempBufferIndicesStored);
		rw::im3d::End();
	}
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}

static void
RenderLine(float x1, float y1, float z1, float x2, float y2, float z2, rw::RGBA c1, rw::RGBA c2)
{
	int i;
	if(TempBufferVerticesStored+2 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+2 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	i = TempBufferVerticesStored;
	TempVertexBuffer[i].setX(x1);
	TempVertexBuffer[i].setY(y1);
	TempVertexBuffer[i].setZ(z1);
	TempVertexBuffer[i].setColor(c1.red, c1.green, c1.blue, c1.alpha);
	TempVertexBuffer[i+1].setX(x2);
	TempVertexBuffer[i+1].setY(y2);
	TempVertexBuffer[i+1].setZ(z2);
	TempVertexBuffer[i+1].setColor(c2.red, c2.green, c2.blue, c2.alpha);
	TempBufferVerticesStored += 2;

	TempIndexBuffer[TempBufferIndicesStored++] = i;
	TempIndexBuffer[TempBufferIndicesStored++] = i+1;
}

void
RenderDebugLines(void)
{
	int i;

	rw::engine->imtexture = nil;

	for(i = 0; i < numDebugLines; i++)
		RenderLine(debugLines[i].v1.x, debugLines[i].v1.y, debugLines[i].v1.z,
			debugLines[i].v2.x, debugLines[i].v2.y, debugLines[i].v2.z,
			debugLines[i].col1, debugLines[i].col2);
	RenderAndEmptyRenderBuffer();
	numDebugLines = 0;
}

static void
AddDebugLine(float x1, float y1, float z1, float x2, float y2, float z2, rw::RGBA c1, rw::RGBA c2)
{
	int n;
	if(numDebugLines >= MAXDEBUGLINES)
		return;
	n = numDebugLines++;
	debugLines[n].v1.x = x1;
	debugLines[n].v1.y = y1;
	debugLines[n].v1.z = z1;
	debugLines[n].v2.x = x2;
	debugLines[n].v2.y = y2;
	debugLines[n].v2.z = z2;
	debugLines[n].col1 = c1;
	debugLines[n].col2 = c2;
}

static void AddDebugLine(rw::V3d v1, rw::V3d v2, rw::RGBA c1, rw::RGBA c2) { AddDebugLine(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, c1, c2); }

static void
RenderBox(CBox *box, rw::RGBA col, rw::Matrix *xform)
{
	rw::V3d verts[8];
	verts[0].x = box->min.x;
	verts[0].y = box->min.y;
	verts[0].z = box->min.z;
	verts[1].x = box->max.x;
	verts[1].y = box->min.y;
	verts[1].z = box->min.z;
	verts[2].x = box->min.x;
	verts[2].y = box->max.y;
	verts[2].z = box->min.z;
	verts[3].x = box->max.x;
	verts[3].y = box->max.y;
	verts[3].z = box->min.z;
	verts[4].x = box->min.x;
	verts[4].y = box->min.y;
	verts[4].z = box->max.z;
	verts[5].x = box->max.x;
	verts[5].y = box->min.y;
	verts[5].z = box->max.z;
	verts[6].x = box->min.x;
	verts[6].y = box->max.y;
	verts[6].z = box->max.z;
	verts[7].x = box->max.x;
	verts[7].y = box->max.y;
	verts[7].z = box->max.z;
	rw::V3d::transformPoints(verts, verts, 8, xform);

	AddDebugLine(verts[0], verts[1], col, col);
	AddDebugLine(verts[1], verts[3], col, col);
	AddDebugLine(verts[3], verts[2], col, col);
	AddDebugLine(verts[2], verts[0], col, col);

	AddDebugLine(verts[0+4], verts[1+4], col, col);
	AddDebugLine(verts[1+4], verts[3+4], col, col);
	AddDebugLine(verts[3+4], verts[2+4], col, col);
	AddDebugLine(verts[2+4], verts[0+4], col, col);

	AddDebugLine(verts[0], verts[4], col, col);
	AddDebugLine(verts[1], verts[5], col, col);
	AddDebugLine(verts[2], verts[6], col, col);
	AddDebugLine(verts[3], verts[7], col, col);
}

static void
RenderSphere(CSphere *sphere, rw::RGBA col, rw::Matrix *xform)
{
	rw::V3d c;
	rw::V3d verts[6];
	rw::V3d::transformPoints(&c, &sphere->center, 1, xform);
	verts[0] = verts[1] = verts[2] = verts[3] = verts[4] = verts[5] = c;
	verts[0].z += sphere->radius;	// top
	verts[1].z -= sphere->radius;	// bottom
	verts[2].x += sphere->radius;
	verts[3].x -= sphere->radius;
	verts[4].y += sphere->radius;
	verts[5].y -= sphere->radius;

	AddDebugLine(verts[0], verts[2], col, col);
	AddDebugLine(verts[0], verts[3], col, col);
	AddDebugLine(verts[0], verts[4], col, col);
	AddDebugLine(verts[0], verts[5], col, col);
	AddDebugLine(verts[1], verts[2], col, col);
	AddDebugLine(verts[1], verts[3], col, col);
	AddDebugLine(verts[1], verts[4], col, col);
}

static void
RenderTriangle(rw::V3d *v1, rw::V3d *v2, rw::V3d *v3, rw::RGBA col, rw::Matrix *xform)
{
	rw::V3d verts[3];
	verts[0] = *v1;
	verts[1] = *v2;
	verts[2] = *v3;
	rw::V3d::transformPoints(verts, verts, 3, xform);
	AddDebugLine(verts[0], verts[1], col, col);
	AddDebugLine(verts[1], verts[2], col, col);
	AddDebugLine(verts[2], verts[0], col, col);
}

void
RenderColModelWire(CColModel *col, rw::Matrix *xform, bool onlyBounds)
{
	static const rw::RGBA red = { 255, 0, 0, 255 };
	static const rw::RGBA green = { 0, 255, 0, 255 };
	static const rw::RGBA blue = { 0, 0, 255, 255 };
	static const rw::RGBA magenta = { 255, 0, 255, 255 };
	static const rw::RGBA white = { 255, 255, 255, 255 };
	int i;
	CColTriangle *tri;

	RenderBox(&col->boundingBox, red, xform);
	if(onlyBounds)
		return;
	for(i = 0; i < col->numBoxes; i++)
		RenderBox(&col->boxes[i].box, white, xform);
	for(i = 0; i < col->numSpheres; i++)
		RenderSphere(&col->spheres[i].sph, magenta, xform);
	for(i = 0; i < col->numTriangles; i++){
		tri = &col->triangles[i];
		if(col->flags & 0x80){
			rw::V3d v[3];
			v[0] = col->compVertices[tri->a].Uncompress();
			v[1] = col->compVertices[tri->b].Uncompress();
			v[2] = col->compVertices[tri->c].Uncompress();
			RenderTriangle(&v[0], &v[1], &v[2], green, xform);
		}else
			RenderTriangle(&col->vertices[tri->a], &col->vertices[tri->b], &col->vertices[tri->c],
				green, xform);
	}
}

