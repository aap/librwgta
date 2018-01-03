#include "euryopa.h"

int gameversion;

Params params;

int gameTxdSlot;

int currentHour = 12;
int currentMinute = 0;
int oldWeather, newWeather;
float weatherInterpolation;
int extraColours = -1;
int currentArea;

// Options

bool gRenderCollision;
bool gRenderOnlyLod;
bool gRenderOnlyHD;
bool gRenderBackground = true;
bool gRenderWater = true;
bool gEnableFog = true;
bool gUseBlurAmb;
bool gNoTimeCull;
bool gNoAreaCull;
bool gDoBackfaceCulling;	// init from params

// SA building pipe
float gDayNightBalance;
float gWetRoadEffect;

bool
IsHourInRange(int h1, int h2)
{
	if(h1 > h2)
		return currentHour >= h1 || currentHour < h2;
	else
		return currentHour >= h1 && currentHour < h2;
}

void
InitParams(void)
{
	static const char *areasVC[] = {
		"Main Map", "Hotel", "Mansion", "Bank", "Mall", "Strip club",
		"Lawyer", "Coffee shop", "Concert hall", "Studio", "Rifle range",
		"Biker bar", "Police station", "Everywhere", "Dirt", "Blood", "Oval ring",
		"Malibu", "Print works"
	};
	static const char *weathersIII[] = {
		"SUNNY", "CLOUDY", "RAINY", "FOGGY"
	};
	static const char *weathersVC[] = {
		"SUNNY", "CLOUDY", "RAINY", "FOGGY", "EXTRASUNNY", "RAINY", "EXTRACOLOURS"
	};
	static const char *weathersSA[] = {
		"EXTRASUNNY LA", "SUNNY LA", "EXTRASUNNY SMOG LA", "SUNNY SMOG LA",
		"CLOUDY LA", "SUNNY SF", "EXTRASUNNY SF", "CLOUDY SF", "RAINY SF", "FOGGY SF",
		"SUNNY VEGAS", "EXTRASUNNY VEGAS", "CLOUDY VEGAS", "EXTRASUNNY COUNTRYSIDE",
		"SUNNY COUNTRYSIDE", "CLOUDY COUNTRYSIDE", "RAINY COUNTRYSIDE", "EXTRASUNNY DESERT",
		"SUNNY DESERT", "SANDSTORM DESERT", "UNDERWATER", "EXTRACOLOURS 1", "EXTRACOLOURS 2"
	};

	params.initcampos.set(1356.0f, -1107.0f, 96.0f);
	params.initcamtarg.set(1276.0f, -984.0f, 68.0f);
	params.backfaceCull = true;

	switch(gameversion){
	case GAME_III:
		params.initcampos.set(970.8f, -497.3f, 36.8f);
		params.initcamtarg.set(1092.5f, -417.3f, 3.8f);
		params.objFlagset = GAME_III;
		params.timecycle = GAME_III;
		params.numHours = 24;
		params.numWeathers = 4;
		params.weatherNames = weathersIII;
		params.water = GAME_III;
		params.waterTex = "water_old";
		params.waterStart.set(-2048.0f, -2048.0f);
		params.waterEnd.set(2048.0f, 2048.0f);
		params.backfaceCull = false;
		break;
	case GAME_VC:
		params.initcampos.set(131.5f, -1674.2f, 59.8f);
		params.initcamtarg.set(67.9f, -1542.0f, 26.3f);
		params.objFlagset = GAME_VC;
		params.numAreas = 19;
		params.areaNames = areasVC;
		params.timecycle = GAME_VC;
		params.numHours = 24;
		params.numWeathers = 7;
		params.extraColours = 6;
		params.numExtraColours = 1;
		params.weatherNames = weathersVC;
		params.water = GAME_VC;
		params.waterTex = "waterclear256";
		params.waterStart.set(-2048.0f - 400.0f, -2048.0f);
		params.waterEnd.set(2048.0f - 400.0f, 2048.0f);
		break;
	case GAME_SA:
		params.initcampos.set(1789.0f, -1667.4f, 66.4f);
		params.initcamtarg.set(1679.1f, -1569.4f, 41.5f);
		params.objFlagset = GAME_SA;
		params.numAreas = 19;
		params.areaNames = areasVC;
		params.timecycle = GAME_SA;
		params.numHours = 8;
		params.numWeathers = 23;
		params.extraColours = 21;
		params.numExtraColours = 2;
		params.weatherNames = weathersSA;
		params.background = GAME_SA;
		params.daynightPipe = true;
		params.water = GAME_SA;
		params.waterTex = "waterclear256";
		break;
	// more configs in the future (LCSPC, VCSPC, UG, ...)
	}
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
	// select
	if(CPad::IsMButtonClicked(1)){
		static rw::RGBA black = { 0, 0, 0, 0xFF };
		TheCamera.m_rwcam->clear(&black, rw::Camera::CLEARIMAGE|rw::Camera::CLEARZ);
		rw::SetRenderState(rw::FOGENABLE, 0);

		renderColourCoded = 1;
		RenderEverything();
		renderColourCoded = 0;
		int32 c = GetColourCode(CPad::newMouseState.x, CPad::newMouseState.y);
		ObjectInst *inst = GetInstanceByID(c);
		if(inst){
			if(CPad::IsShiftDown())
				inst->Select();
			else if(CPad::IsAltDown())
				inst->Deselect();
			else if(CPad::IsCtrlDown()){
				if(inst->m_selected) inst->Deselect();
				else inst->Select();
			}else{
				ClearSelection();
				inst->Select();
			}
		}else
			ClearSelection();
	}
}

