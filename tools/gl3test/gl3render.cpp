#include <cstdio>
#include <cstring>

#include "gldemo.h"

namespace rw {
namespace gl3 {

GLuint vao;
GLuint ubo_scene, ubo_object;
UniformScene uniformScene;
UniformObject uniformObject;

void
beginUpdate(Camera *cam)
{
	Matrix viewmat;
	Matrix::invert(&viewmat, cam->getFrame()->getLTM());
        viewmat.right.x = -viewmat.right.x;
        viewmat.rightw = 0.0;
        viewmat.up.x = -viewmat.up.x;
        viewmat.upw = 0.0;
        viewmat.at.x = -viewmat.at.x;
        viewmat.atw = 0.0;
        viewmat.pos.x = -viewmat.pos.x;
        viewmat.posw = 1.0;

	setProjectionMatrix(cam->projMat);
	setViewMatrix(&viewmat);
}

void
initializeRender(void)
{
        driver[PLATFORM_GL3].beginUpdate = beginUpdate;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	registerBlock("Scene");
	registerBlock("Object");
	registerUniform("u_matColor");
	registerUniform("u_surfaceProps");

	glGenBuffers(1, &ubo_scene);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_scene);
	glBindBufferBase(GL_UNIFORM_BUFFER, gl3::findBlock("Scene"), ubo_scene);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformScene), &uniformScene,
	             GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);


	glGenBuffers(1, &ubo_object);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_object);
	glBindBufferBase(GL_UNIFORM_BUFFER, gl3::findBlock("Object"), ubo_object);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(UniformObject), &uniformObject,
	             GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

}

void
setAttribPointers(InstanceDataHeader *header)
{
	AttribDesc *a;
	for(a = header->attribDesc;
	    a != &header->attribDesc[header->numAttribs];
	    a++){
		glEnableVertexAttribArray(a->index);
		glVertexAttribPointer(a->index, a->size, a->type, a->normalized,
		                      a->stride, (void*)(uint64)a->offset);
	}
}

static bool32 sceneDirty = 1;
static bool32 objectDirty = 1;

void
setWorldMatrix(Matrix *mat)
{
	uniformObject.world = *mat;
	objectDirty = 1;
}

void
setAmbColor(RGBAf *amb)
{
	uniformObject.ambLight = *amb;
	objectDirty = 1;
}

void
setProjectionMatrix(float32 *mat)
{
        memcpy(&uniformScene.proj, mat, 64);
	sceneDirty = 1;
}

void
setViewMatrix(Matrix *mat)
{
        uniformScene.view = *mat;
	sceneDirty = 1;
}

void
flushCache(void)
{
	if(objectDirty){
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_object);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformObject),
				&uniformObject);
		objectDirty = 0;
	}
	if(sceneDirty){
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_scene);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(UniformScene),
				&uniformScene);
		sceneDirty = 0;
	}
}

void
defaultRenderCB(Atomic *atomic, InstanceDataHeader *header)
{
	glBindBuffer(GL_ARRAY_BUFFER, header->vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, header->ibo);
	setAttribPointers(header);

	RGBAf ambLight = (RGBAf){0.3, 0.3, 0.3, 1.0};

	setWorldMatrix(atomic->getFrame()->getLTM());
	setAmbColor(&ambLight);

	InstanceData *inst = header->inst;
	int32 n = header->numMeshes;
	while(n--){
		flushCache();
		glDrawElements(header->primType, inst->numIndex,
		               GL_UNSIGNED_SHORT, (void*)(uintptr)inst->offset);
		inst++;
	}
}


}
}
