#define WITH_D3D
#include <rw.h>
#include "rwgta.h"

#include <string.h>
#include <assert.h>

namespace gta {

using namespace rw;

float buildingPipe_dayNightBalance = 0.0f;
float buildingPipe_wetRoad = 0.0f;
float buildingPipe_platformSwitch = PLATFORM_PS2;

rw::ObjPipeline *buildingPipe;
rw::ObjPipeline *buildingDNPipe;

void
ConvertXboxGeometry(Atomic *atm)
{
	Geometry *geo = atm->geometry;
	// This shouldn't happen, but better check
	if((geo->flags & Geometry::PRELIT) == 0)
		return;

	// Convert normals to extra colors
	if(geo->flags & Geometry::NORMALS){
		RGBA *daycols = geo->colors;
		if(gta::getExtraVertColors(atm) == nil)
			gta::allocateExtraVertColors(geo);
		RGBA *nightcols = gta::getExtraVertColors(atm);
		V3d *normals = geo->morphTargets[0].normals;

		int32 i;
		for(i = 0; i < geo->numVertices; i++){
			nightcols[i].red = normals[i].x*255;
			nightcols[i].green = normals[i].y*255;
			nightcols[i].blue = normals[i].z*255;
			nightcols[i].alpha = daycols[i].alpha;
		}

		geo->flags &= ~Geometry::NORMALS;
	}

	// Move extranormals to normals?
	// But then we may have to reallocate the Geometry
	// Better handle in the instanceCB
}

// similar to GTA code, some useless stuff
void
GetBuildingEnvMatrix(Atomic *atomic, Frame *envframe, RawMatrix *envmat)
{
	Matrix inv, env;
	Clump *clump;
	Frame *frame;

	if(envframe == nil)
		envframe = ((Camera*)engine->currentCamera)->getFrame();

	clump = atomic->clump;

	Matrix::invert(&inv, envframe->getLTM());
	frame = clump ? clump->getFrame() : atomic->getFrame();
	Matrix::mult(&env, frame->getLTM(), &inv);
	convMatrix(envmat, &env);
}

void
SetupBuildingEnvMap(rw::Material *m)
{
	uint32 *flags = (uint32*)&m->surfaceProps.specular;
	*flags = 0;
	gta::EnvMat *env = gta::getEnvMat(m);
	if(env && MatFX::getEffects(m) == MatFX::ENVMAP)
		env->texture = MatFX::get(m)->getEnvTexture();
	if(env && env->getShininess() != 0.0f && env->texture)
		*flags = 1;
}

bool
isBuildingPipeAttached(rw::Atomic *atm)
{
	uint32 id;

	// PS2 logic:
	if(atm->pipeline && atm->pipeline->pluginID == ID_PDS){
		id = atm->pipeline->pluginData;
		if(id == gta::PDS_PS2_CustomBuilding_AtmPipeID ||
		   id == gta::PDS_PS2_CustomBuildingDN_AtmPipeID ||
		   id == gta::PDS_PS2_CustomBuildingEnvMap_AtmPipeID ||
		   id == gta::PDS_PS2_CustomBuildingDNEnvMap_AtmPipeID ||
		   id == gta::PDS_PS2_CustomBuildingUVA_AtmPipeID ||
		   // this one is not checked by the game for some reason:
		   id == gta::PDS_PS2_CustomBuildingDNUVA_AtmPipeID)
			return true;
	}

	id = gta::getPipelineID(atm);
	// Xbox logic:
	if(id == gta::RSPIPE_XBOX_CustomBuilding_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingDN_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingEnvMap_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingDNEnvMap_PipeID)
		return true;

	// PC logic:
	if(id == gta::RSPIPE_PC_CustomBuilding_PipeID ||
	   id == gta::RSPIPE_PC_CustomBuildingDN_PipeID)
		return true;
	if(gta::getExtraVertColors(atm) && atm->geometry->colors)
		return true;

	return false;
}

void
setupBuildingPipe(rw::Atomic *atm)
{
	// ps2:
	//  obj: if has extra colors -> DN pipe else -> regular
	//  mat: if UV transform -> UVA pipe
	//       else if Env map && has env map && has normals -> envmap pipe
	//       else -> regular pipe
	// xbox: use pipeline ID but fall back to non-DN if no normals flag
	// pc: if two sets -> DN, else -> regular

	uint32 id = gta::getPipelineID(atm);
	if(id == gta::RSPIPE_XBOX_CustomBuilding_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingDN_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingEnvMap_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingDNEnvMap_PipeID)
		ConvertXboxGeometry(atm);
	// Just do the PC thing for now
	if(gta::getExtraVertColors(atm) && atm->geometry->colors){
		atm->pipeline = buildingDNPipe;
		gta::setPipelineID(atm, gta::RSPIPE_PC_CustomBuildingDN_PipeID);
	}else{
		atm->pipeline = buildingPipe;
		gta::setPipelineID(atm, gta::RSPIPE_PC_CustomBuilding_PipeID);
	}
	int i;
	for(i = 0; i < atm->geometry->matList.numMaterials; i++)
		SetupBuildingEnvMap(atm->geometry->matList.materials[i]);
}

bool32
instWhite(int type, uint8 *dst, uint32 numVertices, uint32 stride)
{
	if(type == VERT_ARGB || type == VERT_RGBA){
		for(uint32 i = 0; i < numVertices; i++){
			dst[0] = 255;
			dst[1] = 255;
			dst[2] = 255;
			dst[3] = 255;
			dst += stride;
		}
	}else
		assert(0 && "unsupported color type");
	return 0;
}




#ifdef RW_D3D9

using namespace d3d;
using namespace d3d9;

enum {
	// common
	REG_transform	= 0,
	REG_ambient	= 4,
	REG_directCol	= 5,	// 7 lights (main + 6 extra)
	REG_directDir	= 12,	//
	REG_matCol	= 19,
	REG_surfProps	= 20,

