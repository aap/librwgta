#include "storiesview.h"
#include <Xinput.h>

const char *levelNames[] = {
#ifdef LCS
	"INDUST",
	"COMMER",
	"SUBURB",
	"UNDERG",
#else
	"BEACH",
	"MAINLA",
	"MALL",
#endif
	nil
};

int32 atmOffset;	// UNUSED, just for rslconv.cpp

rw::V3d zero = { 0.0f, 0.0f, 0.0f };
struct SceneGlobals {
	rw::World *world;
	rw::Camera *camera;
} Scene;
rw::EngineStartParams engineStartParams;
rw::Material *cubeMat;
rw::Geometry *cubeGeo;
rw::Light *pAmbient;
int drawCubes = 1;
int drawLOD = 1;
int frameCounter = -1;

void
panic(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void
debug(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stdout, fmt, ap);
	fflush(stdout);
	va_end(ap);
}

#define XINPUT
#ifdef XINPUT
int pads[4];
int numPads;
int currentPad;

void
plAttachInput(void)
{
	int i;
	XINPUT_STATE state;

	for(i = 0; i < 4; i++)
		if(XInputGetState(i, &state) == ERROR_SUCCESS)
			pads[numPads++] = i;
}

void
plCapturePad(int arg)
{
	currentPad = arg;
	return;
}

void
plUpdatePad(CControllerState *state)
{
	XINPUT_STATE xstate;
	int pad;

	pad = currentPad < numPads ? pads[currentPad] : -1;
	if(pad < 0 || XInputGetState(pad, &xstate) != ERROR_SUCCESS){
		memset(state, 0, sizeof(CControllerState));
		return;
	}

	state->leftX  = 0;
	state->leftY  = 0;
	state->rightX = 0;
	state->rightY = 0;
	if(xstate.Gamepad.sThumbLX > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || xstate.Gamepad.sThumbLX < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		state->leftX = xstate.Gamepad.sThumbLX;
	if(xstate.Gamepad.sThumbLY > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE || xstate.Gamepad.sThumbLY < -XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
		state->leftY = xstate.Gamepad.sThumbLY;
	if(xstate.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || xstate.Gamepad.sThumbRX < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		state->rightX = xstate.Gamepad.sThumbRX;
	if(xstate.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE || xstate.Gamepad.sThumbRY < -XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
		state->rightY = xstate.Gamepad.sThumbRY;

	state->triangle = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_Y);
	state->circle = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_B);
	state->cross = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_A);
	state->square = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_X);
	state->l1 = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
	state->l2 = xstate.Gamepad.bLeftTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	state->leftshock = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
	state->r1 = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
	state->r2 = xstate.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
	state->rightshock = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
	state->select = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_BACK);
	state->start = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_START);
	state->up = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
	state->right = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
	state->down = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
	state->left = !!(xstate.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);

}
#endif

void
Init(void)
{
	sk::globals.windowtitle = "Stories viewer";
	sk::globals.width = 640;
	sk::globals.height = 480;
	sk::globals.quit = 0;
}

bool
attachPlugins(void)
{
	rw::ps2::registerPDSPlugin(40);
	rw::ps2::registerPluginPDSPipes();

	rw::registerMeshPlugin();
	rw::registerNativeDataPlugin();
	rw::registerAtomicRightsPlugin();
	rw::registerMaterialRightsPlugin();
	rw::xbox::registerVertexFormatPlugin();
	rw::registerSkinPlugin();
	rw::registerUserDataPlugin();
	rw::registerHAnimPlugin();
	rw::registerMatFXPlugin();
	rw::registerUVAnimPlugin();
	rw::ps2::registerADCPlugin();
	return true;
}

