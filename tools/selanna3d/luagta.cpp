#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include <rw.h>
#include <rwgta.h>
#include "sol/sol.hpp"
#include "stuff.h"

using namespace std;
using namespace rw;

void
SetupAtomicPipelines(rw::Atomic *atm)
{
	// SA
	if(gta::isBuildingPipeAttached(atm))
		gta::setupBuildingPipe(atm);
}

void
registerGTA(sol::state &lua)
{
	sol::table gtatab = lua["gta"].get_or_create<sol::table>();

	gtatab.set_function("SetColourCode", [](rw::uint32 code){
		gta::colourCode.red = code & 0xFF;
		gta::colourCode.green = code>>8 & 0xFF;
		gta::colourCode.blue = code>>16 & 0xFF;
		gta::colourCode.alpha = 255;
	});
	gtatab.set_function("GetColourCode", [](int x, int y){
		return gta::getColourCode(x, y);
	});
	gtatab.set_function("SetRenderColourCoded", [](int colour){
		gta::renderColourCoded = colour;
	});
	gtatab.set_function("SetHighlightColour", [](rw::RGBA col){
extern rw::RGBA highlightColor;
		highlightColor = col;
	});

	gtatab.set_function("SetDayNightBalance", [](float f){ gta::buildingPipe_dayNightBalance = f; });
	gtatab.set_function("SetWetRoads", [](float f){ gta::buildingPipe_wetRoad = f; });

void TxdSetParent(rw::TexDictionary *child, rw::TexDictionary *parent);
	gtatab.set_function("TxdSetParent", [](rw::TexDictionary *child, rw::TexDictionary *parent){
			TxdSetParent(child, parent);
	});

	gtatab.set_function("SetupAtomicPipelines", &SetupAtomicPipelines);
}
