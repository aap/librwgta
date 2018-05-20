#include "III.h"

eLevelName CCollision::ms_collisionInMemory;

void
CCollision::Update(void)
{
	CVector pos = FindPlayerCoors();
	eLevelName level = CTheZones::m_CurrLevel;
	bool changeLevel = false;

	// hardcode a level if there are no zones
	if(level == LEVEL_NONE){
		if(CGame::currLevel == LEVEL_INDUSTRIAL &&
		   pos.x < 400.0f){
			level = LEVEL_COMMERCIAL;
			changeLevel = true;
		}else if(CGame::currLevel == LEVEL_SUBURBAN &&
		         pos.x > -450.0f && pos.y < -1400.0f){
			level = LEVEL_COMMERCIAL;
			changeLevel = true;
		}else{
			if(pos.x > 800.0f){
				level = LEVEL_INDUSTRIAL;
				changeLevel = true;
			}else if(pos.x < -800.0f){
				level = LEVEL_SUBURBAN;
				changeLevel = true;
			}
		}
	}
	if(level != LEVEL_NONE && level != CGame::currLevel){
		debug("changing level %d -> %d\n", CGame::currLevel, level);
		CGame::currLevel = level;
	}
	if(ms_collisionInMemory != CGame::currLevel)
		LoadCollisionWhenINeedIt(changeLevel);
	CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
}

void
CCollision::LoadCollisionWhenINeedIt(bool changeLevel)
{
	eLevelName level;
	level = LEVEL_NONE;
	if(!changeLevel){
		//assert(0 && "unimplemented");
	}

	if(level != CGame::currLevel || changeLevel){
		CTimer::Stop();
		CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
		CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
		CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
		CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
		ms_collisionInMemory = CGame::currLevel;
		CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
		CStreaming::RemoveUnusedBuildings(CGame::currLevel);
		CStreaming::RequestBigBuildings(CGame::currLevel);
		CStreaming::LoadAllRequestedModels();
		CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
		CTimer::Update();
	}
}


/*
 * ColModel code
 */

void
CColSphere::Set(float radius, const CVector &center, uint8 surf, uint8 piece)
{
	this->radius = radius;
	this->center = center;
	this->surface = surf;
	this->piece = piece;
}

void
CColBox::Set(const CVector &min, const CVector &max, uint8 surf, uint8 piece)
{
	this->min = min;
	this->max = max;
	this->surface = surf;
	this->piece = piece;
}

void
CColLine::Set(const CVector &p0, const CVector &p1)
{
	this->p0 = p0;
	this->p1 = p1;
}

void
CColTriangle::Set(int a, int b, int c, uint8 surf)
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
RenderOneLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32 c1, uint32 c2)
{
	if(TempBufferVerticesStored+2 >= TEMPBUFFERVERTSIZE ||
	   TempBufferIndicesStored+2 >= TEMPBUFFERINDEXSIZE)
		RenderAndEmptyRenderBuffer();

	TempVertexBuffer[TempBufferVerticesStored + 0].setX(x1);
	TempVertexBuffer[TempBufferVerticesStored + 0].setY(y1);
	TempVertexBuffer[TempBufferVerticesStored + 0].setZ(z1);
	TempVertexBuffer[TempBufferVerticesStored + 0].setColor(c1, c1>>8, c1>>16, c1>>24);
	TempVertexBuffer[TempBufferVerticesStored + 1].setX(x2);
	TempVertexBuffer[TempBufferVerticesStored + 1].setY(y2);
	TempVertexBuffer[TempBufferVerticesStored + 1].setZ(z2);
	TempVertexBuffer[TempBufferVerticesStored + 1].setColor(c2, c2>>8, c2>>16, c2>>24);

	TempIndexBuffer[TempBufferIndicesStored++] = TempBufferVerticesStored++;
	TempIndexBuffer[TempBufferIndicesStored++] = TempBufferVerticesStored++;
}

static void
RenderWireBoxVerts(CVector *verts, rw::RGBA col)
{
	uint32 c = RWRGBAINT(col.red, col.green, col.blue, col.alpha);
	RenderOneLine(verts[0].x, verts[0].y, verts[0].z, verts[1].x, verts[1].y, verts[1].z, c, c);
	RenderOneLine(verts[1].x, verts[1].y, verts[1].z, verts[3].x, verts[3].y, verts[3].z, c, c);
	RenderOneLine(verts[3].x, verts[3].y, verts[3].z, verts[2].x, verts[2].y, verts[2].z, c, c);
	RenderOneLine(verts[2].x, verts[2].y, verts[2].z, verts[0].x, verts[0].y, verts[0].z, c, c);

	RenderOneLine(verts[0+4].x, verts[0+4].y, verts[0+4].z, verts[1+4].x, verts[1+4].y, verts[1+4].z, c, c);
	RenderOneLine(verts[1+4].x, verts[1+4].y, verts[1+4].z, verts[3+4].x, verts[3+4].y, verts[3+4].z, c, c);
	RenderOneLine(verts[3+4].x, verts[3+4].y, verts[3+4].z, verts[2+4].x, verts[2+4].y, verts[2+4].z, c, c);
	RenderOneLine(verts[2+4].x, verts[2+4].y, verts[2+4].z, verts[0+4].x, verts[0+4].y, verts[0+4].z, c, c);

	RenderOneLine(verts[0].x, verts[0].y, verts[0].z, verts[4].x, verts[4].y, verts[4].z, c, c);
	RenderOneLine(verts[1].x, verts[1].y, verts[1].z, verts[5].x, verts[5].y, verts[5].z, c, c);
	RenderOneLine(verts[2].x, verts[2].y, verts[2].z, verts[6].x, verts[6].y, verts[6].z, c, c);
	RenderOneLine(verts[3].x, verts[3].y, verts[3].z, verts[7].x, verts[7].y, verts[7].z, c, c);
}

