#include "camera.h"

void initLua(void);
void initLuaSkeleton(void);
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
