#include "storiesview.h"

using namespace rw;
using namespace d3d;
using namespace d3d9;

void
buildingRenderCB(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix world;

	Geometry *geo = atomic->geometry;
	int lighting = !!(geo->flags & rw::Geometry::LIGHT);
	if(lighting)
		d3d::lightingCB();

	d3d::setRenderState(D3DRS_LIGHTING, lighting);

	Frame *f = atomic->getFrame();
	convMatrix(&world, f->getLTM());
	d3ddevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&world);

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	D3DMATERIAL9 d3dmat;
	D3DCOLORVALUE black = { 0, 0, 0, 0 };
	d3dmat.Ambient = black;
	d3dmat.Diffuse = black;
	d3dmat.Power = 0.0f;
	d3dmat.Emissive.r = currentEmissive.red * 0.5f / 255.0f;
	d3dmat.Emissive.g = currentEmissive.green * 0.5f / 255.0f;
	d3dmat.Emissive.b = currentEmissive.blue * 0.5f / 255.0f;
	d3dmat.Emissive.a = 0;
	d3dmat.Specular = black;
	d3d::setD3dMaterial(&d3dmat);
	d3d::setRenderState(D3DRS_AMBIENT, D3DCOLOR_RGBA(currentAmbient.red, currentAmbient.green, currentAmbient.blue, 0));

	d3d::setRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
	d3d::setRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	d3d::setRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
	setTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
//		d3d::setTexture(0, nil);
		d3d::setTexture(0, inst->material->texture);

		d3d::flushCache();
		d3ddevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)header->primType, inst->baseIndex,
		                                0, inst->numVertices,
		                                inst->startIndex, inst->numPrimitives);
		inst++;
	}
	setTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

}

rw::ObjPipeline*
makeBuildingPipe(void)
{
	d3d9::ObjPipeline *pipe = new d3d9::ObjPipeline(PLATFORM_D3D9);
	pipe->instanceCB = defaultInstanceCB;
	pipe->uninstanceCB = defaultUninstanceCB;
	pipe->renderCB = buildingRenderCB;
	return pipe;
}
