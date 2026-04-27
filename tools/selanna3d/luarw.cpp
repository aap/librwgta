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

char*
getFrameUserName(rw::Frame *f)
{
	using namespace rw;
	int32 i;
	UserDataArray *ar;
	int32 n = UserDataArray::frameGetCount(f);
	for(i = 0; i < n; i++){
		ar = UserDataArray::frameGet(f, i);
		if(strcmp(ar->name, "name") == 0 && ar->datatype == rw::USERDATASTRING)
			return ar->getString(0);
	}
	return (char*)"";
}

char*
getFrameName(Frame *f)
{
	char *name = gta::getNodeName(f);
	if(name[0] == '\0')
		name = getFrameUserName(f);
	return name;
}

void
registerRW(sol::state &lua)
{
	sol::table rwtab = lua["rw"].get_or_create<sol::table>();

	rwtab.set_function("makePath", [](const char *path){
		char *x = strdup(path);
		rw::makePath(x);
		std::string s(x);
		free(x);
		return s;
	});

	rwtab.set_function("V3d", rw::makeV3d);
	lua.new_usertype<rw::V3d>("V3d",
		sol::no_constructor,
		"x", &rw::V3d::x,
		"y", &rw::V3d::y,
		"z", &rw::V3d::z,
		sol::meta_function::to_string, [](const rw::V3d &v) {
			return "V3d(" + std::to_string(v.x) + ", "
			+ std::to_string(v.y) + ", "
			+ std::to_string(v.z) + ")";
		},
		sol::meta_function::addition,       [](const rw::V3d& a, const rw::V3d& b) { return rw::add(a, b); },
		sol::meta_function::subtraction,    [](const rw::V3d& a, const rw::V3d& b) { return rw::sub(a, b); },
		sol::meta_function::multiplication, [](const rw::V3d& a, float s)          { return rw::scale(a, s); }
	);

	rwtab.set_function("Quat", [](float w, float x, float y, float z) {
		return rw::makeQuat(w, x, y, z);
	});
	lua.new_usertype<rw::Quat>("Quat",
		sol::no_constructor,
		"w", &rw::Quat::w,
		"x", &rw::Quat::x,
		"y", &rw::Quat::y,
		"z", &rw::Quat::z,
		"conj", [](const rw::Quat &q) {
			return rw::conj(q);
		},
		sol::meta_function::to_string, [](const rw::Quat &q) {
			return "Quat(" + std::to_string(q.w) + ", "
			+ std::to_string(q.x) + ", "
			+ std::to_string(q.y) + ", "
			+ std::to_string(q.z) + ")";
		},
		sol::meta_function::addition,       [](const rw::Quat& a, const rw::Quat& b) { return rw::add(a, b); },
		sol::meta_function::subtraction,    [](const rw::Quat& a, const rw::Quat& b) { return rw::sub(a, b); },
		sol::meta_function::multiplication, [](const rw::Quat& a, float s)          { return rw::scale(a, s); }
	);

	lua.new_usertype<rw::Matrix>("Matrix",
		sol::no_constructor,
		"right", &rw::Matrix::right,
		"up",    &rw::Matrix::up,
		"at",    &rw::Matrix::at,
		"pos",   &rw::Matrix::pos,
		sol::meta_function::multiplication, [](rw::Matrix *a, rw::Matrix *b) -> rw::Matrix {
			rw::Matrix dst;
			rw::Matrix::mult(&dst, a, b);
			return dst;
		}
	);
	rwtab.set_function("matInvert", [](rw::Matrix *src) -> rw::Matrix {
		rw::Matrix dst;
		rw::Matrix::invert(&dst, src);
		return dst;
	});
	rwtab.set("COMBINEREPLACE", rw::COMBINEREPLACE);
	rwtab.set("COMBINEPRECONCAT", rw::COMBINEPRECONCAT);
	rwtab.set("COMBINEPOSTCONCAT", rw::COMBINEPOSTCONCAT);

	rwtab.set_function("RGBA", makeRGBA);
	lua.new_usertype<rw::RGBA>("RGBA",
		sol::no_constructor,
		"r", &rw::RGBA::red,
		"g", &rw::RGBA::green,
		"b", &rw::RGBA::blue,
		"a", &rw::RGBA::alpha,
		sol::meta_function::to_string, [](const rw::RGBA &c) {
			return "RGBA(" + std::to_string(c.red) + ", "
			+ std::to_string(c.green) + ", "
			+ std::to_string(c.blue) + ", "
			+ std::to_string(c.alpha) + ")";
		}
	);

	lua.new_usertype<rw::TexDictionary>("rwTexDictionary",
		sol::no_constructor,
		"destroy", &rw::TexDictionary::destroy,
		"count", &rw::TexDictionary::count,
		"add", &rw::TexDictionary::add,
		"addFront", &rw::TexDictionary::addFront,
		"remove", &rw::TexDictionary::remove,
		"find", &rw::TexDictionary::find,
		"textures", [](rw::TexDictionary *txd) {
			rw::LLLink *next = nil;
			rw::LLLink *lnk = txd->textures.link.next;
			return sol::as_function([txd,next,lnk]() mutable -> rw::Texture* {
				if(lnk == txd->textures.end()) return nil;
				rw::Texture *t = Texture::fromDict(lnk);
				lnk = lnk->next;
				return t;
			});
		}
	);
	rwtab.set_function("setCurrentTexDict", [](rw::TexDictionary *txd){
		rw::TexDictionary::setCurrent(txd);
	});
	rwtab.set_function("getCurrentTexDict", [](void){
		return rw::TexDictionary::getCurrent();
	});
	rwtab.set_function("TexDictionaryCreate", &rw::TexDictionary::create);

	lua.new_usertype<rw::Clump>("rwClump",
		sol::no_constructor,
		"destroy", &rw::Clump::destroy,
		"clone", &rw::Clump::clone,
		"getFrame", &rw::Clump::getFrame,
		"setFrame", &rw::Clump::setFrame,
		"countAtomics", &rw::Clump::countAtomics,
		"addAtomic", &rw::Clump::addAtomic,
		"removeAtomic", &rw::Clump::removeAtomic,
		"countLights", &rw::Clump::countLights,
		"addLight", &rw::Clump::addLight,
		"removeLight", &rw::Clump::removeLight,
		"countCameras", &rw::Clump::countCameras,
		"addCamera", &rw::Clump::addCamera,
		"removeCamera", &rw::Clump::removeCamera,
		"atomics", [](rw::Clump *c) {
			rw::LLLink *next = nil;
			rw::LLLink *lnk = c->atomics.link.next;
			return sol::as_function([c,next,lnk]() mutable -> rw::Atomic* {
				if(lnk == c->atomics.end()) return nil;
				rw::Atomic *a = Atomic::fromClump(lnk);
				lnk = lnk->next;
				return a;
			});
		},
		"writeFile", [](rw::Clump *c, const char *path) -> rw::Clump* {
			StreamFile stream;
			if(!stream.open(path, "wb"))
				return nil;
			c->streamWrite(&stream);
			stream.close();
			return c;
		},
		"render", &rw::Clump::render
	);

	lua.new_usertype<rw::Frame>("rwFrame",
		sol::no_constructor,
		"destroy", &rw::Frame::destroy,
		"destroyHierarchy", &rw::Frame::destroyHierarchy,
		"cloneHierarchy", &rw::Frame::cloneHierarchy,
		"getParent", &rw::Frame::getParent,
		"getRoot", [](rw::Frame *f) -> rw::Frame* { return f->root; },
		"children", [](rw::Frame *f) {
			rw::Frame *cur = f->child;
			rw::Frame *next = cur ? cur->next : nil;
			return sol::as_function([cur,next]() mutable -> rw::Frame* {
				rw::Frame *f = cur;
				cur = next;
				next = cur ? cur->next : nil;
				return f;
			});
		},
		"removeChild", &rw::Frame::removeChild,
		"addChild", &rw::Frame::addChild,
		"count", &rw::Frame::count,
		"getLTM", &rw::Frame::getLTM,
		// Returns a copy of the local matrix. Safe to modify; pass back via frame:transform(m, rw.COMBINEREPLACE).
		"copyMatrix", [](rw::Frame *f) -> rw::Matrix { return f->matrix; },
		"transform", &rw::Frame::transform,
		"rotate", [](rw::Frame *f, const rw::V3d *axis, rw::float32 angle, rw::CombineOp op) {
			f->rotate(axis, angle, op);
		},
		"rotateQuat", [](rw::Frame *f, const rw::Quat *q, rw::CombineOp op) {
			f->rotate(q, op);
		},
		"translate", &rw::Frame::translate,
		"scale", &rw::Frame::scale,
		"getName", [](rw::Frame *f) -> std::string {
			return getFrameName(f);
		},
		"atomics", [](rw::Frame *frame) {
			rw::LLLink *lnk = frame->objectList.link.next;
			return sol::as_function([frame,lnk]() mutable -> rw::Atomic* {
				for(;;) {
				if(lnk == frame->objectList.end()) return nil;
				rw::Atomic *a = (rw::Atomic*)rw::ObjectWithFrame::fromFrame(lnk);
				lnk = lnk->next;
				if(a->object.object.type == rw::Atomic::ID)
					return a;
				}
			});
		}
	);
	rwtab.set_function("FrameCreate", rw::Frame::create);

	lua.new_usertype<rw::Atomic>("rwAtomic",
		sol::no_constructor,
		"ptr", [](rw::Atomic *a) -> uintptr_t { return (uintptr_t)a; },
		"destroy", &rw::Atomic::destroy,
		"clone", &rw::Atomic::clone,
		"getFrame", &rw::Atomic::getFrame,
		"setFrame", &rw::Atomic::setFrame,
		"getFlags", &rw::Atomic::getFlags,
		"setFlags", &rw::Atomic::setFlags,
		"getGeometry", [](rw::Atomic *a) -> rw::Geometry* { return a->geometry; },
		"setGeometry", &rw::Atomic::setGeometry,
		"render", &rw::Atomic::render,
		"setVisibility", [](rw::Atomic *a, bool visible) {
			if(visible)
				a->object.object.flags |= rw::Atomic::RENDER;
			else
				a->object.object.flags &= ~rw::Atomic::RENDER;
		}
	);

	lua.new_usertype<rw::Geometry>("rwGeometry",
		sol::no_constructor,
		"destroy", &rw::Geometry::destroy,
		"getFlags", [](rw::Geometry *g) { return g->flags; },
		"getNumTriangles", [](rw::Geometry *g) { return g->numTriangles; },
		"getNumVertices", [](rw::Geometry *g) { return g->numVertices; },
		"getNumTexCoordSets", [](rw::Geometry *g) { return g->numTexCoordSets; },
		"getNumMaterials", [](rw::Geometry *g) { return g->matList.numMaterials; },
		"getMaterial", [](rw::Geometry *g, int i) -> rw::Material* {
			if(i < 0 || i >= g->matList.numMaterials)
				return nil;
			return g->matList.materials[i];
		},
		"materials", [](rw::Geometry *g) {
			int i = 0;
			return sol::as_function([g,i]() mutable -> rw::Material* {
				if(i < g->matList.numMaterials)
					return g->matList.materials[i++];
				return nil;
			});
		},
		"setFlags", [](rw::Geometry *g, uint32 f) { g->flags = f; }
	);
	// Geometry::Flags
	rwtab.set("GEO_TRISTRIP",        (int)rw::Geometry::TRISTRIP);
	rwtab.set("GEO_POSITIONS",       (int)rw::Geometry::POSITIONS);
	rwtab.set("GEO_TEXTURED",        (int)rw::Geometry::TEXTURED);
	rwtab.set("GEO_PRELIT",          (int)rw::Geometry::PRELIT);
	rwtab.set("GEO_NORMALS",         (int)rw::Geometry::NORMALS);
	rwtab.set("GEO_LIGHT",           (int)rw::Geometry::LIGHT);
	rwtab.set("GEO_MODULATE",        (int)rw::Geometry::MODULATE);
	rwtab.set("GEO_TEXTURED2",       (int)rw::Geometry::TEXTURED2);
	rwtab.set("GEO_NATIVE",          (int)rw::Geometry::NATIVE);
	rwtab.set("GEO_NATIVEINSTANCE",  (int)rw::Geometry::NATIVEINSTANCE);
	// Atomic flags
	rwtab.set("ATOMIC_COLLISIONTEST",(int)rw::Atomic::COLLISIONTEST);
	rwtab.set("ATOMIC_RENDER",       (int)rw::Atomic::RENDER);

	lua.new_usertype<rw::Material>("rwMaterial",
		sol::no_constructor,
		"getColor", [](rw::Material *m) -> rw::RGBA { return m->color; },
		"setColor", [](rw::Material *m, rw::RGBA c) { m->color = c; },
		"getSurfaceProps", [](rw::Material *m) -> rw::SurfaceProperties* { return &m->surfaceProps; },
		"getTexture", [](rw::Material *m) -> rw::Texture* { return m->texture; },
		"setTexture", &rw::Material::setTexture,
		"getMatFX", [](rw::Material *m) -> rw::MatFX* { return MatFX::get(m); },
		"setMatFXEffect", [](rw::Material *m, int type) { MatFX::setEffects(m, (uint32)type); }
	);

	lua.new_usertype<rw::SurfaceProperties>("rwSurfaceProperties",
		sol::no_constructor,
		"ambient", &rw::SurfaceProperties::ambient,
		"specular", &rw::SurfaceProperties::specular,
		"diffuse", &rw::SurfaceProperties::diffuse,
		sol::meta_function::to_string, [](const rw::SurfaceProperties &s) {
			return "SurfaceProperties(" + std::to_string(s.ambient) + ", "
			+ std::to_string(s.specular) + ", "
			+ std::to_string(s.diffuse) + ")";
		}
	);

	lua.new_usertype<rw::Raster>("rwRaster",
		sol::no_constructor,
		"width",  [](rw::Raster *r) { return r->width; },
		"height", [](rw::Raster *r) { return r->height; },
		"depth",  [](rw::Raster *r) { return r->depth; },
		"format", [](rw::Raster *r) { return r->format & 0xFF00; },
		"type",   [](rw::Raster *r) { return r->format & 0x07; }
	);
	// Raster::Format
	rwtab.set("RASTER_DEFAULT",   (int)rw::Raster::DEFAULT);
	rwtab.set("RASTER_C1555",     (int)rw::Raster::C1555);
	rwtab.set("RASTER_C565",      (int)rw::Raster::C565);
	rwtab.set("RASTER_C4444",     (int)rw::Raster::C4444);
	rwtab.set("RASTER_LUM8",      (int)rw::Raster::LUM8);
	rwtab.set("RASTER_C8888",     (int)rw::Raster::C8888);
	rwtab.set("RASTER_C888",      (int)rw::Raster::C888);
	rwtab.set("RASTER_D16",       (int)rw::Raster::D16);
	rwtab.set("RASTER_D24",       (int)rw::Raster::D24);
	rwtab.set("RASTER_D32",       (int)rw::Raster::D32);
	rwtab.set("RASTER_C555",      (int)rw::Raster::C555);
	rwtab.set("RASTER_AUTOMIPMAP",(int)rw::Raster::AUTOMIPMAP);
	rwtab.set("RASTER_PAL8",      (int)rw::Raster::PAL8);
	rwtab.set("RASTER_PAL4",      (int)rw::Raster::PAL4);
	rwtab.set("RASTER_MIPMAP",    (int)rw::Raster::MIPMAP);
	// Raster::Type
	rwtab.set("RASTER_NORMAL",        (int)rw::Raster::NORMAL);
	rwtab.set("RASTER_ZBUFFER",       (int)rw::Raster::ZBUFFER);
	rwtab.set("RASTER_CAMERA",        (int)rw::Raster::CAMERA);
	rwtab.set("RASTER_TEXTURE",       (int)rw::Raster::TEXTURE);
	rwtab.set("RASTER_CAMERATEXTURE", (int)rw::Raster::CAMERATEXTURE);

	lua.new_usertype<rw::Texture>("rwTexture",
		sol::no_constructor,
		"getName", [](rw::Texture *t) -> std::string { return std::string(t->name); },
		"getMaskName", [](rw::Texture *t) -> std::string { return std::string(t->mask); },
		"raster", sol::property([](rw::Texture *t) -> rw::Raster* { return t->raster; }),
		"getFilter",   [](rw::Texture *t) { return (int)t->getFilter(); },
		"setFilter",   [](rw::Texture *t, int f) { t->setFilter((rw::Texture::FilterMode)f); },
		"getAddressU", [](rw::Texture *t) { return (int)t->getAddressU(); },
		"setAddressU", [](rw::Texture *t, int a) { t->setAddressU((rw::Texture::Addressing)a); },
		"getAddressV", [](rw::Texture *t) { return (int)t->getAddressV(); },
		"setAddressV", [](rw::Texture *t, int a) { t->setAddressV((rw::Texture::Addressing)a); },
		sol::meta_function::to_string, [](const rw::Texture *t) {
			return "Texture(" + std::string(t->name) + ", " + std::string(t->mask) + ")";
		}
	);
	// Texture::FilterMode
	rwtab.set("FILTER_NEAREST",          (int)rw::Texture::NEAREST);
	rwtab.set("FILTER_LINEAR",           (int)rw::Texture::LINEAR);
	rwtab.set("FILTER_MIPNEAREST",       (int)rw::Texture::MIPNEAREST);
	rwtab.set("FILTER_MIPLINEAR",        (int)rw::Texture::MIPLINEAR);
	rwtab.set("FILTER_LINEARMIPNEAREST", (int)rw::Texture::LINEARMIPNEAREST);
	rwtab.set("FILTER_LINEARMIPLINEAR",  (int)rw::Texture::LINEARMIPLINEAR);
	// Texture::Addressing
	rwtab.set("ADDRESS_WRAP",   (int)rw::Texture::WRAP);
	rwtab.set("ADDRESS_MIRROR", (int)rw::Texture::MIRROR);
	rwtab.set("ADDRESS_CLAMP",  (int)rw::Texture::CLAMP);
	rwtab.set("ADDRESS_BORDER", (int)rw::Texture::BORDER);
	rwtab.set_function("ImageSetSearchPath", &rw::Image::setSearchPath);

	lua.new_usertype<rw::MatFX>("rwMatFX",
		sol::no_constructor,
		"getType",             [](rw::MatFX *fx) { return (int)fx->type; },
		// Bump
		"getBumpTexture",      &rw::MatFX::getBumpTexture,
		"getBumpCoefficient",  &rw::MatFX::getBumpCoefficient,
		"setBumpTexture",      &rw::MatFX::setBumpTexture,
		"setBumpCoefficient",  &rw::MatFX::setBumpCoefficient,
		// Env
		"getEnvTexture",       &rw::MatFX::getEnvTexture,
		"getEnvCoefficient",   &rw::MatFX::getEnvCoefficient,
		"getEnvFBAlpha",       [](rw::MatFX *fx) { return (bool)fx->getEnvFBAlpha(); },
		"setEnvTexture",       &rw::MatFX::setEnvTexture,
		"setEnvCoefficient",   &rw::MatFX::setEnvCoefficient,
		"setEnvFBAlpha",       [](rw::MatFX *fx, bool v) { fx->setEnvFBAlpha(v ? 1 : 0); },
		// Dual
		"getDualTexture",      &rw::MatFX::getDualTexture,
		"getDualSrcBlend",     &rw::MatFX::getDualSrcBlend,
		"getDualDestBlend",    &rw::MatFX::getDualDestBlend,
		"setDualTexture",      &rw::MatFX::setDualTexture,
		"setDualSrcBlend",     &rw::MatFX::setDualSrcBlend,
		"setDualDestBlend",    &rw::MatFX::setDualDestBlend
	);
	rwtab.set("MATFX_NOTHING",         (int)rw::MatFX::NOTHING);
	rwtab.set("MATFX_BUMPMAP",         (int)rw::MatFX::BUMPMAP);
	rwtab.set("MATFX_ENVMAP",          (int)rw::MatFX::ENVMAP);
	rwtab.set("MATFX_BUMPENVMAP",      (int)rw::MatFX::BUMPENVMAP);
	rwtab.set("MATFX_DUAL",            (int)rw::MatFX::DUAL);
	rwtab.set("MATFX_UVTRANSFORM",     (int)rw::MatFX::UVTRANSFORM);
	rwtab.set("MATFX_DUALUVTRANSFORM", (int)rw::MatFX::DUALUVTRANSFORM);
	// BlendFunction
	rwtab.set("BLEND_ZERO",          (int)rw::BLENDZERO);
	rwtab.set("BLEND_ONE",           (int)rw::BLENDONE);
	rwtab.set("BLEND_SRCCOLOR",      (int)rw::BLENDSRCCOLOR);
	rwtab.set("BLEND_INVSRCCOLOR",   (int)rw::BLENDINVSRCCOLOR);
	rwtab.set("BLEND_SRCALPHA",      (int)rw::BLENDSRCALPHA);
	rwtab.set("BLEND_INVSRCALPHA",   (int)rw::BLENDINVSRCALPHA);
	rwtab.set("BLEND_DESTALPHA",     (int)rw::BLENDDESTALPHA);
	rwtab.set("BLEND_INVDESTALPHA",  (int)rw::BLENDINVDESTALPHA);
	rwtab.set("BLEND_DESTCOLOR",     (int)rw::BLENDDESTCOLOR);
	rwtab.set("BLEND_INVDESTCOLOR",  (int)rw::BLENDINVDESTCOLOR);
	rwtab.set("BLEND_SRCALPHASAT",   (int)rw::BLENDSRCALPHASAT);

	lua.new_usertype<rw::Camera>("rwCamera",
		sol::no_constructor,
		"destroy", &rw::Camera::destroy,
		"getFrame", &rw::Camera::getFrame,
		"setFrame", &rw::Camera::setFrame,
		"beginUpdate", &rw::Camera::beginUpdate,
		"clear", &rw::Camera::clear,
		"endUpdate", &rw::Camera::endUpdate,
		"showRaster", &rw::Camera::showRaster
	);
	rwtab.set("Camera_CLEARIMAGE", rw::Camera::CLEARIMAGE);
	rwtab.set("Camera_CLEARZ", rw::Camera::CLEARZ);
	rwtab.set("Camera_CLEARSTENCIL", rw::Camera::CLEARSTENCIL);

	lua.new_usertype<rw::Light>("rwLight",
		sol::no_constructor,
		"destroy", &rw::Light::destroy,
		"getFrame", &rw::Light::getFrame,
		"setFrame", &rw::Light::setFrame,
		"setColor", &rw::Light::setColor
	);
	rwtab.set_function("LightCreate", rw::Light::create);
	rwtab.set("Light_DIRECTIONAL", rw::Light::DIRECTIONAL);
	rwtab.set("Light_AMBIENT", rw::Light::AMBIENT);
	rwtab.set("Light_POINT", rw::Light::POINT);
	rwtab.set("Light_SPOT", rw::Light::SPOT);
	rwtab.set("Light_SOFTSPOT", rw::Light::SOFTSPOT);

	lua.new_usertype<rw::World>("rwWorld",
		sol::no_constructor,
		"destroy", &rw::World::destroy,
		"addLight", &rw::World::addLight,
		"removeLight", &rw::World::removeLight,
		"addCamera", &rw::World::addCamera,
		"removeCamera", &rw::World::removeCamera,
		"addAtomic", &rw::World::addAtomic,
		"removeAtomic", &rw::World::removeAtomic,
		"addClump", &rw::World::addClump,
		"removeClump", &rw::World::removeClump
	);
	rwtab.set_function("WorldCreate", &rw::World::create);

	rwtab.set_function("readClumpFile", [](sol::this_state s,
			const char *name, int offset, int size){
		sol::state_view lua(s);
		auto lose = sol::object(sol::lua_nil);
		StreamFile stream;
		if(!stream.open(name, "rb"))
			return lose;
		stream.seek(offset);

		ChunkHeaderInfo header;
		readChunkHeaderInfo(&stream, &header);
		rw::UVAnimDictionary *dict = rw::currentUVAnimDictionary;
		if(header.type == ID_UVANIMDICT) {
			rw::currentUVAnimDictionary = rw::UVAnimDictionary::streamRead(&stream);
			readChunkHeaderInfo(&stream, &header);
		}
		Clump *clump = header.type == ID_CLUMP ? Clump::streamRead(&stream) : nil;
		rw::currentUVAnimDictionary = dict;
		stream.close();
		if(clump == nil)
			return lose;
		return sol::make_object(lua, clump);
	});

	rwtab.set_function("readTexDictFile", [](sol::this_state s,
			const char *name, int offset, int size){
		sol::state_view lua(s);
		auto lose = sol::object(sol::lua_nil);
		StreamFile stream;
		if(!stream.open(name, "rb"))
			return lose;
		stream.seek(offset);

		ChunkHeaderInfo header;
		readChunkHeaderInfo(&stream, &header);
		if(header.type != ID_TEXDICTIONARY)
			return lose;

		rw::TexDictionary *txd = rw::TexDictionary::streamRead(&stream);
		stream.close();
		if(txd == nil)
			return lose;
		return sol::make_object(lua, txd);
	});

}

void
initLua(sol::state &lua)
{
	lua.open_libraries(sol::lib::base,
		sol::lib::io,
		sol::lib::os,
		sol::lib::string,
		sol::lib::math,
		sol::lib::table);

	lua["include"] = [&lua](const char *path){
		lua.script_file(path);
	};
}

void
execLua(const char *str)
{
	auto result = lua.script_file(str, sol::script_pass_on_error);
	if(!result.valid()){
		sol::error err = result;
		luaError(err.what());
	}
}

