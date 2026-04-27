#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>
#include <skeleton.h>
#include <args.h>

#include <rwgta.h>
#include "imgui/ImGuizmo.h"
#include "sol/sol.hpp"

#include "stuff.h"

using namespace std;



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

#define TEMPBUFFERVERTSIZE 10000
#define TEMPBUFFERINDEXSIZE 10000
static int TempBufferIndicesStored;
static int TempBufferVerticesStored;
static rw::RWDEVICE::Im3DVertex TempVertexBuffer[TEMPBUFFERVERTSIZE];
static uint16 TempIndexBuffer[TEMPBUFFERINDEXSIZE];

static void
RenderAndEmptyRenderBuffer(rw::Matrix *matrix = nil)
{
	assert(TempBufferVerticesStored <= TEMPBUFFERVERTSIZE);
	assert(TempBufferIndicesStored <= TEMPBUFFERINDEXSIZE);
	if(TempBufferVerticesStored){
		rw::im3d::Transform(TempVertexBuffer, TempBufferVerticesStored, matrix, rw::im3d::EVERYTHING);
		rw::im3d::RenderIndexedPrimitive(rw::PRIMTYPELINELIST, TempIndexBuffer, TempBufferIndicesStored);
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

	rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);

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

void RenderLine(rw::V3d v1, rw::V3d v2, rw::RGBA c1, rw::RGBA c2) { AddDebugLine(v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, c1, c2); }

void
RenderWireBoxVerts(rw::V3d *verts, rw::RGBA col)
{
	RenderLine(verts[0], verts[1], col, col);
	RenderLine(verts[1], verts[3], col, col);
	RenderLine(verts[3], verts[2], col, col);
	RenderLine(verts[2], verts[0], col, col);

	RenderLine(verts[0+4], verts[1+4], col, col);
	RenderLine(verts[1+4], verts[3+4], col, col);
	RenderLine(verts[3+4], verts[2+4], col, col);
	RenderLine(verts[2+4], verts[0+4], col, col);

	RenderLine(verts[0], verts[4], col, col);
	RenderLine(verts[1], verts[5], col, col);
	RenderLine(verts[2], verts[6], col, col);
	RenderLine(verts[3], verts[7], col, col);
}

void
RenderWireBox(gta::CBox *box, rw::RGBA col, rw::Matrix *xform)
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
	if(xform)
		rw::V3d::transformPoints(verts, verts, 8, xform);

	RenderWireBoxVerts(verts, col);
}

void
RenderSphereAsWireBox(gta::CSphere *sphere, rw::RGBA col, rw::Matrix *xform)
{
	gta::CBox box;
	rw::V3d sz = { 1.0f, 1.0f, 1.0f };
	sz = rw::scale(sz,sphere->radius*0.5f);
	box.min = rw::sub(sphere->center, sz);
	box.max = rw::add(sphere->center, sz);
	RenderWireBox(&box, col, xform);
}

void
RenderSphereAsCross(gta::CSphere *sphere, rw::RGBA col, rw::Matrix *xform)
{
	using namespace rw;
	float off = sphere->radius*0.5f;
	RenderLine(sub(sphere->center, makeV3d(off,0.0f,0.0f)),
	           add(sphere->center, makeV3d(off,0.0f,0.0f)),
	           col, col);
	RenderLine(sub(sphere->center, makeV3d(0.0f,off,0.0f)),
	           add(sphere->center, makeV3d(0.0f,off,0.0f)),
	           col, col);
	RenderLine(sub(sphere->center, makeV3d(0.0f,0.0f,off)),
	           add(sphere->center, makeV3d(0.0f,0.0f,off)),
	           col, col);
}

void
RenderWireSphere(gta::CSphere *sphere, rw::RGBA col, rw::Matrix *xform)
{
	rw::V3d c;
	rw::V3d verts[6];
	c = sphere->center;
	if(xform)
		rw::V3d::transformPoints(&c, &c, 1, xform);
	verts[0] = verts[1] = verts[2] = verts[3] = verts[4] = verts[5] = c;
	verts[0].z += sphere->radius;	// top
	verts[1].z -= sphere->radius;	// bottom
	verts[2].x += sphere->radius;
	verts[3].x -= sphere->radius;
	verts[4].y += sphere->radius;
	verts[5].y -= sphere->radius;

	RenderLine(verts[0], verts[2], col, col);
	RenderLine(verts[0], verts[3], col, col);
	RenderLine(verts[0], verts[4], col, col);
	RenderLine(verts[0], verts[5], col, col);
	RenderLine(verts[1], verts[2], col, col);
	RenderLine(verts[1], verts[3], col, col);
	RenderLine(verts[1], verts[4], col, col);
}

void
RenderWireTriangle(rw::V3d *v1, rw::V3d *v2, rw::V3d *v3, rw::RGBA col, rw::Matrix *xform)
{
	rw::V3d verts[3];
	verts[0] = *v1;
	verts[1] = *v2;
	verts[2] = *v3;
	if(xform)
		rw::V3d::transformPoints(verts, verts, 3, xform);
	RenderLine(verts[0], verts[1], col, col);
	RenderLine(verts[1], verts[2], col, col);
	RenderLine(verts[2], verts[0], col, col);
}

