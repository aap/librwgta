#include <d3d9.h>
#include "storiesview.h"

using namespace rw;
using namespace d3d;
using namespace d3d9;

// Does not support material colour right now
void
buildingRenderCB(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix world;
	Geometry *geo = atomic->geometry;

	d3d::setRenderState(D3DRS_LIGHTING, 1);

	Frame *f = atomic->getFrame();
	convMatrix(&world, f->getLTM());
	d3ddevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&world);

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	d3d::setTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	d3d::setTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);
		const static rw::RGBA white = { 255, 255, 255, 255 };
		D3DMATERIAL9 d3dmat;
		D3DCOLORVALUE black = { 0, 0, 0, 255 };
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
		d3d::setRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		d3d::setRenderState(D3DRS_DIFFUSEMATERIALSOURCE, inst->vertexAlpha ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);

		if(inst->material->texture){
			// Texture
			d3d::setTexture(0, inst->material->texture);
			d3d::setTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE2X);
			d3d::setTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
			d3d::setTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			d3d::setTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			d3d::setTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
			d3d::setTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
		}else{
			d3d::setTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			d3d::setTextureStageState(0, D3DTSS_COLORARG1, D3DTA_CURRENT);
			d3d::setTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
			d3d::setTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_CURRENT);
		}

		drawInst_GSemu(header, inst);
		inst++;
	}
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
