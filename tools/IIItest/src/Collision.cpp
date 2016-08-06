#include "III.h"

eLevelName CCollision::ms_collisionInMemory;

void
CCollision::Update(void)
{
	CVector pos = FindPlayerCoors();
	eLevelName level = CTheZones::m_CurrLevel;
	bool changeLevel = false;

	// hardcode a level if there are no zones
	if(level == LEVEL_NONE){
		if(CGame::currLevel == LEVEL_INDUSTRIAL &&
		   pos.x < 400.0f){
			level = LEVEL_COMMERCIAL;
			changeLevel = true;
		}else if(CGame::currLevel == LEVEL_SUBURBAN &&
		         pos.x > -450.0f && pos.y < -1400.0f){
			level = LEVEL_COMMERCIAL;
			changeLevel = true;
		}else{
			if(pos.x > 800.0f){
				level = LEVEL_INDUSTRIAL;
				changeLevel = true;
			}else if(pos.x < -800.0f){
				level = LEVEL_SUBURBAN;
				changeLevel = true;
			}
		}
	}
	if(level != LEVEL_NONE && level != CGame::currLevel){
		debug("changing level %d -> %d\n", CGame::currLevel, level);
		CGame::currLevel = level;
	}
	if(ms_collisionInMemory != CGame::currLevel)
		LoadCollisionWhenINeedIt(changeLevel);
	CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
}

void
CCollision::LoadCollisionWhenINeedIt(bool changeLevel)
{
	eLevelName level;
	level = LEVEL_NONE;
	if(!changeLevel){
		//assert(0 && "unimplemented");
	}

	if(level != CGame::currLevel || changeLevel){
		CTimer::Stop();
		CStreaming::RemoveIslandsNotUsed(LEVEL_INDUSTRIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_COMMERCIAL);
		CStreaming::RemoveIslandsNotUsed(LEVEL_SUBURBAN);
		CStreaming::RemoveBigBuildings(LEVEL_INDUSTRIAL);
		CStreaming::RemoveBigBuildings(LEVEL_COMMERCIAL);
		CStreaming::RemoveBigBuildings(LEVEL_SUBURBAN);
		ms_collisionInMemory = CGame::currLevel;
		CStreaming::RemoveUnusedBigBuildings(CGame::currLevel);
		CStreaming::RemoveUnusedBuildings(CGame::currLevel);
		CStreaming::RequestBigBuildings(CGame::currLevel);
		CStreaming::LoadAllRequestedModels();
		CStreaming::HaveAllBigBuildingsLoaded(CGame::currLevel);
		CTimer::Update();
	}
}