void
RenderAxesWidget(rw::V3d pos, rw::V3d x, rw::V3d y, rw::V3d z)
{
	rw::RGBA red = { 255, 0, 0, 255 };
	rw::RGBA green = { 0, 255, 0, 255 };
	rw::RGBA blue = { 0, 0, 255, 255 };
	RenderLine(pos, add(pos, x), red, red);
	RenderLine(pos, add(pos, y), green, green);
	RenderLine(pos, add(pos, z), blue, blue);
}

void
RenderGrid(int nx, int ny, float spacing)
{
	rw::RGBA col = { 0x8e, 0x8e, 0x8e, 0xFF };
	rw::RGBA black = { 0, 0, 0, 0xFF };
	int sx = - nx/2; int ex = nx - nx/2;
	int sy = - ny/2; int ey = nx - nx/2;
	for(int x = sx; x <= ex; x++) {
		rw::V3d start = { x*spacing, sy*spacing, 0.0f };
		rw::V3d end = { x*spacing, ey*spacing, 0.0f };
		if(x == 0)
			RenderLine(start, end, black, black);
		else
			RenderLine(start, end, col, col);
	}
	for(int y = sy; y <= ey; y++) {
		rw::V3d start = { sx*spacing, y*spacing, 0.0f };
		rw::V3d end = { ex*spacing, y*spacing, 0.0f };
		if(y == 0)
			RenderLine(start, end, black, black);
		else
			RenderLine(start, end, col, col);
	}
}

void
RenderColModelWire(gta::CColModel *col, rw::Matrix *xform, bool onlyBounds)
{
	static const rw::RGBA red = { 255, 0, 0, 255 };
	static const rw::RGBA green = { 0, 255, 0, 255 };
	static const rw::RGBA blue = { 0, 0, 255, 255 };
	static const rw::RGBA magenta = { 255, 0, 255, 255 };
	static const rw::RGBA white = { 255, 255, 255, 255 };
	int i;
	gta::CColTriangle *tri;

	RenderWireBox(&col->boundingBox, red, xform);
	if(onlyBounds)
		return;
	for(i = 0; i < col->numBoxes; i++)
		RenderWireBox(&col->boxes[i].box, white, xform);
	for(i = 0; i < col->numSpheres; i++)
		RenderWireSphere(&col->spheres[i].sph, magenta, xform);
	for(i = 0; i < col->numTriangles; i++){
		tri = &col->triangles[i];
		if(col->flags & 0x80){
			rw::V3d v[3];
			v[0] = col->compVertices[tri->a].Uncompress();
			v[1] = col->compVertices[tri->b].Uncompress();
			v[2] = col->compVertices[tri->c].Uncompress();
			RenderWireTriangle(&v[0], &v[1], &v[2], green, xform);
		}else
			RenderWireTriangle(&col->vertices[tri->a], &col->vertices[tri->b], &col->vertices[tri->c],
					green, xform);
	}
}

void
registerDebugRender(sol::state &lua)
{
	sol::table gtatab = lua["gta"].get_or_create<sol::table>();

	gtatab.set_function("renderWireBox", &RenderWireBox);
	gtatab.set_function("renderSphereAsWireBox", &RenderSphereAsWireBox);
	gtatab.set_function("renderSphereAsCross", &RenderSphereAsCross);
	gtatab.set_function("renderWireSphere", &RenderWireSphere);
	gtatab.set_function("renderWireTriangle", &RenderWireTriangle);
	gtatab.set_function("renderAxesWidget", &RenderAxesWidget);
	gtatab.set_function("renderGrid", &RenderGrid);
	gtatab.set_function("renderColModelWire", &RenderColModelWire);
	gtatab.set_function("renderDebugLines", &RenderDebugLines);
}

void
renderWireAtomic(rw::Atomic *atomic, const rw::RGBA &col)
{
	rw::Geometry *geo = atomic->geometry;

	int i;
	RenderAndEmptyRenderBuffer();

	rw::SetRenderStatePtr(rw::TEXTURERASTER, nil);
	rw::Camera *cam = (rw::Camera*)rw::engine->currentCamera;
	rw::V3d fwd = cam->getFrame()->getLTM()->at;

	rw::V3d *verts = geo->morphTargets[0].vertices;
	for(int i = 0; i < geo->numVertices; i++) {
		TempVertexBuffer[i].setX(verts[i].x - fwd.x*0.005);
		TempVertexBuffer[i].setY(verts[i].y - fwd.y*0.005);
		TempVertexBuffer[i].setZ(verts[i].z - fwd.z*0.005);
		TempVertexBuffer[i].setColor(col.red, col.green, col.blue, col.alpha);
	}
	TempBufferVerticesStored = geo->numVertices;

	rw::Triangle *tris = geo->triangles;
	for(int i = 0; i < geo->numTriangles; i++) {
		TempIndexBuffer[i*6+0] = tris[i].v[0];
		TempIndexBuffer[i*6+1] = tris[i].v[1];
		TempIndexBuffer[i*6+2] = tris[i].v[1];
		TempIndexBuffer[i*6+3] = tris[i].v[2];
		TempIndexBuffer[i*6+4] = tris[i].v[2];
		TempIndexBuffer[i*6+5] = tris[i].v[0];
	}
	TempBufferIndicesStored = geo->numTriangles*6;

	RenderAndEmptyRenderBuffer(atomic->getFrame()->getLTM());
}
