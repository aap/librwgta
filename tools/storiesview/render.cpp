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
drawEntity(CEntity *e)
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
renderCubesIPL(void)
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
		drawEntity(b);
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		b = pTreadablePool->GetSlot(i);
		if(b == nil)
			continue;
		drawEntity(b);
	}
}

}