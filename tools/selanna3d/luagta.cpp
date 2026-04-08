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



	// Collision

	gtatab.set_function("Sphere", [](const rw::V3d &center, float radius) {
		gta::CSphere sph;
		sph.center = center;
		sph.radius = radius;
		return sph;
	});
	lua.new_usertype<gta::CSphere>("Sphere",
		sol::no_constructor,
		"center", &gta::CSphere::center,
		"radius", &gta::CSphere::radius
	);

	gtatab.set_function("Box", [](const rw::V3d &min, const rw::V3d &max) {
		gta::CBox box;
		box.min = min;
		box.max = max;
		return box;
	});
	lua.new_usertype<gta::CBox>("Box",
		sol::no_constructor,
		"min", &gta::CBox::min,
		"max", &gta::CBox::max
	);

	gtatab.set_function("ColSphere", [](const rw::V3d &center, float radius,
				rw::uint8 surf, rw::uint8 piece, rw::uint8 lighting) {
		gta::CColSphere sph;
		sph.Set(radius, &center, surf, piece, lighting);
		return sph;
	});
	lua.new_usertype<gta::CColSphere>("ColSphere",
		sol::no_constructor,
		"sph", &gta::CColSphere::sph,
		"surface", &gta::CColSphere::surface,
		"piece", &gta::CColSphere::piece,
		"lighting", &gta::CColSphere::lighting
	);

	gtatab.set_function("ColBox", [](const rw::V3d &min, const rw::V3d &max,
				rw::uint8 surf, rw::uint8 piece, rw::uint8 lighting) {
		gta::CColBox box;
		box.Set(&min, &max, surf, piece, lighting);
		return box;
	});
	lua.new_usertype<gta::CColBox>("ColBox",
		sol::no_constructor,
		"box", &gta::CColBox::box,
		"surface", &gta::CColBox::surface,
		"piece", &gta::CColBox::piece,
		"lighting", &gta::CColBox::lighting
	);

	gtatab.set_function("ColLine", [](const rw::V3d &p0, const rw::V3d &p1) {
		gta::CColLine line;
		line.Set(&p0, &p1);
		return line;
	});
	lua.new_usertype<gta::CColLine>("ColLine",
		sol::no_constructor,
		"p0", &gta::CColLine::p0,
		"p1", &gta::CColLine::p1
	);

	gtatab.set_function("ColTriangle", [](int a, int b, int c,
				rw::uint8 surf, rw::uint8 lighting) {
		gta::CColTriangle tri;
		tri.Set(a, b, c, surf, lighting);
		return tri;
	});
	lua.new_usertype<gta::CColTriangle>("ColTriangle",
		sol::no_constructor,
		"a", &gta::CColTriangle::a,
		"b", &gta::CColTriangle::b,
		"c", &gta::CColTriangle::c,
		"surface", &gta::CColTriangle::surface,
		"lighting", &gta::CColTriangle::lighting
	);

	// TODO: dtor?
	gtatab.set_function("ColModel", []() {
		return new gta::CColModel;
	});
	lua.new_usertype<gta::CColModel>("ColModel",
		sol::no_constructor,
		"boundingSphere", &gta::CColModel::boundingSphere,
		"boundingBox", &gta::CColModel::boundingBox,
		"numSphere", &gta::CColModel::numSpheres,
		"numLines", &gta::CColModel::numLines,
		"numBoxes", &gta::CColModel::numBoxes,
		"numTriangles", &gta::CColModel::numTriangles,
		"level", &gta::CColModel::level,
		"flags", &gta::CColModel::flags,
		"allocFlag", &gta::CColModel::allocFlag,

		"getSphere", [](gta::CColModel *c, int i) -> gta::CColSphere* {
			if(i < 0 || i >= c->numSpheres)
				return nil;
			return &c->spheres[i];
		},
		"getLine", [](gta::CColModel *c, int i) -> gta::CColLine* {
			if(i < 0 || i >= c->numLines)
				return nil;
			return &c->lines[i];
		},
		"getBox", [](gta::CColModel *c, int i) -> gta::CColBox* {
			if(i < 0 || i >= c->numBoxes)
				return nil;
			return &c->boxes[i];
		},
		"getVertex", [](gta::CColModel *c, int i) -> rw::V3d* {
			// unfortunately no bounds checking
			// TODO: compressed?
			return &c->vertices[i];
		},
		"getTriangle", [](gta::CColModel *c, int i) -> gta::CColTriangle* {
			if(i < 0 || i >= c->numTriangles)
				return nil;
			return &c->triangles[i];
		},

		"read", [](gta::CColModel *c, const char *data, int len) {
			gta::readColModel(c, (rw::uint8*)data, len);
		},
		"readV2", [](gta::CColModel *c, const char *data, int len) {
			gta::readColModelVer2(c, (rw::uint8*)data, len);
		},
		"readV3", [](gta::CColModel *c, const char *data, int len) {
			gta::readColModelVer3(c, (rw::uint8*)data, len);
		},
		"readV4", [](gta::CColModel *c, const char *data, int len) {
			gta::readColModelVer4(c, (rw::uint8*)data, len);
		}
	);
}
