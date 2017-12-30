#include "euryopa.h"

using namespace rw;
using namespace d3d;
using namespace d3d9;

#ifdef RW_D3D9

uint32 colourCode;

void
colourCodeRenderCB(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix world;
	Geometry *geo = atomic->geometry;

	d3d::setRenderState(D3DRS_LIGHTING, 0);

	Frame *f = atomic->getFrame();
	convMatrix(&world, f->getLTM());
	d3ddevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&world);

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	InstanceData *inst = header->inst;
	uint32 blend;
	for(uint32 i = 0; i < header->numMeshes; i++){
		d3d::setTexture(0, inst->material->texture);

		d3d::getRenderState(D3DRS_ALPHABLENDENABLE, &blend);
		d3d::setRenderState(D3DRS_ALPHABLENDENABLE, 0);

		d3d::setRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(colourCode&0xFF, colourCode>>8 & 0xFF, colourCode>> 16 & 0xFF, 255));
		d3d::setTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3d::setTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		d3d::setTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		d3d::setTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

		d3d::flushCache();
		d3ddevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)header->primType, inst->baseIndex,
		                                0, inst->numVertices,
		                                inst->startIndex, inst->numPrimitives);
		d3d::setRenderState(D3DRS_ALPHABLENDENABLE, blend);
		inst++;
	}
}

rw::ObjPipeline*
makeColourCodePipeline(void)
{
	d3d9::ObjPipeline *pipe = new d3d9::ObjPipeline(PLATFORM_D3D9);
	pipe->instanceCB = defaultInstanceCB;
	pipe->uninstanceCB = defaultUninstanceCB;
	pipe->renderCB = colourCodeRenderCB;
	return pipe;
}

int32
GetColourCode(int x, int y)
{
	int32 res = 0;
	IDirect3DSurface9 *backbuffer = nil;
	d3ddevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

	D3DLOCKED_RECT d3dlr;
	D3DSURFACE_DESC desc;
	LPDIRECT3DSURFACE9 surf = nil;
	backbuffer->GetDesc(&desc);
	d3ddevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
		D3DPOOL_SYSTEMMEM, &surf, nil);
	d3ddevice->GetRenderTargetData(backbuffer, surf);

	surf->LockRect(&d3dlr, nil, D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_READONLY);
	if(desc.Format == D3DFMT_A8R8G8B8){
		uint8 *col = (uint8*)d3dlr.pBits + d3dlr.Pitch*y + x*4;
		res = col[0]<<16 | col[1]<<8 | col[2];
	}
	surf->UnlockRect();

	surf->Release();
	backbuffer->Release();
	return res;
}


#endif
