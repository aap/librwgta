#include "euryopa.h"

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
