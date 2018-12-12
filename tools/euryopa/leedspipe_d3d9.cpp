#include "euryopa.h"

#ifdef RW_D3D9

using namespace rw;
using namespace d3d;
using namespace d3d9;

enum {
	REG_transform	= 0,
	REG_ambient	= 4,
	REG_emissive	= 5,
	REG_matCol	= 19,
	REG_surfProps	= 20,

	REG_shaderParams= 29,
};

static void *leedsPS2VS;
static void *simple4PS;

rw::ObjPipeline *leedsPipe;

void getComposedMatrix(Atomic *atm, RawMatrix *combined);
void defaultRenderCB_GSemu(Atomic *atomic, d3d9::InstanceDataHeader *header);

static void
leedsRenderCB_PS2(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix combined;
	float colorscale[4];
	Geometry *geo = atomic->geometry;

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	setVertexShader(leedsPS2VS);
	setPixelShader(simple4PS);

	getComposedMatrix(atomic, &combined);
	d3ddevice->SetVertexShaderConstantF(REG_transform, (float*)&combined, 4);

	rw::RGBAf amb = Timecycle::currentColours.amb;
	rw::RGBAf emiss = Timecycle::currentColours.amb_bl;
	d3ddevice->SetVertexShaderConstantF(REG_ambient, (float*)&amb, 1);
	d3ddevice->SetVertexShaderConstantF(REG_emissive, (float*)&emiss, 1);

	colorscale[3] = 1.0f;

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		float cs = 1.0f;
		if(inst->material->texture)
			cs = 255/128.0f;
		colorscale[0] = colorscale[1] = colorscale[2] = cs;
		d3ddevice->SetPixelShaderConstantF(0, colorscale, 1);

		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(REG_matCol, (float*)&col, 1);
		float surfprops[4];
		surfprops[0] = inst->material->surfaceProps.ambient;
		surfprops[1] = inst->material->surfaceProps.specular;
		surfprops[2] = inst->material->surfaceProps.diffuse;
		surfprops[3] = 0.5f;
		d3ddevice->SetVertexShaderConstantF(REG_surfProps, surfprops, 1);

		if(params.ps2AlphaTest)
			drawInst_GSemu(header, inst);
		else
			drawInst(header, inst);

		inst++;
	}

	d3ddevice->SetVertexShader(nil);
	d3ddevice->SetPixelShader(nil);
	d3d9UsedVertexShader = true;
}

static void
leedsRenderCB_PSP(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix combined;
	float colorscale[4];
	Geometry *geo = atomic->geometry;

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	setVertexShader(leedsPS2VS);
	setPixelShader(simple4PS);

	getComposedMatrix(atomic, &combined);
	d3ddevice->SetVertexShaderConstantF(REG_transform, (float*)&combined, 4);

	rw::RGBAf amb = Timecycle::currentColours.amb;
	rw::RGBAf emiss = Timecycle::currentColours.amb_bl;
	d3ddevice->SetVertexShaderConstantF(REG_ambient, (float*)&amb, 1);
	d3ddevice->SetVertexShaderConstantF(REG_emissive, (float*)&emiss, 1);

	colorscale[3] = 1.0f;

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		float cs = 1.0f;
		if(inst->material->texture)
			cs = 2.0f;
		colorscale[0] = colorscale[1] = colorscale[2] = cs;
		d3ddevice->SetPixelShaderConstantF(0, colorscale, 1);

		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(REG_matCol, (float*)&col, 1);
		float surfprops[4];
		surfprops[0] = inst->material->surfaceProps.ambient;
		surfprops[1] = inst->material->surfaceProps.specular;
		surfprops[2] = inst->material->surfaceProps.diffuse;
		surfprops[3] = 1.22f;
		d3ddevice->SetVertexShaderConstantF(REG_surfProps, surfprops, 1);

		if(params.ps2AlphaTest)
			drawInst_GSemu(header, inst);
		else
			drawInst(header, inst);

		inst++;
	}

	d3ddevice->SetVertexShader(nil);
	d3ddevice->SetPixelShader(nil);
	d3d9UsedVertexShader = true;
}


static void
leedsRenderCB_mobile(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	RawMatrix combined;
	float colorscale[4];
	Geometry *geo = atomic->geometry;

	d3ddevice->SetStreamSource(0, (IDirect3DVertexBuffer9*)header->vertexStream[0].vertexBuffer,
	                           0, header->vertexStream[0].stride);
	d3ddevice->SetIndices((IDirect3DIndexBuffer9*)header->indexBuffer);
	d3ddevice->SetVertexDeclaration((IDirect3DVertexDeclaration9*)header->vertexDeclaration);

	setVertexShader(leedsPS2VS);
	setPixelShader(simple4PS);

	getComposedMatrix(atomic, &combined);
	d3ddevice->SetVertexShaderConstantF(REG_transform, (float*)&combined, 4);

	rw::RGBAf amb = Timecycle::currentColours.amb;
	rw::RGBAf emiss = Timecycle::currentColours.amb_bl;
	d3ddevice->SetVertexShaderConstantF(REG_ambient, (float*)&amb, 1);
	d3ddevice->SetVertexShaderConstantF(REG_emissive, (float*)&emiss, 1);
	
	colorscale[0] = colorscale[1] = colorscale[2] = colorscale[3] = 1.0f;
	d3ddevice->SetPixelShaderConstantF(0, colorscale, 1);

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(REG_matCol, (float*)&col, 1);
		float surfprops[4];
		surfprops[0] = inst->material->surfaceProps.ambient;
		surfprops[1] = inst->material->surfaceProps.specular;
		surfprops[2] = inst->material->surfaceProps.diffuse;
		surfprops[3] = 1.0f;
		d3ddevice->SetVertexShaderConstantF(REG_surfProps, surfprops, 1);

		if(params.ps2AlphaTest)
			drawInst_GSemu(header, inst);
		else
			drawInst(header, inst);

		inst++;
	}

	d3ddevice->SetVertexShader(nil);
	d3ddevice->SetPixelShader(nil);
	d3d9UsedVertexShader = true;
}

static void
leedsRenderCB(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	switch(gBuildingPipeSwitch){
	case PLATFORM_NULL:
		defaultRenderCB_GSemu(atomic, header);
		break;
	case PLATFORM_PS2:
		leedsRenderCB_PS2(atomic, header);
		break;
	case PLATFORM_PSP:
		leedsRenderCB_PSP(atomic, header);
		break;
	// TEST
	case PLATFORM_PC:
		leedsRenderCB_mobile(atomic, header);
		break;
	}
}

void
MakeLeedsPipe(void)
{
#include "d3d_shaders/leedsPS2VS.inc"
#include "d3d_shaders/simple4PS.inc"
	leedsPS2VS = createVertexShader(leedsPS2VS_cso);
	simple4PS = createPixelShader(simple4PS_cso);

	d3d9::ObjPipeline *pipe;
	pipe = new d3d9::ObjPipeline(PLATFORM_D3D9);
	pipe->instanceCB = d3d9::defaultInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = leedsRenderCB;
	leedsPipe = pipe;
}

#endif
