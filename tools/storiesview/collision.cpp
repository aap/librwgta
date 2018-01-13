#include "storiesview.h"

void
LoadCollisionFile(int id, uint8 *data)
{
	struct ColChkEntry {
		int32 id;
		void *colmodel;
	} *col;

	uint8 *chunk;
	sChunkHeader *header = (sChunkHeader*)data;
	if(header->ident != COL2_IDENT)
		return;
	chunk = rwNewT(uint8, header->fileSize, 0);
	memcpy(chunk, data, header->fileSize);
	cReloctableChunk(header->ident, header->shrink).Fixup(chunk);

	pColPool->items[id].chunkData = chunk;
	CBaseModelInfo *mi;
	for(col = (ColChkEntry*)(chunk + sizeof(sChunkHeader));
	    col->id >= 0;
	    col++){
		mi = CModelInfo::Get(col->id);
		assert(mi);
		mi->colModel = (CColModel*)col->colmodel;
	}
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

//	RenderWireBox(&col->boundingBox, red, xform);
	RenderWireSphere(&col->boundingSphere, red, xform);
	if(onlyBounds)
		return;
	for(i = 0; i < col->numBoxes; i++)
		RenderWireBox(&col->boxes[i].box, white, xform);
	for(i = 0; i < col->numSpheres; i++)
		RenderWireSphere(&col->spheres[i].sph, magenta, xform);
	for(i = 0; i < col->numTriangles; i++){
		tri = &col->triangles[i];
		rw::V3d v[3];
		v[0] = *(rw::V3d*)&col->vertices[tri->a/6].Uncompress();
		v[1] = *(rw::V3d*)&col->vertices[tri->b/6].Uncompress();
		v[2] = *(rw::V3d*)&col->vertices[tri->c/6].Uncompress();
		RenderWireTriangle(&v[0], &v[1], &v[2], green, xform);
	}
}
