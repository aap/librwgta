#include "III.h"

int CGame::currLevel;

void
CGame::InitialiseRW(void)
{
	rw::Engine::init();
	gta::attachPlugins();
	rw::Driver::open();
#ifdef RW_GL3
	rw::platform = rw::PLATFORM_GL3;
	rw::gl3::initializeRender();
#elif RW_D3D9
	rw::platform = rw::PLATFORM_D3D9;
#endif

	rw::d3d::isP8supported = 0;
	rw::loadTextures = 0;

	rw::TexDictionary::setCurrent(rw::TexDictionary::create());
	rw::Image::setSearchPath("D:\\rockstargames\\ps2\\gta3\\MODELS\\gta3_archive\\txd_extracted\\;");

	CTxdStore::Initialize();
}

void
CGame::InitialiseAfterRW(void)
{
	CAnimManager::LoadAnimGroups();	// not in III
	CHandlingData::Initialise();
	CPedStats::Initialise();
	CTimeCycle::Initialise();
}

int gameTxdSlot;

void
CGame::Initialise(void)
{
	CGame::currLevel = 1;

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
	CdStream::addImage("MODELS\\GTA3.IMG");
	CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
	CFileLoader::LoadLevel("DATA\\GTA3.DAT");

	printf("--Setup Streaming\n");
	CStreaming::Init();
}
