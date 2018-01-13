#include "storiesview.h"

namespace Renderer
{

rw::ObjPipeline *buildingPipe;

int numOpaqueAtomics;
int numTransparentAtomics;
rw::Atomic *opaqueRenderList[0x8000];
rw::Atomic *transparentRenderList[0x8000];

void
reset(void)
{
	numOpaqueAtomics = 0;
	numTransparentAtomics = 0;
}

void
addToOpaqueRenderList(rw::Atomic *a)
{
	opaqueRenderList[numOpaqueAtomics++] = a;
}

void
addToTransparentRenderList(rw::Atomic *a)
{
	// TODO: sort
	transparentRenderList[numTransparentAtomics++] = a;
}

void
renderOpaque(void)
{
	int i;
	for(i = 0; i < numOpaqueAtomics; i++)
		opaqueRenderList[i]->render();
}

void
renderTransparent(void)
{
	int i;
	for(i = 0; i < numTransparentAtomics; i++)
		transparentRenderList[i]->render();
}

void
drawEntityCube(CEntity *e)
{
	rw::Atomic *atomic;
	if(e->rslObject == 0){
		atomic = rw::Atomic::create();
		atomic->setGeometry(cubeGeo, 0);
		rw::Frame *f = rw::Frame::create();
		atomic->setFrame(f);
		f->matrix = *(rw::Matrix*)&e->placeable.matrix.matrix;
		f->matrix.optimize();
		e->rslObject = (int32)atomic;
	}else
		atomic = (rw::Atomic*)e->rslObject;
	atomic->render();
}

void
drawEntityBS(CEntity *e)
{
	rw::Matrix mat;
	CBaseModelInfo *mi = CModelInfo::Get(e->modelIndex);
	if(mi == nil || mi->colModel == nil)
		return;
	if(mi->type != MODELINFO_SIMPLE && mi->type != MODELINFO_TIME)
		return;
	CSimpleModelInfo *smi = (CSimpleModelInfo*)mi;
	CTimeModelInfo *tmi = nil;
	if(mi->type == MODELINFO_TIME)
		tmi = (CTimeModelInfo*)mi;

	bool lod = !!(smi->flags & 0x10);
	if(lod != drawLOD)
		return;
	if(tmi && !GetIsTimeInRange(tmi->timeOn, tmi->timeOff))
		return;

	if(TheCamera.distanceTo(*(rw::V3d*)&e->placeable.matrix.matrix.pos) > smi->drawDistances[0])
		return;

	mat = *(rw::Matrix*)&e->placeable.matrix.matrix;
	mat.optimize();
	RenderColModelWire(mi->colModel, &mat, true);
}

void
renderDebugIPL(void)
{
	CBuilding *b;
	int i, n;

	cubeMat->color.red = 0;
	cubeMat->color.green = 255;
	cubeMat->color.blue = 255;
	pAmbient->setColor(1.0f, 1.0f, 1.0f);

	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		b = pBuildingPool->GetSlot(i);
		if(b == nil)
			continue;
		drawEntityBS(b);
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		b = pTreadablePool->GetSlot(i);
		if(b == nil)
			continue;
		drawEntityBS(b);
	}
}

void
renderPathNodes(void)
{
	int i;
	static rw::Atomic *atomic;
	if(atomic == nil){
		atomic = rw::Atomic::create();
		atomic->setGeometry(cubeGeo, 0);
		rw::Frame *f = rw::Frame::create();
		atomic->setFrame(f);
	}

	cubeMat->color.red = 255;
	cubeMat->color.green = 0;
	cubeMat->color.blue = 0;

	rw::Frame *f = atomic->getFrame();
	for(i = 0; i < gpThePaths->numPathNodes; i++){
		CPathNode *pn = &gpThePaths->pathNodes[i];
//		if(i >= 0 && i < gpThePaths->numCarNodes)
//			continue;

		rw::V3d pos;
		pos.x = pn->x/8.0f;
		pos.y = pn->y/8.0f;
		pos.z = pn->z;
#ifdef LCS
		pos.z /= 8.0f;
#endif
		f->translate(&pos, rw::COMBINEREPLACE);
		atomic->render();
	}
}

}