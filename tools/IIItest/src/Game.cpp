#include "III.h"

eLevelName CGame::currLevel;

void
CGame::InitialiseRW(void)
{
	rw::Engine::init();
	gta::attachPlugins();
	CVisibilityPlugins::PluginAttach();
	rw::Engine::open();
	rw::Engine::start(&engineStartParams);
	plAttachInput();

	rw::d3d::isP8supported = 0;
	rw::engine->loadTextures = 1;
	//rw::engine->makeDummies = 1;

	rw::Image::setSearchPath("D:\\rockstargames\\ps2\\gta3\\MODELS\\gta3_archive\\txd_extracted\\;");

	CTxdStore::Initialize();
	CVisibilityPlugins::Initialise();
	rwCamera = rw::Camera::create();
	rwCamera->setFrame(rw::Frame::create());
	rwCamera->setFarPlane(4000.0f);
	rwCamera->setNearPlane(0.9f);
	TheCamera.m_rwcam = rwCamera;
	TheCamera.m_aspectRatio = 640.0f/480.0f;
//	TheCamera.m_target.set(0.0f, 0.0f, 0.0f);
//	TheCamera.m_position.set(-100.0f, -100.0f, 50.0f);
	TheCamera.m_target.set(1155.0f, -190.0f, -18.0f);
	TheCamera.m_position.set(1286.0f, -211.0f, 50.0f);

	rwWorld  = rw::World::create();
	rwWorld->addCamera(rwCamera);

	LightsCreate(rwWorld);
}

void
CGame::InitialiseAfterRW(void)
{
	CTimer::Initialise();
	CAnimManager::LoadAnimGroups();	// not in III
	CHandlingData::Initialise();
	CPedStats::Initialise();
	CTimeCycle::Initialise();
}

int gameTxdSlot;

void
CGame::Initialise(void)
{
	CPools::Initialise();

	CGame::currLevel = LEVEL_NONE;	// INDUSTRIAL gives problems :/

	printf("--Loading generic textures\n");
	gameTxdSlot = CTxdStore::AddTxdSlot("generic");
	CTxdStore::Create(gameTxdSlot);
	CTxdStore::AddRef(gameTxdSlot);

	printf("--Loading particles\n");
	int part = CTxdStore::AddTxdSlot("particle");
	CTxdStore::LoadTxd(part, "MODELS/PARTICLE.TXD");
	CTxdStore::AddRef(part);
	CTxdStore::SetCurrentTxd(gameTxdSlot);

	printf("--Setup game variables\n");
	CPathFind::AllocatePathFindInfoMem(PATHNODESIZE);
	CWeather::Init();
	CCullZones::Init();
	CTheZones::Init();
	InitModelIndices();
	CModelInfo::Initialise();
	CdStream::addImage("MODELS\\GTA3.IMG");
	CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
	CFileLoader::LoadLevel("DATA\\GTA3.DAT");
	CTheZones::PostZoneCreation();

	printf("--Setup Streaming\n");
	CStreaming::Init();
	CStreaming::RequestInitialVehicles();
	CStreaming::RequestInitialPeds();
	CStreaming::RequestBigBuildings(LEVEL_NONE);
	CStreaming::LoadAllRequestedModels();

	printf("--Setup game variables\n");
	CClock::Initialise(1000);

	printf("--Load scene\n");
	CCollision::ms_collisionInMemory = CGame::currLevel;
}

void
CGame::Process(void)
{
	CPad::UpdatePads();

	CPad *pad = CPad::GetPad(0);
	if(CPad::IsKeyDown('Q') || CPad::IsKeyDown(KEY_ESC) ||
	   pad->NewState.start && pad->NewState.select){
		isRunning = 0;
		return;
	}
	CStreaming::LoadAllRequestedModels();

	if(!CTimer::m_UserPause && !CTimer::m_CodePause){
		CTheZones::Update();
		CClock::Update();
		CWeather::Update();
		CCollision::Update();
		CTimeCycle::Update();
		TheCamera.Process();
	}
}
