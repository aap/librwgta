#include "storiesview.h"

namespace Renderer
{

rw::ObjPipeline *buildingPipe;

rw::ObjPipeline *colourCodePipe;
rw::RGBA colourCode;
rw::RGBA highlightColor;
bool renderColourCoded;


struct InstAtm
{
	sGeomInstance *inst;
	rw::Atomic *atm;
};

int numOpaqueAtomics;
int numTransparentAtomics;
InstAtm opaqueRenderList[0x8000];
InstAtm transparentRenderList[0x8000];

void
reset(void)
{
	numOpaqueAtomics = 0;
	numTransparentAtomics = 0;
}

void
addToOpaqueRenderList(sGeomInstance *inst, rw::Atomic *a)
{
	opaqueRenderList[numOpaqueAtomics].inst = inst;
	opaqueRenderList[numOpaqueAtomics++].atm = a;
}

void
addToTransparentRenderList(sGeomInstance *inst, rw::Atomic *a)
{
	// TODO: sort
	transparentRenderList[numTransparentAtomics].inst = inst;
	transparentRenderList[numTransparentAtomics++].atm = a;
}

void
myRenderCB(rw::Atomic *atomic)
{
	if(renderColourCoded)
		colourCodePipe->render(atomic);
	else if(highlightColor.red || highlightColor.green || highlightColor.blue){
		atomic->getPipeline()->render(atomic);
		colourCode = highlightColor;
		colourCode.alpha = 128;
		int32 zwrite, fog, aref;
		zwrite = GetRenderState(rw::ZWRITEENABLE);
		fog = rw::GetRenderState(rw::FOGENABLE);
		aref = rw::GetRenderState(rw::ALPHATESTREF);
		SetRenderState(rw::ZWRITEENABLE, 0);
		SetRenderState(rw::FOGENABLE, 0);
		SetRenderState(rw::ALPHATESTREF, 10);
		colourCodePipe->render(atomic);
		SetRenderState(rw::ZWRITEENABLE, zwrite);
		SetRenderState(rw::FOGENABLE, fog);
		SetRenderState(rw::ALPHATESTREF, aref);
	}else
		atomic->getPipeline()->render(atomic);
}

void
setColourCode(int n)
{
	colourCode.red = n & 0xFF;
	colourCode.green = n>>8 & 0xFF;
	colourCode.blue = n>>16 & 0xFF;
	colourCode.alpha = 255;
}

void
RenderInst(sGeomInstance *inst, rw::Atomic *a)
{
	static rw::RGBA black = { 0, 0, 0, 255 };
	static rw::RGBA red = { 255, 0, 0, 255 };
	static rw::RGBA green = { 0, 255, 0, 255 };
	static rw::RGBA blue = { 0, 0, 255, 255 };
	static rw::RGBA highlightCols[] = { black, green, red, blue };

	int id = inst->GetId();
	BuildingExt *be = GetBuildingExt(id);
	setColourCode(id);

	if(be->selected && be->highlight < HIGHLIGHT_SELECTION)
		be->highlight = HIGHLIGHT_SELECTION;
	highlightColor = highlightCols[be->highlight];

	a->render();
	highlightColor = black;
}

void
renderOpaque(void)
{
	int i;
	for(i = 0; i < numOpaqueAtomics; i++)
		RenderInst(opaqueRenderList[i].inst, opaqueRenderList[i].atm);
}

void
renderTransparent(void)
{
	int i;
	for(i = 0; i < numTransparentAtomics; i++)
		RenderInst(transparentRenderList[i].inst, transparentRenderList[i].atm);
}

void
renderEverythingColourCoded(void)
{
	rw::SetRenderState(rw::FOGENABLE, 0);
	SetRenderState(rw::ALPHATESTREF, 10);
	renderColourCoded = 1;
	renderOpaque();
	renderTransparent();
	renderColourCoded = 0;
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
drawEntityCol(CEntity *e)
{
	rw::Matrix mat;
	CBaseModelInfo *mi = CModelInfo::Get(e->modelIndex);
	EntityExt *ee = (EntityExt*)e->vtable;

	if(drawUnmatched)
		if(ee->n == 1)
			return;
	if(!drawCol && ee->highlight == 0 && !ee->selected)
		return;

//	assert(e->area == 0);
	assert(mi);
	assert(mi->colModel);
	if(mi == nil || mi->colModel == nil)
		return;
	if(mi->type != MODELINFO_SIMPLE && mi->type != MODELINFO_TIME)
		return;
	if(drawUnnamed && mi->field0)
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

	rw::V3d pos = *(rw::V3d*)&e->placeable.matrix.matrix.pos;
	if(!ee->highlight && !ee->selected && pos.z < 100.0f && TheCamera.distanceTo(pos) > smi->drawDistances[0])
		return;

	ee->highlight = 0;

	mat = *(rw::Matrix*)&e->placeable.matrix.matrix;
	mat.optimize();
	if(drawWorld && !ee->selected)
		RenderColModelWire(mi->colModel, &mat, false);
	else{
		rw::RGBA col;
		if(renderColourCoded)
			col = colourCode;
		else{
			if(ee->selected)
				RenderColModelWire(mi->colModel, &mat, false);
			else
				RenderColModelWire(mi->colModel, &mat, true);

			srand((int)mi->colModel);
			int tmp = rand();
			col = *(rw::RGBA*)&tmp;
			col.alpha = 255;
		}
		RenderColMeshSolid(mi->colModel, &mat, col);
		if(drawBounds)
			RenderColBoxSolid(mi->colModel, &mat, col);
	}
}

void
renderColModels(void)
{
	CEntity *e;
	int i, n;

	cubeMat->color.red = 0;
	cubeMat->color.green = 255;
	cubeMat->color.blue = 255;
	pAmbient->setColor(1.0f, 1.0f, 1.0f);

	n = pBuildingPool->GetSize();
	for(i = 0; i < n; i++){
		e = pBuildingPool->GetSlot(i);
		if(e == nil)
			continue;
		setColourCode(i | 0x10000);
		drawEntityCol(e);
	}
	n = pTreadablePool->GetSize();
	for(i = 0; i < n; i++){
		e = pTreadablePool->GetSlot(i);
		if(e == nil)
			continue;
		setColourCode(i | 0x20000);
		drawEntityCol(e);
	}
	if(drawDummies){
		n = pDummyPool->GetSize();
		for(i = 0; i < n; i++){
			e = pDummyPool->GetSlot(i);
			if(e == nil)
				continue;
			setColourCode(i | 0x30000);
			drawEntityCol(e);
		}
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