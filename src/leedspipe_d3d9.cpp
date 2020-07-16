#define WITH_D3D
#include <rw.h>
#include "rwgta.h"

namespace gta {

#ifdef RW_D3D9

using namespace rw;
using namespace d3d;
using namespace d3d9;

enum {
	VSLOC_emissive = VSLOC_lightOffset,

	PSLOC_colorscale = 1
};

rw::RGBAf leedsPipe_amb;
rw::RGBAf leedsPipe_emiss;

static void *leedsPS2VS;
static void *simple4PS;

rw::ObjPipeline *leedsPipe;

void
leedsRenderCB_PS2(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	float colorscale[4];
	Geometry *geo = atomic->geometry;

	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	d3ddevice->SetVertexShaderConstantF(VSLOC_fogData, (float*)&d3dShaderState.fogData, 1);
	d3ddevice->SetPixelShaderConstantF(PSLOC_fogColor, (float*)&d3dShaderState.fogColor, 1);

	setVertexShader(leedsPS2VS);
	setPixelShader(simple4PS);

	uploadMatrices(atomic->getFrame()->getLTM());

	d3ddevice->SetVertexShaderConstantF(VSLOC_ambLight, (float*)&leedsPipe_amb, 1);
	d3ddevice->SetVertexShaderConstantF(VSLOC_emissive, (float*)&leedsPipe_emiss, 1);

	colorscale[3] = 1.0f;

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		float cs = 1.0f;
		if(inst->material->texture)
			cs = 255/128.0f;
		colorscale[0] = colorscale[1] = colorscale[2] = cs;
		d3ddevice->SetPixelShaderConstantF(PSLOC_colorscale, colorscale, 1);

		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(VSLOC_matColor, (float*)&col, 1);
		float surfprops[4];
		surfprops[0] = inst->material->surfaceProps.ambient;
		surfprops[1] = inst->material->surfaceProps.specular;
		surfprops[2] = inst->material->surfaceProps.diffuse;
		surfprops[3] = 0.5f;
		d3ddevice->SetVertexShaderConstantF(VSLOC_surfProps, surfprops, 1);

		drawInst(header, inst);

		inst++;
	}
}

void
leedsRenderCB_PSP(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	float colorscale[4];
	Geometry *geo = atomic->geometry;

	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	d3ddevice->SetVertexShaderConstantF(VSLOC_fogData, (float*)&d3dShaderState.fogData, 1);
	d3ddevice->SetPixelShaderConstantF(PSLOC_fogColor, (float*)&d3dShaderState.fogColor, 1);

	setVertexShader(leedsPS2VS);
	setPixelShader(simple4PS);

	uploadMatrices(atomic->getFrame()->getLTM());

	d3ddevice->SetVertexShaderConstantF(VSLOC_ambLight, (float*)&leedsPipe_amb, 1);
	d3ddevice->SetVertexShaderConstantF(VSLOC_emissive, (float*)&leedsPipe_emiss, 1);

	colorscale[3] = 1.0f;

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		float cs = 1.0f;
		if(inst->material->texture)
			cs = 2.0f;
		colorscale[0] = colorscale[1] = colorscale[2] = cs;
		d3ddevice->SetPixelShaderConstantF(PSLOC_colorscale, colorscale, 1);

		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(VSLOC_matColor, (float*)&col, 1);
		float surfprops[4];
		surfprops[0] = inst->material->surfaceProps.ambient;
		surfprops[1] = inst->material->surfaceProps.specular;
		surfprops[2] = inst->material->surfaceProps.diffuse;
		surfprops[3] = 1.22f;
		d3ddevice->SetVertexShaderConstantF(VSLOC_surfProps, surfprops, 1);

		drawInst(header, inst);

		inst++;
	}
}


void
leedsRenderCB_mobile(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	float colorscale[4];
	Geometry *geo = atomic->geometry;

	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	d3ddevice->SetVertexShaderConstantF(VSLOC_fogData, (float*)&d3dShaderState.fogData, 1);
	d3ddevice->SetPixelShaderConstantF(PSLOC_fogColor, (float*)&d3dShaderState.fogColor, 1);

	setVertexShader(leedsPS2VS);
	setPixelShader(simple4PS);

	uploadMatrices(atomic->getFrame()->getLTM());

	d3ddevice->SetVertexShaderConstantF(VSLOC_ambLight, (float*)&leedsPipe_amb, 1);
	d3ddevice->SetVertexShaderConstantF(VSLOC_emissive, (float*)&leedsPipe_emiss, 1);
	
	colorscale[0] = colorscale[1] = colorscale[2] = colorscale[3] = 1.0f;
	d3ddevice->SetPixelShaderConstantF(PSLOC_colorscale, colorscale, 1);

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(VSLOC_matColor, (float*)&col, 1);
		float surfprops[4];
		surfprops[0] = inst->material->surfaceProps.ambient;
		surfprops[1] = inst->material->surfaceProps.specular;
		surfprops[2] = inst->material->surfaceProps.diffuse;
		surfprops[3] = 1.0f;
		d3ddevice->SetVertexShaderConstantF(VSLOC_surfProps, surfprops, 1);

		drawInst(header, inst);

		inst++;
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
	pipe->renderCB = leedsRenderCB_PS2;	// default to PS2 for now
	leedsPipe = pipe;
}

#endif

}
