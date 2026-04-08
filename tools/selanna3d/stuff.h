#include "camera.h"

void initLua(sol::state &lua);
void registerRW(sol::state &lua);
void registerGTA(sol::state &lua);
void registerSkeleton(sol::state &lua);
void execLua(const char *str);
void luaError(const char *msg);

extern sol::state lua;

template<typename... Args>
void
luaCall(const char *name, Args&&... args)
{
	sol::protected_function f = lua[name];
	if(!f.valid()) return;
	auto result = f(std::forward<Args>(args)...);
	if(!result.valid()){
		sol::error err = result;
		luaError(err.what());
	}
}

using rw::int32;
using rw::uint32;
using rw::int16;
using rw::uint16;
using rw::int8;
using rw::uint8;
using rw::float32;