void
makeCube(void)
{
	using namespace rw;

	cubeMat = Material::create();

	cubeGeo = Geometry::create(8, 12, Geometry::MODULATE | Geometry::LIGHT);
	cubeGeo->matList.appendMaterial(cubeMat);

	MorphTarget *mt = &cubeGeo->morphTargets[0];
	V3d *verts = mt->vertices;
	Triangle *tri = cubeGeo->triangles;

	float size = 1.0f;

	verts[0].set(-size, -size,  size);
	verts[1].set(-size,  size,  size);
	verts[2].set( size, -size,  size);
	verts[3].set( size,  size,  size);
	verts[4].set(-size, -size, -size);
	verts[5].set(-size,  size, -size);
	verts[6].set( size, -size, -size);
	verts[7].set( size,  size, -size);

	// top
	tri[0].v[0] = 0; tri[0].v[1] = 1; tri[0].v[2] = 2; tri[0].matId = 0;
	tri[1].v[0] = 2; tri[1].v[1] = 1; tri[1].v[2] = 3; tri[1].matId = 0;

	// bottom
	tri[2].v[0] = 4; tri[2].v[1] = 5; tri[2].v[2] = 6; tri[2].matId = 0;
	tri[3].v[0] = 5; tri[3].v[1] = 6; tri[3].v[2] = 7; tri[3].matId = 0;

	// front
	tri[4].v[0] = 0; tri[4].v[1] = 4; tri[4].v[2] = 6; tri[4].matId = 0;
	tri[5].v[0] = 0; tri[5].v[1] = 6; tri[5].v[2] = 2; tri[5].matId = 0;

	// back
	tri[6].v[0] = 1; tri[6].v[1] = 5; tri[6].v[2] = 7; tri[6].matId = 0;
	tri[7].v[0] = 1; tri[7].v[1] = 7; tri[7].v[2] = 3; tri[7].matId = 0;

	// left
	tri[8].v[0] = 1; tri[8].v[1] = 5; tri[8].v[2] = 4; tri[8].matId = 0;
	tri[9].v[0] = 1; tri[9].v[1] = 4; tri[9].v[2] = 0; tri[9].matId = 0;

	// right
	tri[10].v[0] = 3; tri[10].v[1] = 7; tri[10].v[2] = 6; tri[10].matId = 0;
	tri[12].v[0] = 3; tri[11].v[1] = 6; tri[11].v[2] = 2; tri[11].matId = 0;

	cubeGeo->buildMeshes();
	cubeGeo->calculateBoundingSphere();
};

bool
InitRW(void)
{
//	rw::platform = rw::PLATFORM_D3D8;
	if(!sk::InitRW())
		return false;

	Scene.world = rw::World::create();

	pAmbient = rw::Light::create(rw::Light::AMBIENT);
//	pAmbient->setColor(0.2f, 0.2f, 0.2f);
	pAmbient->setColor(1.0f, 1.0f, 1.0f);
	Scene.world->addLight(pAmbient);

	rw::V3d xaxis = { 1.0f, 0.0f, 0.0f };
	rw::Light *direct = rw::Light::create(rw::Light::DIRECTIONAL);
	direct->setColor(0.8f, 0.8f, 0.8f);
	direct->setFrame(rw::Frame::create());
	direct->getFrame()->rotate(&xaxis, 180.0f, rw::COMBINEREPLACE);
	Scene.world->addLight(direct);

	Scene.camera = rw::Camera::create();
	Scene.camera->setFrame(rw::Frame::create());
	Scene.camera->setFarPlane(5000.0f);
	Scene.camera->setNearPlane(0.9f);
	TheCamera.m_rwcam = Scene.camera;
	TheCamera.m_aspectRatio = 640.0f/480.0f;
//	TheCamera.m_target.set(0.0f, 0.0f, 0.0f);
//	TheCamera.m_position.set(-100.0f, -100.0f, 50.0f);
	TheCamera.m_target.set(1155.0f, -190.0f, -18.0f);
	TheCamera.m_position.set(1286.0f, -211.0f, 50.0f);


	Scene.world->addCamera(Scene.camera);

	makeCube();

	return true;
}

int curSectX = 28;
int curSectY = 4;
int curIntr = 0;
int curHour = 12;

bool
GetIsTimeInRange(uint8 h1, uint8 h2)
{
	if(h1 > h2)
		return curHour >= h1 || curHour < h2;
	else
		return curHour >= h1 && curHour < h2;
}


void
InitGame(void)
{
	Zfile *zfile;
	sk::args.argv++;
	sk::args.argc--;
	if(sk::args.argc > 0){
		SetCurrentDirectory(*sk::args.argv);
		sk::args.argv++;
		sk::args.argc--;
	}
	eLevel levelToLoad = (eLevel)1;
	if(sk::args.argc > 0){
		const char *levelname = *sk::args.argv;
		int i;
		const char **names = levelNames;
		for(i = 0; *names; names++, i++){
			if(strcmpi(*names, levelname) == 0){
				levelToLoad = (eLevel)(i+1);
				goto found;
			}
		}
		panic("unknown level");
found:
		sk::args.argv++;
		sk::args.argc--;
	}

#ifdef LCS
	zfile = zopen("CHK/PS2/GAME.DTZ", "rb");
#else
	zfile = zopen("PS2/GAME.DTZ", "rb");
#endif
	if(zfile == nil){
		sk::globals.quit = 1;
		return;
	}

	sChunkHeader header;
	zread(zfile, &header, sizeof(sChunkHeader));
	uint8 *data = (uint8*)malloc(header.fileSize-sizeof(sChunkHeader));
	zread(zfile, data, header.fileSize-sizeof(sChunkHeader));
	zclose(zfile);
	cReloctableChunk(header.ident, header.shrink).Fixup(header, data);

	ResourceImage *resimg = (ResourceImage*)data;
	pBuildingPool = (BuildingPool*)resimg->buildingPool;
	pTreadablePool = (TreadablePool*)resimg->treadablePool;
	pDummyPool = (DummyPool*)resimg->dummyPool;
	pTexStorePool = (TexlistPool*)resimg->texlistPool;
	CModelInfo::Load(resimg->numModelInfos, resimg->modelInfoPtrs);

	LoadLevel(levelToLoad);
	int i;
	for(i = 0; i < gLevel->numSectors; i++)
		LoadSector(i);
#ifdef VCS
	for(i = 0; i < gLevel->chunk->numAreas; i++)
		LoadArea(i);
//		LoadArea(0);
//	exit(0);
#endif
}

