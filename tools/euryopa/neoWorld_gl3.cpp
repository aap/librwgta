#include "euryopa.h"

#ifdef RW_GL3

using namespace rw;
using namespace gl3;

rw::ObjPipeline *neoWorldPipe;

static gl3::Shader *neoWorldIIIShader, *neoWorldVCShader;
static int32 u_tex0, u_tex1, u_lm;

#define U(i) currentShader->uniformLocations[i]

static void
neoWorldRenderCB(Atomic *atomic, gl3::InstanceDataHeader *header)
{
	Material *m;
	GLfloat surfProps[4];

	setWorldMatrix(atomic->getFrame()->getLTM());
	lightingCB(!!(atomic->geometry->flags & Geometry::NORMALS));

	glBindBuffer(GL_ARRAY_BUFFER, header->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, header->ibo);
	setAttribPointers(header->attribDesc, header->numAttribs);

	InstanceData *inst = header->inst;
	int32 n = header->numMeshes;

	if(params.neoWorldPipe == GAME_III)
		neoWorldIIIShader->use();
	else
		neoWorldVCShader->use();
	glUniform1i(U(u_tex0), 0);
	glUniform1i(U(u_tex1), 1);

	RGBAf col = { 1.0f, 1.0f, 1.0f, 1.0f };
	glUniform4fv(U(u_matColor), 1, (GLfloat*)&col);

	float lightfactor[4];
	while(n--){
		m = inst->material;

		if(MatFX::getEffects(inst->material) == MatFX::DUAL){
			MatFX *matfx = MatFX::get(inst->material);
			Texture *dualtex = matfx->getDualTexture();
			if(dualtex == nil)
				goto notex;
			setTexture(1, dualtex);
			lightfactor[0] = lightfactor[1] = lightfactor[2] = gNeoLightMapStrength;
		}else{
		notex:
			setTexture(1, nil);
			lightfactor[0] = lightfactor[1] = lightfactor[2] = 0.0f;
		}
		lightfactor[3] = m->color.alpha/255.0f;
		setTexture(0, inst->material->texture);
		glUniform4fv(U(u_lm), 1, (GLfloat*)&lightfactor);

		surfProps[0] = m->surfaceProps.ambient;
		surfProps[1] = m->surfaceProps.specular;
		surfProps[2] = m->surfaceProps.diffuse;
		surfProps[3] = 0.0f;
		glUniform4fv(U(u_surfaceProps), 1, surfProps);

		rw::SetRenderState(VERTEXALPHA, inst->vertexAlpha || m->color.alpha != 0xFF);

		flushCache();
		glDrawElements(header->primType, inst->numIndex,
		               GL_UNSIGNED_SHORT, (void*)(uintptr)inst->offset);
		inst++;
	}
	disableAttribPointers(header->attribDesc, header->numAttribs);
}

void
MakeNeoWorldPipe(void)
{
	u_tex0 = registerUniform("tex0");
	u_tex1 = registerUniform("tex1");
	u_lm = registerUniform("u_lm");

#include "gl_shaders/simple_vs_gl3.inc"
#include "gl_shaders/neoWorldIII_fs_gl3.inc"
#include "gl_shaders/neoWorldVC_fs_gl3.inc"
	neoWorldIIIShader = Shader::fromStrings(simple_vert_src, neoWorldIII_frag_src);
	neoWorldVCShader = Shader::fromStrings(simple_vert_src, neoWorldVC_frag_src);

	gl3::ObjPipeline *pipe;
	pipe = new gl3::ObjPipeline(PLATFORM_D3D9);
	pipe->instanceCB = gl3::defaultInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = neoWorldRenderCB;
	neoWorldPipe = pipe;
}

#endif
