#include "III.h"

int CGame::currLevel;

void
CGame::InitialiseRW(void)
{
	gta::attachPlugins();
	rw::d3d::registerNativeRaster();
	rw::d3d::isP8supported = 0;
	rw::loadTextures = 0;

	rw::currentTexDictionary = rw::TexDictionary::create();
	rw::Image::setSearchPath("D:\\rockstargames\\ps2\\gta3\\MODELS\\gta3_archive\\txd_extracted\\;");

	rw::platform = rw::PLATFORM_D3D9;
	//rw::d3d::device = Device;

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

void
CGame::Initialise(void)
{
	CGame::currLevel = 1;
	CPathFind::AllocatePathFindInfoMem(PATHNODESIZE);
	CdStream::addImage("models/gta3.img");
	CFileLoader::LoadLevel("data/default.dat");
	CFileLoader::LoadLevel("data/gta3.dat");
	CStreaming::Init();
}
