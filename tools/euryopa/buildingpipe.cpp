#include "euryopa.h"

using namespace rw;

rw::ObjPipeline *buildingPipe;
rw::ObjPipeline *buildingDNPipe;


void
UpdateDayNightBalance(void)
{
	float minute = currentHour*60.0f + currentMinute;
	const float morningStart = 6 * 60.0f;
	const float morningEnd = 7 * 60.0f;
	const float eveningStart = 20 * 60.0f;
	const float eveningEnd = 21 * 60.0f;

	// 1.0 is night, 0.0 is day
	if(minute < morningStart)
		gDayNightBalance = 1.0f;
	else if(minute < morningEnd)
		gDayNightBalance = (morningEnd - minute) / (morningEnd - morningStart);
	else if(minute < eveningStart)
		gDayNightBalance = 0.0f;
	else if(minute < eveningEnd)
		gDayNightBalance = 1.0f - (eveningEnd - minute) / (eveningEnd - eveningStart);
	else
		gDayNightBalance = 1.0f;
}

bool
IsBuildingPipeAttached(rw::Atomic *atm)
{
	uint32 id = gta::getPipelineID(atm);
	// PC logic:
	if(id == gta::RSPIPE_PC_CustomBuilding_PipeID || id == gta::RSPIPE_PC_CustomBuildingDN_PipeID)
		return true;
	if(gta::getExtraVertColors(atm) && atm->geometry->colors)
		return true;

	// Xbox logic:
	if(id == gta::RSPIPE_XBOX_CustomBuilding_PipeID || id == gta::RSPIPE_XBOX_CustomBuildingDN_PipeID ||
	   id == gta::RSPIPE_XBOX_CustomBuildingEnvMap_PipeID || id == gta::RSPIPE_XBOX_CustomBuildingDNEnvMap_PipeID)
		return true;

	return false;
}

void
SetupBuildingPipe(rw::Atomic *atm)
{
	// pc: if two sets -> DN, else -> regular
	// xbox: fall back to non -DN if no normal flags

	// Just only do the PC thing for now
	if(gta::getExtraVertColors(atm) && atm->geometry->colors)
		atm->pipeline = buildingDNPipe;
	else
		atm->pipeline = buildingPipe;
}

bool32
instWhite(int type, uint8 *dst, uint32 numVertices, uint32 stride)
{
	if(type == VERT_ARGB || type == VERT_RGBA){
		for(uint32 i = 0; i < numVertices; i++){
			dst[0] = 255;
			dst[1] = 255;
			dst[2] = 255;
			dst[3] = 255;
			dst += stride;
		}
	}else
		assert(0 && "unsupported color type");
	return 0;
}
