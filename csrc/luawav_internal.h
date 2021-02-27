#include "luawav.h"
#include "dr_wav.h"

#if __GNUC__ > 4
#define LUAWAV_PRIVATE __attribute__ ((visibility ("hidden")))
#else
#define LUAWAV_PRIVATE
#endif

typedef struct luawav_metamethods_s {
    const char *name;
    const char *metaname;
} luawav_metamethods;


#if (!defined LUA_VERSION_NUM) || LUA_VERSION_NUM == 501
#define lua_setuservalue(L,i) lua_setfenv((L),(i))
#define lua_getuservalue(L,i) lua_getfenv((L),(i))
#define lua_rawlen(L,i) lua_objlen((L),(i))
#endif

#define luawav_push_const(x) lua_pushinteger(L,x) ; lua_setfield(L,-2, #x)

#ifdef __cplusplus
extern "C" {
#endif

LUAWAV_PRIVATE
drwav_uint64
luawav_touint64(lua_State *L, int idx);

LUAWAV_PRIVATE
drwav_int64
luawav_toint64(lua_State *L, int idx);

LUAWAV_PRIVATE
void
luawav_pushuint64(lua_State *L, drwav_uint64 v);

LUAWAV_PRIVATE
void
luawav_pushint64(lua_State *L, drwav_int64 v);

LUAWAV_PRIVATE
extern const char * const luawav_uint64_mt;

LUAWAV_PRIVATE
extern const char * const luawav_int64_mt;

#if !defined(luaL_newlibtable) \
  && (!defined LUA_VERSION_NUM || LUA_VERSION_NUM==501)
LUAWAV_PRIVATE
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);

LUAWAV_PRIVATE
void luaL_setmetatable(lua_State *L, const char *str);

LUAWAV_PRIVATE
void *luaL_testudata (lua_State *L, int i, const char *tname);
#endif

#ifdef __cplusplus
}
#endif