	REG_shaderParams= 29,
	// DN and UVA
	REG_dayparam	= 30,
	REG_nightparam	= 31,
	REG_texmat	= 32,
	// Env
	REG_fxParams	= 36,
	REG_envXform	= 37,
	REG_envmat	= 38,

	PSLOC_colorscale = 1
};


static void *ps2BuildingVS, *pcBuildingVS, *ps2BuildingFxVS;
static void *simplePS, *ps2EnvPS;

void
getComposedMatrix(Atomic *atm, RawMatrix *combined)
{
	RawMatrix world, worldview;
	Camera *cam = (Camera*)engine->currentCamera;
	convMatrix(&world, atm->getFrame()->getLTM());
	RawMatrix::mult(&worldview, &world, &cam->devView);
	RawMatrix::mult(combined, &worldview, &cam->devProj);
}

static void
buildingRenderCB_PS2(Atomic *atomic, d3d9::InstanceDataHeader *header)
{
	int vsBits;
	RawMatrix combined, ident;
	WorldLights lightData;
	Matrix *texmat;
	Geometry *geo = atomic->geometry;
	RawMatrix::setIdentity(&ident);

	setStreamSource(0, header->vertexStream[0].vertexBuffer, 0, header->vertexStream[0].stride);
	setIndices(header->indexBuffer);
	setVertexDeclaration(header->vertexDeclaration);

	((World*)engine->currentWorld)->enumerateLights(atomic, &lightData);

	d3ddevice->SetVertexShaderConstantF(VSLOC_fogData, (float*)&d3dShaderState.fogData, 1);
	d3ddevice->SetPixelShaderConstantF(PSLOC_fogColor, (float*)&d3dShaderState.fogColor, 1);

	setVertexShader(buildingPipe_platformSwitch == PLATFORM_PS2 ? ps2BuildingVS : pcBuildingVS);
	setPixelShader(simplePS);

	getComposedMatrix(atomic, &combined);
	d3ddevice->SetVertexShaderConstantF(REG_transform, (float*)&combined, 4);

	float dayparam[4], nightparam[4];
	if(atomic->pipeline->pluginData == gta::RSPIPE_PC_CustomBuilding_PipeID){
		dayparam[0] = dayparam[1] = dayparam[2] = dayparam[3] = 0.0f;
		nightparam[0] = nightparam[1] = nightparam[2] = nightparam[3] = 1.0f;
	}else{
		dayparam[0] = dayparam[1] = dayparam[2] = 1.0f-buildingPipe_dayNightBalance;
		nightparam[0] = nightparam[1] = nightparam[2] = buildingPipe_dayNightBalance;
		dayparam[3] = buildingPipe_wetRoad;
		nightparam[3] = (1.0f-buildingPipe_wetRoad);
	}

	d3ddevice->SetVertexShaderConstantF(REG_dayparam, dayparam, 1);
	d3ddevice->SetVertexShaderConstantF(REG_nightparam, nightparam, 1);

	d3ddevice->SetVertexShaderConstantF(REG_ambient, (float*)&lightData.ambient, 1);

	RawMatrix envmat;
	GetBuildingEnvMatrix(atomic, nil, &envmat);
	d3ddevice->SetVertexShaderConstantF(REG_envmat, (float*)&envmat, 4);

	InstanceData *inst = header->inst;
	for(uint32 i = 0; i < header->numMeshes; i++){
		float colorscale = 1.0f;
		if(inst->material->texture && buildingPipe_platformSwitch == PLATFORM_PS2)
			colorscale = 255.0f/128.0f;
		d3ddevice->SetVertexShaderConstantF(REG_shaderParams, &colorscale, 1);
		d3ddevice->SetPixelShaderConstantF(PSLOC_colorscale, &colorscale, 1);

		int hasEnv = (*(uint32*)&inst->material->surfaceProps.specular) & 1;
		gta::EnvMat *env = gta::getEnvMat(inst->material);

		d3d::setTexture(0, inst->material->texture);

		SetRenderState(VERTEXALPHA, inst->vertexAlpha || inst->material->color.alpha != 255);

		// Material colour
		rw::RGBAf col;
		convColor(&col, &inst->material->color);
		d3ddevice->SetVertexShaderConstantF(REG_matCol, (float*)&col, 1);
		d3ddevice->SetVertexShaderConstantF(REG_surfProps, (float*)&inst->material->surfaceProps, 1);

		// UV animation
		if(MatFX::getEffects(inst->material) == MatFX::UVTRANSFORM){
			MatFX *matfx = MatFX::get(inst->material);
			matfx->getUVTransformMatrices(&texmat, nil);
			if(texmat)
				d3ddevice->SetVertexShaderConstantF(REG_texmat, (float*)texmat, 4);
			else
				d3ddevice->SetVertexShaderConstantF(REG_texmat, (float*)&ident, 4);
		}else
			d3ddevice->SetVertexShaderConstantF(REG_texmat, (float*)&ident, 4);


		drawInst(header, inst);

		if(hasEnv){
			setVertexShader(ps2BuildingFxVS);
			setPixelShader(ps2EnvPS);

			d3d::setTexture(0, env->texture);

			float envxform[4];
			envxform[0] = envxform[1] = 0.0f;
			envxform[2] = env->getScaleX();
			envxform[3] = env->getScaleY();
			d3ddevice->SetVertexShaderConstantF(REG_envXform, envxform, 1);

			float fxparams[2];
			fxparams[0] = env->getShininess();
			fxparams[1] = 1.0f;
			d3ddevice->SetVertexShaderConstantF(REG_fxParams, fxparams, 1);

			int dst, fog;
			dst = GetRenderState(DESTBLEND);
			fog = GetRenderState(FOGENABLE);
			SetRenderState(DESTBLEND, BLENDONE);
			SetRenderState(FOGENABLE, 0);
			SetRenderState(VERTEXALPHA, 1);
			d3d::flushCache();
			d3ddevice->DrawIndexedPrimitive((D3DPRIMITIVETYPE)header->primType, inst->baseIndex,
			                                0, inst->numVertices,
			                                inst->startIndex, inst->numPrimitives);
			SetRenderState(DESTBLEND, dst);
			SetRenderState(FOGENABLE, fog);

			setVertexShader(buildingPipe_platformSwitch == PLATFORM_PS2 ? ps2BuildingVS : pcBuildingVS);
			setPixelShader(simplePS);
		}

		inst++;
	}
}

#define GETEXTRACOLOREXT(g) PLUGINOFFSET(gta::ExtraVertColors, g, gta::extraVertColorOffset)

static void
buildingInstanceCB(Geometry *geo, d3d9::InstanceDataHeader *header, bool32 reinstance)
{
	int i = 0;
	VertexElement dcl[12];
	VertexStream *s = &header->vertexStream[0];
	gta::ExtraVertColors *extracols = GETEXTRACOLOREXT(geo);
	V3d *extranormals = gta::getExtraNormals(geo);

	bool isPrelit = (geo->flags & Geometry::PRELIT) != 0;
	bool hasNormals = (geo->flags & Geometry::NORMALS) != 0;

	if(!reinstance){
		// Create declarations and buffers only the first time

		assert(s->vertexBuffer == nil);
		s->offset = 0;
		s->managed = 1;
		s->geometryFlags = 0;
		s->dynamicLock = 0;

		dcl[i].stream = 0;
		dcl[i].offset = 0;
		dcl[i].type = D3DDECLTYPE_FLOAT3;
		dcl[i].method = D3DDECLMETHOD_DEFAULT;
		dcl[i].usage = D3DDECLUSAGE_POSITION;
		dcl[i].usageIndex = 0;
		i++;
		uint16 stride = 12;
		s->geometryFlags |= 0x2;

		if(isPrelit){
			// night
			dcl[i].stream = 0;
			dcl[i].offset = stride;
			dcl[i].type = D3DDECLTYPE_D3DCOLOR;
			dcl[i].method = D3DDECLMETHOD_DEFAULT;
			dcl[i].usage = D3DDECLUSAGE_COLOR;
			dcl[i].usageIndex = 0;
			i++;
			s->geometryFlags |= 0x8;
			stride += 4;

			// day
			dcl[i].stream = 0;
			dcl[i].offset = stride;
			dcl[i].type = D3DDECLTYPE_D3DCOLOR;
			dcl[i].method = D3DDECLMETHOD_DEFAULT;
			dcl[i].usage = D3DDECLUSAGE_COLOR;
			dcl[i].usageIndex = 1;
			i++;
			s->geometryFlags |= 0x8;
			stride += 4;
		}else{
			// we need vertex colors in the shader so force white like on PS2, one set is enough
			dcl[i].stream = 0;
			dcl[i].offset = stride;
			dcl[i].type = D3DDECLTYPE_D3DCOLOR;
			dcl[i].method = D3DDECLMETHOD_DEFAULT;
			dcl[i].usage = D3DDECLUSAGE_COLOR;
			dcl[i].usageIndex = 0;
			i++;
			s->geometryFlags |= 0x8;
			stride += 4;
		}

		for(int32 n = 0; n < geo->numTexCoordSets; n++){
			dcl[i].stream = 0;
			dcl[i].offset = stride;
			dcl[i].type = D3DDECLTYPE_FLOAT2;
			dcl[i].method = D3DDECLMETHOD_DEFAULT;
			dcl[i].usage = D3DDECLUSAGE_TEXCOORD;
			dcl[i].usageIndex = (uint8)n;
			i++;
			s->geometryFlags |= 0x10 << n;
			stride += 8;
		}

		if(hasNormals || extranormals){
			dcl[i].stream = 0;
			dcl[i].offset = stride;
			dcl[i].type = D3DDECLTYPE_FLOAT3;
			dcl[i].method = D3DDECLMETHOD_DEFAULT;
			dcl[i].usage = D3DDECLUSAGE_NORMAL;
			dcl[i].usageIndex = 0;
			i++;
			s->geometryFlags |= 0x4;
			stride += 12;
		}
		dcl[i] = D3DDECL_END();
		s->stride = stride;

		assert(header->vertexDeclaration == nil);
		header->vertexDeclaration = createVertexDeclaration((VertexElement*)dcl);

		assert(s->vertexBuffer == nil);
		s->vertexBuffer = createVertexBuffer(header->totalNumVertex*s->stride, 0, false);
	}else
		getDeclaration(header->vertexDeclaration, dcl);

	uint8 *verts = lockVertices(s->vertexBuffer, 0, 0, D3DLOCK_NOSYSLOCK);

	// Instance vertices
	if(!reinstance || geo->lockedSinceInst&Geometry::LOCKVERTICES){
		for(i = 0; dcl[i].usage != D3DDECLUSAGE_POSITION || dcl[i].usageIndex != 0; i++)
			;
		instV3d(vertFormatMap[dcl[i].type], verts + dcl[i].offset,
			geo->morphTargets[0].vertices,
			header->totalNumVertex,
			header->vertexStream[dcl[i].stream].stride);
	}

	// Instance prelight colors
	if(!reinstance || geo->lockedSinceInst&Geometry::LOCKPRELIGHT){
		if(isPrelit){
			int j;
			for(i = 0; dcl[i].usage != D3DDECLUSAGE_COLOR || dcl[i].usageIndex != 0; i++)
				;
			for(j = 0; dcl[j].usage != D3DDECLUSAGE_COLOR || dcl[j].usageIndex != 1; j++)
				;

			InstanceData *inst = header->inst;
			uint32 n = header->numMeshes;
			rw::RGBA *dayColors = geo->colors;//extracols->dayColors;
			rw::RGBA *nightColors = extracols->nightColors;
	//		if(dayColors == nil) dayColors = geo->colors;
			if(nightColors == nil) nightColors = dayColors;
			while(n--){
				uint32 stride = header->vertexStream[dcl[i].stream].stride;
				inst->vertexAlpha = instColor(vertFormatMap[dcl[i].type],
					verts + dcl[i].offset + stride*inst->minVert,
					nightColors + inst->minVert,
					inst->numVertices,
					stride);
				inst->vertexAlpha |= instColor(vertFormatMap[dcl[j].type],
					verts + dcl[j].offset + stride*inst->minVert,
					dayColors + inst->minVert,
					inst->numVertices,
					stride);
				inst++;
			}
		}else{
			for(i = 0; dcl[i].usage != D3DDECLUSAGE_COLOR || dcl[i].usageIndex != 0; i++)
				;
			InstanceData *inst = header->inst;
			uint32 n = header->numMeshes;
			while(n--){
				inst->vertexAlpha = 0;
				inst++;
			}
			instWhite(vertFormatMap[dcl[i].type],
				verts + dcl[i].offset,
				header->totalNumVertex,
				header->vertexStream[dcl[i].stream].stride);
		}
	}

	// Instance tex coords
	for(int32 n = 0; n < geo->numTexCoordSets; n++){
		if(!reinstance || geo->lockedSinceInst&(Geometry::LOCKTEXCOORDS<<n)){
			for(i = 0; dcl[i].usage != D3DDECLUSAGE_TEXCOORD || dcl[i].usageIndex != n; i++)
				;
			instTexCoords(vertFormatMap[dcl[i].type], verts + dcl[i].offset,
				geo->texCoords[n],
				header->totalNumVertex,
				header->vertexStream[dcl[i].stream].stride);
		}
	}

	// Instance normals
	if((hasNormals || extranormals) && (!reinstance || geo->lockedSinceInst&Geometry::LOCKNORMALS)){
		for(i = 0; dcl[i].usage != D3DDECLUSAGE_NORMAL || dcl[i].usageIndex != 0; i++)
			;
		instV3d(vertFormatMap[dcl[i].type], verts + dcl[i].offset,
			hasNormals ? geo->morphTargets[0].normals : extranormals,
			header->totalNumVertex,
			header->vertexStream[dcl[i].stream].stride);
	}
	unlockVertices(s->vertexBuffer);
}


void
makeCustomBuildingPipelines(void)
{
	d3d9::ObjPipeline *pipe;

#include "d3d_shaders/ps2BuildingVS.inc"
#include "d3d_shaders/pcBuildingVS.inc"
#include "d3d_shaders/simplePS.inc"
	ps2BuildingVS = createVertexShader(ps2BuildingVS_cso);
	pcBuildingVS = createVertexShader(pcBuildingVS_cso);
	simplePS = createPixelShader(simplePS_cso);
	assert(ps2BuildingVS);
	assert(pcBuildingVS);
	assert(simplePS);

#include "d3d_shaders/ps2BuildingFxVS.inc"
#include "d3d_shaders/ps2EnvPS.inc"
	ps2BuildingFxVS = createVertexShader(ps2BuildingFxVS_cso);
	ps2EnvPS = createPixelShader(ps2EnvPS_cso);
	assert(ps2BuildingFxVS);
	assert(ps2EnvPS);


	pipe = d3d9::ObjPipeline::create();
	pipe->pluginID = gta::RSPIPE_PC_CustomBuilding_PipeID;
	pipe->pluginData = gta::RSPIPE_PC_CustomBuilding_PipeID;
	pipe->instanceCB = buildingInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = buildingRenderCB_PS2;
	buildingPipe = pipe;

	pipe = d3d9::ObjPipeline::create();
	pipe->pluginID = gta::RSPIPE_PC_CustomBuildingDN_PipeID;
	pipe->pluginData = gta::RSPIPE_PC_CustomBuildingDN_PipeID;
	pipe->instanceCB = buildingInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = buildingRenderCB_PS2;
	buildingDNPipe = pipe;
}
#endif




#ifdef RW_GL3

using namespace gl3;

enum AttribIndices
{
	ATTRIB_NIGHTCOLOR = ATTRIB_COLOR,
	ATTRIB_DAYCOLOR = ATTRIB_WEIGHTS,	// hack
};

static gl3::Shader *ps2BuildingShader, *pcBuildingShader, *ps2BuildingFXShader;
int32 u_dayparam, u_nightparam;	// DN
int32 u_texmat;	// UVA
int32 u_envmat, u_envXform, u_shininess;	// EnvMap
int32 u_colorscale;

#define U(i) currentShader->uniformLocations[i]

#define GETEXTRACOLOREXT(g) PLUGINOFFSET(gta::ExtraVertColors, g, gta::extraVertColorOffset)

static void
buildingInstanceCB(Geometry *geo, gl3::InstanceDataHeader *header, bool32 reinstance)
{
	AttribDesc *attribs, *a, *b;

	gta::ExtraVertColors *extracols = GETEXTRACOLOREXT(geo);
	V3d *extranormals = gta::getExtraNormals(geo);
	bool isPrelit = !!(geo->flags & Geometry::PRELIT);
	bool hasNormals = !!(geo->flags & Geometry::NORMALS);

	if(!reinstance){
		AttribDesc tmpAttribs[12];
		uint32 stride;

		//
		// Create attribute descriptions
		//
		a = tmpAttribs;
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
		if(hasNormals){
			a->index = ATTRIB_NORMAL;
			a->size = 3;
			a->type = GL_FLOAT;
			a->normalized = GL_FALSE;
			a->offset = stride;
			stride += 12;
			a++;
		}

		// Prelighting
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

		header->numAttribs = a - tmpAttribs;
		for(a = tmpAttribs; a != &tmpAttribs[header->numAttribs]; a++)
			a->stride = stride;
		header->attribDesc = rwNewT(AttribDesc, header->numAttribs, MEMDUR_EVENT | ID_GEOMETRY);
		memcpy(header->attribDesc, tmpAttribs,
		       header->numAttribs*sizeof(AttribDesc));

		//
		// Allocate vertex buffer
		//
		header->vertexBuffer = rwNewT(uint8, header->totalNumVertex*stride, MEMDUR_EVENT | ID_GEOMETRY);
		assert(header->vbo == 0);
		glGenBuffers(1, &header->vbo);
	}

	attribs = header->attribDesc;

	//
	// Fill vertex buffer
	//

	uint8 *verts = header->vertexBuffer;

	// Positions
	if(!reinstance || geo->lockedSinceInst&Geometry::LOCKVERTICES){
		for(a = attribs; a->index != ATTRIB_POS; a++)
			;
		instV3d(VERT_FLOAT3, verts + a->offset,
			geo->morphTargets[0].vertices,
			header->totalNumVertex, a->stride);
	}

	// Normals
	if((hasNormals || extranormals) && (!reinstance || geo->lockedSinceInst&Geometry::LOCKNORMALS)){
		for(a = attribs; a->index != ATTRIB_NORMAL; a++)
			;
		instV3d(VERT_FLOAT3, verts + a->offset,
			hasNormals ? geo->morphTargets[0].normals : extranormals,
			header->totalNumVertex, a->stride);
	}

	// Prelighting
	if(!reinstance || geo->lockedSinceInst&Geometry::LOCKPRELIGHT){
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
	}

	// Texture coordinates
	for(int32 n = 0; n < geo->numTexCoordSets; n++){
		if(!reinstance || geo->lockedSinceInst&(Geometry::LOCKTEXCOORDS<<n)){
			for(a = attribs; a->index != ATTRIB_TEXCOORDS0+n; a++)
				;
			instTexCoords(VERT_FLOAT2, verts + a->offset,
				geo->texCoords[n],
				header->totalNumVertex, a->stride);
		}
	}

#ifdef RW_GL_USE_VAOS
	glBindVertexArray(header->vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, header->ibo);
#endif
	glBindBuffer(GL_ARRAY_BUFFER, header->vbo);
	glBufferData(GL_ARRAY_BUFFER, header->totalNumVertex*attribs[0].stride,
	             header->vertexBuffer, GL_STATIC_DRAW);
#ifdef RW_GL_USE_VAOS
	setAttribPointers(header->attribDesc, header->numAttribs);
	glBindVertexArray(0);
#endif
}

static void
buildingRenderCB(Atomic *atomic, gl3::InstanceDataHeader *header)
{
	RawMatrix ident;
	Matrix *texmat;
	Material *m;
	Geometry *geo = atomic->geometry;

	RawMatrix::setIdentity(&ident);
	uint32 flags = geo->flags;
	setWorldMatrix(atomic->getFrame()->getLTM());
	int32 vsBits = lightingCB(atomic);

	setupVertexInput(header);

	InstanceData *inst = header->inst;
	int32 n = header->numMeshes;

	(buildingPipe_platformSwitch == PLATFORM_PS2 ? ps2BuildingShader : pcBuildingShader)->use();

	float dayparam[4], nightparam[4];
	if(atomic->pipeline->pluginData == gta::RSPIPE_PC_CustomBuilding_PipeID){
		dayparam[0] = dayparam[1] = dayparam[2] = dayparam[3] = 0.0f;
		nightparam[0] = nightparam[1] = nightparam[2] = nightparam[3] = 1.0f;
	}else{
		dayparam[0] = dayparam[1] = dayparam[2] = 1.0f-buildingPipe_dayNightBalance;
		nightparam[0] = nightparam[1] = nightparam[2] = buildingPipe_dayNightBalance;
		dayparam[3] = buildingPipe_wetRoad;
		nightparam[3] = (1.0f-buildingPipe_wetRoad);
	}

	glUniform4fv(U(u_dayparam), 1, dayparam);
	glUniform4fv(U(u_nightparam), 1, nightparam);

	RawMatrix envmat;
	GetBuildingEnvMatrix(atomic, nil, &envmat);

	while(n--){
		m = inst->material;

		float colorscale = 1.0f;
		if(m->texture && buildingPipe_platformSwitch == PLATFORM_PS2)
			colorscale = 255.0f/128.0f;
		glUniform1fv(U(u_colorscale), 1, &colorscale);

		setMaterial(flags, m->color, m->surfaceProps);
		setTexture(0, m->texture);

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

		drawInst(header, inst);

		int hasEnv = (*(uint32*)&inst->material->surfaceProps.specular) & 1;
		if(hasEnv){
			gta::EnvMat *env = gta::getEnvMat(inst->material);
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

			drawInst(header, inst);

			SetRenderState(DESTBLEND, dst);
			(buildingPipe_platformSwitch == PLATFORM_PS2 ? ps2BuildingShader : pcBuildingShader)->use();
		}

		inst++;
	}
	teardownVertexInput(header);
}

void
makeCustomBuildingPipelines(void)
{
	gl3::ObjPipeline *pipe;

	u_dayparam = registerUniform("u_dayparam");
	u_nightparam = registerUniform("u_nightparam");
	u_texmat = registerUniform("u_texmat");
	u_envmat = registerUniform("u_envmat");
	u_envXform = registerUniform("u_envXform");
	u_shininess = registerUniform("u_shininess");
	u_colorscale = registerUniform("u_colorscale");

	{
#include "gl_shaders/ps2Building_vert.inc"
#include "gl_shaders/pcBuilding_vert.inc"
#include "gl_shaders/ps2Building_frag.inc"
	const char *vs[] = { shaderDecl, header_vert_src, ps2Building_vert_src, nil };
	const char *vspc[] = { shaderDecl, header_vert_src, pcBuilding_vert_src, nil };
	const char *fs[] = { shaderDecl, header_frag_src, ps2Building_frag_src, nil };
	ps2BuildingShader = Shader::create(vs, fs);
	assert(ps2BuildingShader);
	pcBuildingShader = Shader::create(vspc, fs);
	assert(pcBuildingShader);
	}
	{
#include "gl_shaders/ps2BuildingFX_vert.inc"
#include "gl_shaders/ps2Env_frag.inc"
	const char *vs[] = { shaderDecl, header_vert_src, ps2BuildingFX_vert_src, nil };
	const char *fs[] = { shaderDecl, header_frag_src, ps2Env_frag_src, nil };
	ps2BuildingFXShader = Shader::create(vs, fs);
	assert(ps2BuildingFXShader);
	}

	pipe = gl3::ObjPipeline::create();
	pipe->pluginID = gta::RSPIPE_PC_CustomBuilding_PipeID;
	pipe->pluginData = gta::RSPIPE_PC_CustomBuilding_PipeID;
	pipe->instanceCB = buildingInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = buildingRenderCB;
	buildingPipe = pipe;

	pipe = gl3::ObjPipeline::create();
	pipe->pluginID = gta::RSPIPE_PC_CustomBuildingDN_PipeID;
	pipe->pluginData = gta::RSPIPE_PC_CustomBuildingDN_PipeID;
	pipe->instanceCB = buildingInstanceCB;
	pipe->uninstanceCB = nil;
	pipe->renderCB = buildingRenderCB;
	buildingDNPipe = pipe;
}
#endif


}