void
Draw(void)
{
	static rw::RGBA clearcol = { 0x80, 0x80, 0x80, 0xFF };

	CPad *pad = CPad::GetPad(0);
	if(CPad::IsKeyDown('Q') || CPad::IsKeyDown(KEY_ESC) ||
	   pad->NewState.start && pad->NewState.select){
		sk::globals.quit = 1;
		return;
	}
	if(CPad::IsKeyJustDown('J')){
		curSectY--;
		if(curSectY < 0) curSectY = NUMSECTORSY-1;
	}
	if(CPad::IsKeyJustDown('K')){
		curSectY++;
		if(curSectY >= NUMSECTORSY) curSectY = 0;
	}
	if(CPad::IsKeyJustDown('H')){
		curSectX--;
		if(curSectX < 0) curSectX = NUMSECTORSX-1;
	}
	if(CPad::IsKeyJustDown('L')){
		curSectX++;
		if(curSectX >= NUMSECTORSX) curSectX = 0;
	}
	if(CPad::IsKeyJustDown('I')){
		curIntr++;
		if(curIntr >= gLevel->chunk->numInteriors) curIntr = 0;
	}
	if(CPad::IsKeyJustDown('U')){
		curIntr--;
		if(curIntr < 0) curIntr = gLevel->chunk->numInteriors-1;
	}
	if(CPad::IsKeyJustDown('T')){
		curHour--;
		if(curHour < 0) curHour = 23;
		debug("hour: %d\n", curHour);
	}
	if(CPad::IsKeyJustDown('Y')){
		curHour++;
		if(curHour >= 24) curHour = 0;
		debug("hour: %d\n", curHour);
	}
	if(CPad::IsKeyJustDown('C'))
		drawCubes = !drawCubes;
	if(CPad::IsKeyJustDown('B'))
		drawLOD = !drawLOD;

	CPad::UpdatePads();
	TheCamera.Process();

	TheCamera.m_rwcam->clear(&clearcol, rw::Camera::CLEARIMAGE|rw::Camera::CLEARZ);
	TheCamera.update();
	TheCamera.m_rwcam->beginUpdate();

	if(drawCubes)
		renderCubesIPL();
//	renderCubesSector(curSectX, curSectY);

//	renderSector(worldSectors[curSectX][curSectY]);
	int i;
	for(i = 0; i < gLevel->numWorldSectors; i++)
		renderSector(&gLevel->sectors[i]);
//	renderSector(&gLevel->sectors[gLevel->chunk->interiors[curIntr].sectorId]);

	TheCamera.m_rwcam->endUpdate();
	TheCamera.m_rwcam->showRaster();

	frameCounter++;
}

void
Idle(void)
{
	static int state = 0;
	switch(state){
	case 0:
		InitGame();
//dumpInstances();
//exit(0);
		state = 1;
		break;
	case 1:
		Draw();
		break;
	}
}


sk::EventStatus
AppEventHandler(sk::Event e, void *param)
{
	using namespace sk;
	switch(e){
	case INITIALIZE:
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		Init();
		plAttachInput();
		return EVENTPROCESSED;
	case RWINITIALIZE:
		return ::InitRW() ? EVENTPROCESSED : EVENTERROR;
	case PLUGINATTACH:
		return attachPlugins() ? EVENTPROCESSED : EVENTERROR;
	case KEYDOWN:
		CPad::tempKeystates[*(int*)param] = 1;
		return EVENTPROCESSED;
	case KEYUP:
		CPad::tempKeystates[*(int*)param] = 0;
		return EVENTPROCESSED;
	case IDLE:
		Idle();
		return EVENTPROCESSED;
	}
	return sk::EVENTNOTPROCESSED;
}
