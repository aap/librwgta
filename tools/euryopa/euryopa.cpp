#include "euryopa.h"

int gameversion;

int gameTxdSlot;

int currentHour = 12;
int currentWeather;
int currentArea;

// Options

bool gRenderCollision;

bool
IsHourInRange(int h1, int h2)
{
	if(h1 > h2)
		return currentHour >= h1 || currentHour < h2;
	else
		return currentHour >= h1 && currentHour < h2;
}

void
FindVersion(void)
{
	FILE *f;

	if(f = fopen_ci("data/gta3.dat", "r"), f)
		gameversion = GAME_III;
	else if(f = fopen_ci("data/gta_vc.dat", "r"), f)
		gameversion = GAME_VC;
	else if(f = fopen_ci("data/gta.dat", "r"), f)
		gameversion = GAME_SA;
	else{
		gameversion = GAME_NA;
		return;
	}
	fclose(f);
}

void
test(void)
{
	CPtrNode *p;
	ObjectInst *inst, *inst2;
	int i;
	for(p = instances.first; p; p = p->next){
		inst = (ObjectInst*)p->item;

//		if(inst->m_numChildren > 1)
//			printf("%s has %d lod children\n", GetObjectDef(inst->m_objectId)->m_name, inst->m_numChildren);
		i = 0;
		for(inst2 = inst; inst2; inst2 = inst2->m_lod)
			i++;
		if(i > 2){
			printf("%s has %d lod levels\n", GetObjectDef(inst->m_objectId)->m_name, i);
			for(inst2 = inst; inst2; inst2 = inst2->m_lod)
				printf(" %s\n", GetObjectDef(inst2->m_objectId)->m_name);
		}
	}
}

void
handleTool(void)
{
	if(CPad::IsMButtonClicked(1)){
		static rw::RGBA black = { 0, 0, 0, 0xFF };
		TheCamera.m_rwcam->clear(&black, rw::Camera::CLEARIMAGE|rw::Camera::CLEARZ);
		renderColourCoded = 1;
		RenderEverything();
		renderColourCoded = 0;
		int32 c = GetColourCode(CPad::newMouseState.x, CPad::newMouseState.y);
		ObjectInst *inst = GetInstanceByID(c);
		if(inst){
			ObjectDef *obj = GetObjectDef(inst->m_objectId);
			printf("inst: %s\n", obj->m_name);
		}
	}
}

void
LoadGame(void)
{
	SetCurrentDirectory("C:/Users/aap/games/gta3");
//	SetCurrentDirectory("C:/Users/aap/games/gtavc");
//	SetCurrentDirectory("C:/Users/aap/games/gtasa");
//	SetCurrentDirectory("F://gtasa");
//	SetCurrentDirectory("C:\\Users\\aap\\games\\gta3d_latest");

	FindVersion();
	switch(gameversion){
	case GAME_III: debug("found III!\n"); break;
	case GAME_VC: debug("found VC!\n"); break;
	case GAME_SA: debug("found SA!\n"); break;
	default: panic("unknown game");
	}

	defaultTxd = rw::TexDictionary::getCurrent();

	gameTxdSlot = AddTxdSlot("generic");
	CreateTxd(gameTxdSlot);
	TxdMakeCurrent(gameTxdSlot);

	AddColSlot("generic");
	AddIplSlot("generic");

	AddCdImage("models/gta3.img");

	FileLoader::LoadLevel("data/default.dat");
	switch(gameversion){
	case GAME_III: FileLoader::LoadLevel("data/gta3.dat"); break;
	case GAME_VC: FileLoader::LoadLevel("data/gta_vc.dat"); break;
	case GAME_SA: FileLoader::LoadLevel("data/gta.dat"); break;
	}

	InitSectors();

	CPtrNode *p;
	ObjectInst *inst;
	for(p = instances.first; p; p = p->next){
		inst = (ObjectInst*)p->item;
		InsertInstIntoSectors(inst);
	}

	// hide the islands
	ObjectDef *obj;
	if(isIII()){
		obj = GetObjectDef("IslandLODInd", nil);
		if(obj) obj->m_isHidden = true;
		obj = GetObjectDef("IslandLODcomIND", nil);
		if(obj) obj->m_isHidden = true;
		obj = GetObjectDef("IslandLODcomSUB", nil);
		if(obj) obj->m_isHidden = true;
		obj = GetObjectDef("IslandLODsubIND", nil);
		if(obj) obj->m_isHidden = true;
		obj = GetObjectDef("IslandLODsubCOM", nil);
		if(obj) obj->m_isHidden = true;
	}else if(isVC()){
		obj = GetObjectDef("IslandLODmainland", nil);
		if(obj) obj->m_isHidden = true;
		obj = GetObjectDef("IslandLODbeach", nil);
		if(obj) obj->m_isHidden = true;
	}

	//test();

	if(0){
		int i;
		CPtrNode *p;
		i = 0;
		for(p = instances.first; p; p = p->next)
			i++;
		log("%d instances\n", i);
	}
}

void
Draw(float timeDelta)
{
	static rw::RGBA clearcol = { 0x80, 0x80, 0x80, 0xFF };

	CPad *pad = CPad::GetPad(0);
	if(/*CPad::IsKeyDown('Q') || CPad::IsKeyDown(KEY_ESC) ||*/
	   pad->NewState.start && pad->NewState.select){
		sk::globals.quit = 1;
		return;
	}

	CPad::UpdatePads();
	TheCamera.Process();

	TheCamera.update();
	TheCamera.m_rwcam->beginUpdate();

	LoadAllRequestedObjects();

	BuildRenderList();

	DefinedState();

	handleTool();

	TheCamera.m_rwcam->clear(&clearcol, rw::Camera::CLEARIMAGE|rw::Camera::CLEARZ);
	RenderEverything();

	DefinedState();
	rw::SetRenderState(rw::FOGENABLE, 0);

	if(gRenderCollision)
		RenderEverythingCollisions();

	RenderDebugLines();

	// ImGUI
	gui(timeDelta);

	TheCamera.m_rwcam->endUpdate();
	TheCamera.m_rwcam->showRaster();
}

void
Idle(float timeDelta)
{
	static int state = 0;
	switch(state){
	case 0:
		LoadGame();
		state = 1;
		break;
	case 1:
		Draw(timeDelta);
		break;
	}
}