static void
RenderWireBox(const CMatrix &mat, const CVector &min, const CVector &max, rw::RGBA col)
{
	CVector verts[8];
	verts[0].x = min.x;
	verts[0].y = min.y;
	verts[0].z = min.z;
	verts[1].x = max.x;
	verts[1].y = min.y;
	verts[1].z = min.z;
	verts[2].x = min.x;
	verts[2].y = max.y;
	verts[2].z = min.z;
	verts[3].x = max.x;
	verts[3].y = max.y;
	verts[3].z = min.z;
	verts[4].x = min.x;
	verts[4].y = min.y;
	verts[4].z = max.z;
	verts[5].x = max.x;
	verts[5].y = min.y;
	verts[5].z = max.z;
	verts[6].x = min.x;
	verts[6].y = max.y;
	verts[6].z = max.z;
	verts[7].x = max.x;
	verts[7].y = max.y;
	verts[7].z = max.z;
	rw::V3d::transformPoints((rw::V3d*)verts, (rw::V3d*)verts, 8, &mat.m_matrix);
	RenderWireBoxVerts(verts, col);
}

static void
RenderWireSphere(const CVector &center, float radius, rw::RGBA col)
{
	uint32 c = RWRGBAINT(col.red, col.green, col.blue, col.alpha);
	RenderOneLine(center.x, center.y, center.z - radius, center.x - radius, center.y - radius, center.z, c, c);
	RenderOneLine(center.x, center.y, center.z - radius, center.x - radius, center.y + radius, center.z, c, c);
	RenderOneLine(center.x, center.y, center.z - radius, center.x + radius, center.y - radius, center.z, c, c);
	RenderOneLine(center.x, center.y, center.z - radius, center.x + radius, center.y + radius, center.z, c, c);
	RenderOneLine(center.x - radius, center.y - radius, center.z, center.x, center.y, center.z + radius, c, c);
	RenderOneLine(center.x - radius, center.y + radius, center.z, center.x, center.y, center.z + radius, c, c);
	RenderOneLine(center.x + radius, center.y - radius, center.z, center.x, center.y, center.z + radius, c, c);
	RenderOneLine(center.x + radius, center.y + radius, center.z, center.x, center.y, center.z + radius, c, c);
}

static void
RenderWireTri(CVector *verts, rw::RGBA col)
{
	uint32 c = RWRGBAINT(col.red, col.green, col.blue, col.alpha);
	RenderOneLine(verts[0].x, verts[0].y, verts[0].z, verts[1].x, verts[1].y, verts[1].z, c, c);
	RenderOneLine(verts[0].x, verts[0].y, verts[0].z, verts[2].x, verts[2].y, verts[2].z, c, c);
	RenderOneLine(verts[2].x, verts[2].y, verts[2].z, verts[1].x, verts[1].y, verts[1].z, c, c);
}

void
CCollision::DrawColModel(const CMatrix &mat, const CColModel &colModel)
{
	static rw::RGBA red = { 255, 0, 0, 255 };
	static rw::RGBA green = { 0, 255, 0, 255 };
	static rw::RGBA magenta = { 255, 0, 255, 255 };
	static rw::RGBA cyan = { 0, 255, 255, 255 };
	static rw::RGBA white = { 255, 255, 255, 255 };
	uint32 c;
	int i;
	CVector verts[3];

	rw::SetRenderState(rw::ZWRITEENABLE, 1);
	rw::SetRenderState(rw::VERTEXALPHA, 1);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
	rw::engine->imtexture = nil;

	RenderWireBox(mat, colModel.boundingBox.min, colModel.boundingBox.max, red);

	for(i = 0; i < colModel.numSpheres; i++){
		CVector center = mat * colModel.spheres[i].center;
		RenderWireSphere(center, colModel.spheres[i].radius, magenta);
	}

	for(i = 0; i < colModel.numBoxes; i++)
		RenderWireBox(mat, colModel.boxes[i].min, colModel.boxes[i].max, white);

	c = RWRGBAINT(cyan.red, cyan.green, cyan.blue, cyan.alpha);
	for(i = 0; i < colModel.numLines; i++){
		verts[0] = colModel.lines[i].p0;
		verts[1] = colModel.lines[i].p1;
		rw::V3d::transformPoints((rw::V3d*)verts, (rw::V3d*)verts, 2, &mat.m_matrix);
		RenderOneLine(verts[0].x, verts[0].y, verts[0].z,
			verts[1].x, verts[1].y, verts[1].z, c, c);
	}

	for(i = 0; i < colModel.numTriangles; i++){
		verts[0] = colModel.vertices[colModel.triangles[i].a];
		verts[1] = colModel.vertices[colModel.triangles[i].b];
		verts[2] = colModel.vertices[colModel.triangles[i].c];
		rw::V3d::transformPoints((rw::V3d*)verts, (rw::V3d*)verts, 3, &mat.m_matrix);
		RenderWireTri(verts, green);
	}

	RenderAndEmptyRenderBuffer();

	rw::SetRenderState(rw::ZWRITEENABLE, 1);
	rw::SetRenderState(rw::ZTESTENABLE, 1);
	rw::SetRenderState(rw::VERTEXALPHA, 0);
	rw::SetRenderState(rw::SRCBLEND, rw::BLENDSRCALPHA);
	rw::SetRenderState(rw::DESTBLEND, rw::BLENDINVSRCALPHA);
}
