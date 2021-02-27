#include <lua.h>
#include <lauxlib.h>

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(_MSC_VER)
#define LUAWAV_PUBLIC __declspec(dllexport)
#elif __GNUC__ > 4
#define LUAWAV_PUBLIC __attribute__ ((visibility ("default")))
#else
#define LUAWAV_PUBLIC
#endif

#ifdef __cplusplus
extern "C" {
#endif

LUAWAV_PUBLIC
int luaopen_luawav(lua_State *L);

LUAWAV_PUBLIC
int luaopen_luawav_decoder(lua_State *L);

#ifdef __cplusplus
}
#endif
