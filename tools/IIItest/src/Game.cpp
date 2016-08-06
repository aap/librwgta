#include "III.h"

eLevelName CGame::currLevel;

void
CGame::InitialiseRW(void)
{
	rw::Engine::init();
	gta::attachPlugins();
	CVisibilityPlugins::PluginAttach();
	rw::Driver::open();
#ifdef RW_GL3
	rw::platform = rw::PLATFORM_GL3;
	rw::gl3::initializeRender();
#elif RW_D3D9
	rw::platform = rw::PLATFORM_D3D9;
#endif

	rw::d3d::isP8supported = 0;
	rw::engine->loadTextures = 1;
	rw::engine->makeDummies = 1;

	rw::Image::setSearchPath("D:\\rockstargames\\ps2\\gta3\\MODELS\\gta3_archive\\txd_extracted\\;");

	CTxdStore::Initialize();
	CVisibilityPlugins::Initialise();
	rwCamera = rw::Camera::create();
	rwCamera->setFrame(rw::Frame::create());
//	rwCamera->setFarPlane(2000.0f);
	rwCamera->setFarPlane(4000.0f);
	rwCamera->setNearPlane(0.9f);
	TheCamera.m_rwcam = rwCamera;
	TheCamera.m_aspectRatio = 640.0f/480.0f;
	TheCamera.m_target.set(0.0f, 0.0f, 0.0f);
	TheCamera.m_position.set(-100.0f, -100.0f, 50.0f);

	rwWorld  = rw::World::create();
	rwWorld->addCamera(rwCamera);

	// Ambient light
	ambient = rw::Light::create(rw::Light::AMBIENT);
	ambient->setColor(0.3f, 0.3f, 0.3f);
	rwWorld->addLight(ambient);

	// Diffuse light
	direct = rw::Light::create(rw::Light::DIRECTIONAL);
	rw::Frame *frm = rw::Frame::create();
	direct->setFrame(frm);
	frm->matrix.pointInDirection((rw::V3d){1.0, 0.0, -0.5},
			(rw::V3d){0.0, 0.0, 1.0});
	frm->updateObjects();
	rwWorld->addLight(direct);
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

	CGame::currLevel = LEVEL_INDUSTRIAL;

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

	printf("--Setup game variables\n");
	CClock::Initialise(1000);

	printf("--Load scene\n");
	CCollision::ms_collisionInMemory = CGame::currLevel;
}

void
CGame::Process(void)
{
	CStreaming::LoadAllRequestedModels();
	if(!CTimer::m_UserPause && !CTimer::m_CodePause){
		CTheZones::Update();
		CCollision::Update();
	}
}
