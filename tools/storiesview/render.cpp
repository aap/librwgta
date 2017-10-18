#include "storiesview.h"

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