void
LoadGame(void)
{
//	SetCurrentDirectory("C:/Users/aap/games/gta3");
//	SetCurrentDirectory("C:/Users/aap/games/gtavc");
//	SetCurrentDirectory("C:/Users/aap/games/gtasa");
//	SetCurrentDirectory("F://gtasa");
//	SetCurrentDirectory("H://");
//	SetCurrentDirectory("C:\\Users\\aap\\games\\gta3d_latest");

	FindVersion();
	switch(gameversion){
	case GAME_III: debug("found III!\n"); break;
	case GAME_VC: debug("found VC!\n"); break;
	case GAME_SA: debug("found SA!\n"); break;
	default: panic("unknown game");
	}
	InitParams();

	TheCamera.m_position = params.initcampos;
	TheCamera.m_target = params.initcamtarg;
	gDoBackfaceCulling = params.backfaceCull;

	defaultTxd = rw::TexDictionary::getCurrent();

	int particleTxdSlot = AddTxdSlot("particle");
	LoadTxd(particleTxdSlot, "MODELS/PARTICLE.TXD");

	gameTxdSlot = AddTxdSlot("generic");
	CreateTxd(gameTxdSlot);
	TxdMakeCurrent(gameTxdSlot);

	Timecycle::Initialize();
	WaterLevel::Initialise();

	AddColSlot("generic");
	AddIplSlot("generic");

	AddCdImage("MODELS\\GTA3.IMG");

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
dogizmo(void)
{
	rw::Camera *cam;
	rw::Matrix tmp, view;
	rw::RawMatrix gizview;
	float *fview, *fproj, *fobj;
	static rw::RawMatrix gizobj;
	static bool first = true;
	if(first){
		tmp.setIdentity();
		convMatrix(&gizobj, &tmp);
		gizobj.pos = TheCamera.m_target;
		first = false;
	}

	cam = (rw::Camera*)rw::engine->currentCamera;
	rw::Matrix::invert(&view, cam->getFrame()->getLTM());
	rw::convMatrix(&gizview, &view);
	fview = (float*)&cam->devView;
	fproj = (float*)&cam->devProj;
	fobj = (float*)&gizobj;

	ImGuiIO &io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(fview, fproj, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, fobj, nil, nil);
//	ImGuizmo::DrawCube(fview, fproj, fobj);
//	ImGuizmo::DrawCube((float*)&gizview, (float*)&cam->devProj, (float*)&gizobj);
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

	ImGui_ImplRW_NewFrame(timeDelta);
	ImGuizmo::BeginFrame();

	Timecycle::Update();
	Timecycle::SetLights();

	UpdateDayNightBalance();

	TheCamera.m_rwcam->setFarPlane(Timecycle::currentColours.farClp);
	TheCamera.m_rwcam->fogPlane = Timecycle::currentColours.fogSt;

	CPad::UpdatePads();
	TheCamera.Process();
	TheCamera.update();
	TheCamera.m_rwcam->beginUpdate();

	DefinedState();

	LoadAllRequestedObjects();
	BuildRenderList();

	gui(timeDelta);
//	dogizmo();

	handleTool();

	TheCamera.m_rwcam->clear(&clearcol, rw::Camera::CLEARIMAGE|rw::Camera::CLEARZ);
	if(gRenderBackground){
		SetRenderState(rw::CULLMODE, rw::CULLNONE);
		rw::RGBA skytop, skybot;
		rw::convColor(&skytop, &Timecycle::currentColours.skyTop);
		rw::convColor(&skybot, &Timecycle::currentColours.skyBottom);
		if(params.background == GAME_SA)
			Clouds::RenderSkyPolys();
		else{
			Clouds::RenderBackground(skytop.red, skytop.green, skytop.blue,
				skybot.red, skybot.green, skybot.blue, 255);
			Clouds::RenderHorizon();
		}
	}

	rw::SetRenderState(rw::FOGENABLE, gEnableFog);
	RenderOpaque();
	if(gRenderWater)
		WaterLevel::Render();
	RenderTransparent();

	DefinedState();
	rw::SetRenderState(rw::FOGENABLE, 0);

	SetRenderState(rw::CULLMODE, rw::CULLNONE);

	TheCamera.DrawTarget();
	if(gRenderCollision)
		RenderEverythingCollisions();

	RenderDebugLines();

	ImGui::EndFrame();
	ImGui::Render();

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
