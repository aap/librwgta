#define WITH_D3D
#include "euryopa.h"

using namespace rw;

rw::RGBA colourCode;

#ifdef RW_D3D9

using namespace d3d;
using namespace d3d9;

static void *vs;

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

	// Use a vertex shader here if we're drawing over an object drawn
	// with a vertex shader to minimize z-fights
	if(d3d9UsedVertexShader){
		void getComposedMatrix(Atomic *atm, RawMatrix *combined);
		RawMatrix combined, ident;
		RawMatrix::setIdentity(&ident);
		setVertexShader(vs);
		getComposedMatrix(atomic, &combined);
		d3ddevice->SetVertexShaderConstantF(0, (float*)&combined, 4);
		d3ddevice->SetVertexShaderConstantF(32, (float*)&ident, 4);
	}

	InstanceData *inst = header->inst;
	uint32 blend;
	for(uint32 i = 0; i < header->numMeshes; i++){
		d3d::setTexture(0, inst->material->texture);
		SetRenderState(VERTEXALPHA, inst->vertexAlpha || colourCode.alpha != 255);

		d3d::getRenderState(D3DRS_ALPHABLENDENABLE, &blend);
		if(renderColourCoded)
			d3d::setRenderState(D3DRS_ALPHABLENDENABLE, 0);

		d3d::setRenderState(D3DRS_TEXTUREFACTOR, D3DCOLOR_RGBA(colourCode.red, colourCode.green, colourCode.blue, colourCode.alpha));
		d3d::setTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		d3d::setTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TFACTOR);
		d3d::setTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		d3d::setTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		d3d::setTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TFACTOR);

		d3d9::drawInst(header, inst);

		d3d::setRenderState(D3DRS_ALPHABLENDENABLE, blend);
		inst++;
	}
	setVertexShader(nil);
}

rw::ObjPipeline*
makeColourCodePipeline(void)
{
	// reuse as our vertex shader
#include "d3d_shaders/ps2BuildingVS.inc"
	vs = createVertexShader(ps2BuildingVS_cso);

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
	IDirect3DSurface9 *surf = nil;
	backbuffer->GetDesc(&desc);
	d3ddevice->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
		D3DPOOL_SYSTEMMEM, &surf, nil);
	d3ddevice->GetRenderTargetData(backbuffer, surf);

	surf->LockRect(&d3dlr, nil, D3DLOCK_NO_DIRTY_UPDATE|D3DLOCK_READONLY);
	// TODO: check format and dimensions properly
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

#ifdef RW_GL3

using namespace gl3;

Shader *colourCodeShader;

#define U(i) currentShader->uniformLocations[i]

void
colourCodeRenderCB(Atomic *atomic, gl3::InstanceDataHeader *header)
{
	Material *m;
	RGBAf col;

	setWorldMatrix(atomic->getFrame()->getLTM());
	lightingCB(0);

	glBindBuffer(GL_ARRAY_BUFFER, header->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, header->ibo);
	setAttribPointers(header->attribDesc, header->numAttribs);

	InstanceData *inst = header->inst;
	int32 n = header->numMeshes;

	colourCodeShader->use();

	while(n--){
		m = inst->material;

		convColor(&col, &colourCode);
		glUniform4fv(U(u_matColor), 1, (GLfloat*)&col);

		setTexture(0, m->texture);
		rw::SetRenderState(VERTEXALPHA, inst->vertexAlpha || colourCode.alpha != 0xFF);

		GLboolean blend;
		glGetBooleanv(GL_BLEND, &blend);
		if(renderColourCoded)
			glDisable(GL_BLEND);

		flushCache();
		glDrawElements(header->primType, inst->numIndex,
		               GL_UNSIGNED_SHORT, (void*)(uintptr)inst->offset);

		(blend ? glEnable : glDisable)(GL_BLEND);

		inst++;
	}
	disableAttribPointers(header->attribDesc, header->numAttribs);
}

rw::ObjPipeline*
makeColourCodePipeline(void)
{
#include "gl_shaders/colcode_vs_gl3.inc"
#include "gl_shaders/colcode_fs_gl3.inc"
	colourCodeShader = Shader::fromStrings(colcode_vert_src, colcode_frag_src);

	gl3::ObjPipeline *pipe = new gl3::ObjPipeline(PLATFORM_GL3);
	pipe->instanceCB = defaultInstanceCB;
	pipe->uninstanceCB = defaultUninstanceCB;
	pipe->renderCB = colourCodeRenderCB;
	return pipe;
}

int32
GetColourCode(int x, int y)
{
	rw::RGBA col;
	// TODO: check format and dimensions properly
	glReadPixels(x, sk::globals.height-y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &col);
	return col.blue<<16 | col.green<<8 | col.red;
}
#endif
