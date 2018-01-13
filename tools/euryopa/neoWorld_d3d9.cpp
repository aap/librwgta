#include "euryopa.h"

using namespace rw;

#ifdef RW_D3D9

using namespace d3d;
using namespace d3d9;

rw::ObjPipeline *neoWorldPipe;

static void *neoWorldIII_PS;
static void *neoWorldVC_PS;

static void
neoWorldRenderCB(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix world;
	Geometry *geo = atomic->geometry;

	int lighting = !!(geo->flags & rw::Geometry::LIGHT);
	if(lighting)
		d3d::lightingCB(!!(geo->flags & rw::Geometry::NORMALS));

	d3d::setRenderState(D3DRS_LIGHTING, lighting);

	Frame *f = atomic->getFrame();
	convMatrix(&world, f->getLTM());
	d3ddevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&world);

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	if(params.neoWorldPipe == GAME_III)
		setPixelShader(neoWorldIII_PS);
	else
		setPixelShader(neoWorldVC_PS);

	float lightfactor[4];

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		if(MatFX::getEffects(inst->material) == MatFX::DUAL){
			MatFX *matfx = MatFX::get(inst->material);
			Texture *dualtex = matfx->getDualTexture();
			if(dualtex == nil)
				goto notex;
			d3d::setTexture(1, dualtex);
			lightfactor[0] = lightfactor[1] = lightfactor[2] = gNeoLightMapStrength;
		}else{
		notex:
			d3d::setTexture(1, nil);
			lightfactor[0] = lightfactor[1] = lightfactor[2] = 0.0f;
		}
		lightfactor[3] = inst->material->color.alpha/255.0f;
		d3d::setTexture(0, inst->material->texture);
		d3ddevice->SetPixelShaderConstantF(0, lightfactor, 1);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		rw::RGBA col = { 255, 255, 255, 255 };
		d3d::setMaterial(inst->material->surfaceProps, col);

		d3d::setRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);
		if(geo->flags & Geometry::PRELIT)
			d3d::setRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1);
		else
			d3d::setRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);
		d3d::setRenderState(D3DRS_DIFFUSEMATERIALSOURCE, inst->vertexAlpha ? D3DMCS_COLOR1 : D3DMCS_MATERIAL);


		if(params.ps2AlphaTest)
			drawInst_GSemu(header, inst);
		else
			drawInst(header, inst);

		inst++;
	}
	d3ddevice->SetPixelShader(nil);
}


void
MakeNeoWorldPipe(void)
{
#include "d3d_shaders/neoWorldIII_PS.inc"
#include "d3d_shaders/neoWorldVC_PS.inc"
	neoWorldIII_PS = createPixelShader(neoWorldIII_PS_cso);
	neoWorldVC_PS = createPixelShader(neoWorldVC_PS_cso);

	d3d9::ObjPipeline *pipe;
	pipe = new d3d9::ObjPipeline(PLATFORM_D3D9);
	pipe->instanceCB = d3d9::defaultInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = neoWorldRenderCB;
	neoWorldPipe = pipe;
}

#endif
