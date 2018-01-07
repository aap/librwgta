#include "euryopa.h"

#ifdef RW_GL3

using namespace rw;
using namespace gl3;

enum AttribIndices
{
	ATTRIB_NIGHTCOLOR = 2,
	ATTRIB_DAYCOLOR = 4,
};

static gl3::Shader *ps2BuildingShader, *ps2BuildingFXShader;
int32 u_dayparam, u_nightparam;	// DN
int32 u_texmat;	// UVA
int32 u_envmat, u_envXform, u_shininess;	// EnvMap
int32 u_colorscale;

#define U(i) currentShader->uniformLocations[i]

#define GETEXTRACOLOREXT(g) PLUGINOFFSET(gta::ExtraVertColors, g, gta::extraVertColorOffset)

static void
buildingInstanceCB(Geometry *geo, gl3::InstanceDataHeader *header)
{
	AttribDesc attribs[12], *a, *b;
	uint32 stride;
	gta::ExtraVertColors *extracols = GETEXTRACOLOREXT(geo);
	V3d *extranormals = gta::getExtraNormals(geo);

	//
	// Create attribute descriptions
	//
	a = attribs;
	stride = 0;

	// Positions
	a->index = ATTRIB_POS;
	a->size = 3;
	a->type = GL_FLOAT;
	a->normalized = GL_FALSE;
	a->offset = stride;
	stride += 12;
	a++;

	// Normals
	// TODO: compress
	bool hasNormals = !!(geo->flags & Geometry::NORMALS);
	if(hasNormals || extranormals){
		a->index = ATTRIB_NORMAL;
		a->size = 3;
		a->type = GL_FLOAT;
		a->normalized = GL_FALSE;
		a->offset = stride;
		stride += 12;
		a++;
	}

	// Prelighting
	bool isPrelit = !!(geo->flags & Geometry::PRELIT);
	if(isPrelit){
		a->index = ATTRIB_NIGHTCOLOR;
		a->size = 4;
		a->type = GL_UNSIGNED_BYTE;
		a->normalized = GL_TRUE;
		a->offset = stride;
		stride += 4;
		a++;

		a->index = ATTRIB_DAYCOLOR;
		a->size = 4;
		a->type = GL_UNSIGNED_BYTE;
		a->normalized = GL_TRUE;
		a->offset = stride;
		stride += 4;
		a++;
	}else{
		// we need vertex colors in the shader so force white like on PS2, one set is enough
		a->index = ATTRIB_NIGHTCOLOR;
		a->size = 4;
		a->type = GL_UNSIGNED_BYTE;
		a->normalized = GL_TRUE;
		a->offset = stride;
		stride += 4;
		a++;
	}

	// Texture coordinates
	for(int32 n = 0; n < geo->numTexCoordSets; n++){
		a->index = ATTRIB_TEXCOORDS0+n;
		a->size = 2;
		a->type = GL_FLOAT;
		a->normalized = GL_FALSE;
		a->offset = stride;
		stride += 8;
		a++;
	}

	header->numAttribs = a - attribs;
	for(a = attribs; a != &attribs[header->numAttribs]; a++)
		a->stride = stride;
	header->attribDesc = rwNewT(AttribDesc, header->numAttribs, MEMDUR_EVENT | ID_GEOMETRY);
	memcpy(header->attribDesc, attribs,
	       header->numAttribs*sizeof(AttribDesc));

	//
	// Allocate and fill vertex buffer
	//
	uint8 *verts = rwNewT(uint8, header->totalNumVertex*stride, MEMDUR_EVENT | ID_GEOMETRY);
	header->vertexBuffer = verts;

	// Positions
	for(a = attribs; a->index != ATTRIB_POS; a++)
		;
	instV3d(VERT_FLOAT3, verts + a->offset,
	        geo->morphTargets[0].vertices,
	        header->totalNumVertex, a->stride);

	// Normals
	if(hasNormals || extranormals){
		for(a = attribs; a->index != ATTRIB_NORMAL; a++)
			;
		instV3d(VERT_FLOAT3, verts + a->offset,
			hasNormals ? geo->morphTargets[0].normals : extranormals,
			header->totalNumVertex, a->stride);
	}

	// Prelighting
	if(isPrelit){
		for(a = attribs; a->index != ATTRIB_NIGHTCOLOR; a++)
			;
		for(b = attribs; b->index != ATTRIB_DAYCOLOR; b++)
			;
		int n = header->numMeshes;
		InstanceData *inst = header->inst;
		rw::RGBA *dayColors = geo->colors; //extracols->dayColors;
		rw::RGBA *nightColors = extracols->nightColors;
//		if(dayColors == nil) dayColors = geo->colors;
		if(nightColors == nil) nightColors = dayColors;
		while(n--){
			inst->vertexAlpha = instColor(VERT_RGBA,
				verts + a->offset + a->stride*inst->minVert,
				nightColors + inst->minVert,
				inst->numVertices, a->stride);
			inst->vertexAlpha |= instColor(VERT_RGBA,
				verts + b->offset + b->stride*inst->minVert,
				dayColors + inst->minVert,
				inst->numVertices, b->stride);
			inst++;
		}
	}else{
		for(a = attribs; a->index != ATTRIB_NIGHTCOLOR; a++)
			;
		int n = header->numMeshes;
		InstanceData *inst = header->inst;
		while(n--){
			inst->vertexAlpha = 0;
			inst++;
		}
		instWhite(VERT_RGBA, verts + a->offset,
			header->totalNumVertex, a->stride);
	}

	// Texture coordinates
	for(int32 n = 0; n < geo->numTexCoordSets; n++){
		for(a = attribs; a->index != ATTRIB_TEXCOORDS0+n; a++)
			;
		instTexCoords(VERT_FLOAT2, verts + a->offset,
			geo->texCoords[n],
			header->totalNumVertex, a->stride);
	}

	glGenBuffers(1, &header->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, header->vbo);
	glBufferData(GL_ARRAY_BUFFER, header->totalNumVertex*stride,
	             header->vertexBuffer, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static void
buildingRenderCB(Atomic *atomic, gl3::InstanceDataHeader *header)
{
	RawMatrix ident;
	Matrix *texmat;
	Material *m;
	RGBAf col;
	GLfloat surfProps[4];

	RawMatrix::setIdentity(&ident);
	setWorldMatrix(atomic->getFrame()->getLTM());
	lightingCB(0);

	glBindBuffer(GL_ARRAY_BUFFER, header->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, header->ibo);
	setAttribPointers(header->attribDesc, header->numAttribs);

	InstanceData *inst = header->inst;
	int32 n = header->numMeshes;

	ps2BuildingShader->use();

	float dayparam[4], nightparam[4];
	if(atomic->pipeline->pluginData == gta::RSPIPE_PC_CustomBuilding_PipeID){
		dayparam[0] = dayparam[1] = dayparam[2] = dayparam[3] = 0.0f;
		nightparam[0] = nightparam[1] = nightparam[2] = nightparam[3] = 1.0f;
	}else{
		dayparam[0] = dayparam[1] = dayparam[2] = 1.0f-gDayNightBalance;
		nightparam[0] = nightparam[1] = nightparam[2] = gDayNightBalance;
		dayparam[3] = gWetRoadEffect;
		nightparam[3] = (1.0f-gWetRoadEffect);
	}

	glUniform4fv(U(u_dayparam), 1, dayparam);
	glUniform4fv(U(u_nightparam), 1, nightparam);

	RawMatrix envmat;
	GetBuildingEnvMatrix(atomic, nil, &envmat);

	while(n--){
		m = inst->material;

		float colorscale = 1.0f;
		if(m->texture)
			colorscale = 255.0f/128.0f;
		setTexture(0, m->texture);
		glUniform1fv(U(u_colorscale), 1, &colorscale);

		int hasEnv = (*(uint32*)&inst->material->surfaceProps.specular) & 1;
		gta::EnvMat *env = gta::getEnvMat(inst->material);

		convColor(&col, &m->color);
		glUniform4fv(U(u_matColor), 1, (GLfloat*)&col);

		surfProps[0] = m->surfaceProps.ambient;
		surfProps[1] = m->surfaceProps.specular;
		surfProps[2] = m->surfaceProps.diffuse;
		surfProps[3] = 0.0f;
		glUniform4fv(U(u_surfaceProps), 1, surfProps);

		// UV animation
		if(MatFX::getEffects(inst->material) == MatFX::UVTRANSFORM){
			MatFX *matfx = MatFX::get(inst->material);
			matfx->getUVTransformMatrices(&texmat, nil);
			if(texmat)
				glUniformMatrix4fv(U(u_texmat), 1, 0, (float*)texmat);
			else
				glUniformMatrix4fv(U(u_texmat), 1, 0, (float*)&ident);
		}else
			glUniformMatrix4fv(U(u_texmat), 1, 0, (float*)&ident);

		rw::SetRenderState(VERTEXALPHA, inst->vertexAlpha || m->color.alpha != 0xFF);

		flushCache();
		glDrawElements(header->primType, inst->numIndex,
		               GL_UNSIGNED_SHORT, (void*)(uintptr)inst->offset);


		if(hasEnv){
			ps2BuildingFXShader->use();
			glUniformMatrix4fv(U(u_envmat), 1, 0, (float*)&envmat);

			setTexture(0, env->texture);

			float envxform[4];
			envxform[0] = envxform[1] = 0.0f;
			envxform[2] = env->getScaleX();
			envxform[3] = env->getScaleY();
			glUniform4fv(U(u_envXform), 1, envxform);

			float shininess;
			shininess = env->getShininess();
			glUniform1fv(U(u_shininess), 1, &shininess);

			int dst;
			dst = GetRenderState(DESTBLEND);
			SetRenderState(DESTBLEND, BLENDONE);
			SetRenderState(VERTEXALPHA, 1);

			flushCache();
			glDrawElements(header->primType, inst->numIndex,
			               GL_UNSIGNED_SHORT, (void*)(uintptr)inst->offset);

			SetRenderState(DESTBLEND, dst);
			ps2BuildingShader->use();
		}

		inst++;
	}
	disableAttribPointers(header->attribDesc, header->numAttribs);
}

void
MakeCustomBuildingPipelines(void)
{
	gl3::ObjPipeline *pipe;

	u_dayparam = registerUniform("u_dayparam");
	u_nightparam = registerUniform("u_nightparam");
	u_texmat = registerUniform("u_texmat");
	u_envmat = registerUniform("u_envmat");
	u_envXform = registerUniform("u_envXform");
	u_shininess = registerUniform("u_shininess");
	u_colorscale = registerUniform("u_colorscale");

#include "gl_shaders/ps2Building_vs_gl3.inc"
#include "gl_shaders/ps2Building_fs_gl3.inc"
	ps2BuildingShader = Shader::fromStrings(ps2Building_vert_src, ps2Building_frag_src);
#include "gl_shaders/ps2BuildingFX_vs_gl3.inc"
#include "gl_shaders/ps2Env_fs_gl3.inc"
	ps2BuildingFXShader = Shader::fromStrings(ps2BuildingFX_vert_src, ps2Env_frag_src);
	assert(ps2BuildingFXShader);

	pipe = new gl3::ObjPipeline(PLATFORM_GL3);
	pipe->pluginID = gta::RSPIPE_PC_CustomBuilding_PipeID;
	pipe->pluginData = gta::RSPIPE_PC_CustomBuilding_PipeID;
	pipe->instanceCB = buildingInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = buildingRenderCB;
	buildingPipe = pipe;

	pipe = new gl3::ObjPipeline(PLATFORM_GL3);
	pipe->pluginID = gta::RSPIPE_PC_CustomBuildingDN_PipeID;
	pipe->pluginData = gta::RSPIPE_PC_CustomBuildingDN_PipeID;
	pipe->instanceCB = buildingInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = buildingRenderCB;
	buildingDNPipe = pipe;
}


#endif
