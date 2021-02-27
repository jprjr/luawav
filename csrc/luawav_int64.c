#include "luawav_internal.h"
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>

static const char * const digits = "0123456789";
const char * const luawav_uint64_mt = "drwav_uint64";
const char * const luawav_int64_mt = "drwav_int64";

static char *
luawav_uint64_to_str(drwav_uint64 value, char buffer[21], size_t *len) {
    char *p = buffer + 20;
    *p = '\0';
    do {
        *--p = digits[value % 10];
    } while (value /= 10);

    *len = 20 - (p - buffer);
    return p;
}

static char *
luawav_int64_to_str(drwav_int64 value, char buffer[22], size_t *len) {
    int sign;
    drwav_uint64 tmp;

    if(value < 0) {
        sign = 1;
        tmp = -value;
    } else {
        sign = 0;
        tmp = value;
    }
    char *p = luawav_uint64_to_str(tmp,buffer+1,len);
    if(sign) {
        *--p = '-';
        *len += 1;
    }
    return p;
}

static int
luawav_uint64(lua_State *L) {
    /* create a new int64 object from a number or string */
    drwav_uint64 *t = 0;

    t = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    if(t == NULL) {
        return luaL_error(L,"out of memory");
    }

    if(lua_gettop(L) > 1) {
        *t = luawav_touint64(L,1);
    }
    else {
        *t = 0;
    }

    luaL_setmetatable(L,luawav_uint64_mt);
    return 1;
}


static int
luawav_uint64__unm(lua_State *L) {
    drwav_uint64 *o = NULL;
    drwav_int64 *r = NULL;

    o = lua_touserdata(L,1);

    if(*o > 0x8000000000000000) {
        return luaL_error(L,"out of range");
    }

    r = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *r = -(*o);

    luaL_setmetatable(L,luawav_int64_mt);
    return 1;
}

static int
luawav_uint64__add(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a + b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__sub(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a - b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__mul(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a * b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__div(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a / b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__mod(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a % b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__pow(lua_State *L) {
    drwav_uint64 base = 0;
    drwav_uint64 exp = 0;
    drwav_uint64 *res = NULL;
    drwav_uint64 result = 1;

    base = luawav_touint64(L,1);
    exp = luawav_touint64(L,2);

    for (;;) {
        if(exp & 1) {
            result *= base;
        }
        exp >>= 1;
        if(!exp) {
            break;
        }
        base *= base;
    }

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = result;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__eq(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    lua_pushboolean(L,a==b);
    return 1;
}

static int
luawav_uint64__lt(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    lua_pushboolean(L,a<b);
    return 1;
}

static int
luawav_uint64__le(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    lua_pushboolean(L,a<=b);
    return 1;
}

static int
luawav_uint64__band(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a & b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__bor(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a | b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__bxor(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = a ^ b;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__bnot(lua_State *L) {
    drwav_uint64 *o = NULL;
    drwav_uint64 *r = NULL;

    o = lua_touserdata(L,1);

    r = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));

    *r = ~*o;

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__shl(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = (drwav_uint64)(a << b);

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__shr(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_uint64 *res = NULL;

    a = luawav_touint64(L,1);
    b = luawav_touint64(L,2);

    res = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
    *res = (drwav_uint64)(a >> b);

    luaL_setmetatable(L,luawav_uint64_mt);

    return 1;
}

static int
luawav_uint64__tostring(lua_State *L) {
    char *p;
    char t[21];
    size_t l;

    drwav_uint64 *o = (drwav_uint64 *)lua_touserdata(L,1);
    p = luawav_uint64_to_str(*o,t,&l);
    lua_pushlstring(L,p,l);
    return 1;
}

static int
luawav_uint64__concat(lua_State *L) {
    lua_getglobal(L,"tostring");
    lua_pushvalue(L,1);
    lua_call(L,1,1);

    lua_getglobal(L,"tostring");
    lua_pushvalue(L,2);
    lua_call(L,1,1);

    lua_concat(L,2);
    return 1;
}

static int
luawav_int64__unm(lua_State *L) {
    drwav_int64 *o = NULL;
    drwav_int64 *r = NULL;
    drwav_uint64 *t = NULL;

    o = lua_touserdata(L,1);

    if(*o == ((drwav_int64) 0x8000000000000000)) {
        t = (drwav_uint64 *)lua_newuserdata(L,sizeof(drwav_uint64));
        *t = -(*o);
        luaL_setmetatable(L,luawav_uint64_mt);
    } else {
        r = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
        *r = -(*o);
        luaL_setmetatable(L,luawav_int64_mt);
    }

    return 1;
}

static int
luawav_int64__add(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    if(res == NULL) {
        return luaL_error(L,"out of memory");
    }

    *res = a + b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__sub(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a - b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__mul(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a * b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__div(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a / b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__mod(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a % b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__pow(lua_State *L) {
    drwav_int64 base = 0;
    drwav_int64 exp = 0;
    drwav_int64 *res = NULL;
    drwav_int64 result = 1;

    base = luawav_toint64(L,1);
    exp = luawav_toint64(L,2);

    if(exp < 0) {
        return luaL_error(L,"exp must be positive");
    }

    for (;;) {
        if(exp & 1) {
            result *= base;
        }
        exp >>= 1;
        if(!exp) {
            break;
        }
        base *= base;
    }

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = result;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__eq(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    lua_pushboolean(L,a==b);
    return 1;
}

static int
luawav_int64__lt(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    lua_pushboolean(L,a<b);
    return 1;
}

static int
luawav_int64__le(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    lua_pushboolean(L,a<=b);
    return 1;
}

static int
luawav_int64__band(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a & b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__bor(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a | b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__bxor(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = a ^ b;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__bnot(lua_State *L) {
    drwav_int64 *o = NULL;
    drwav_int64 *r = NULL;

    o = lua_touserdata(L,1);

    r = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));

    *r = ~*o;

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__shl(lua_State *L) {
    drwav_int64 a = 0;
    drwav_int64 b = 0;
    drwav_int64 *res = NULL;

    a = luawav_toint64(L,1);
    b = luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = (drwav_int64)(a << b);

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__shr(lua_State *L) {
    drwav_uint64 a = 0;
    drwav_uint64 b = 0;
    drwav_int64 *res = NULL;

    a = (drwav_uint64)luawav_toint64(L,1);
    b = (drwav_uint64)luawav_toint64(L,2);

    res = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    *res = (drwav_int64)(a >> b);

    luaL_setmetatable(L,luawav_int64_mt);

    return 1;
}

static int
luawav_int64__tostring(lua_State *L) {
    char *p;
    char t[22];
    size_t l;

    drwav_int64 *o = (drwav_int64 *)lua_touserdata(L,1);
    p = luawav_int64_to_str(*o,t,&l);
    lua_pushlstring(L,p,l);
    return 1;
}

static int
luawav_int64__concat(lua_State *L) {
    lua_getglobal(L,"tostring");
    lua_pushvalue(L,1);
    lua_call(L,1,1);

    lua_getglobal(L,"tostring");
    lua_pushvalue(L,2);
    lua_call(L,1,1);

    lua_concat(L,2);
    return 1;
}

static int
luawav_int64(lua_State *L) {
    /* create a new int64 object from a number or string */
    drwav_int64 *t = 0;

    t = (drwav_int64 *)lua_newuserdata(L,sizeof(drwav_int64));
    if(t == NULL) {
        return luaL_error(L,"out of memory");
    }

    if(lua_gettop(L) > 1) {
        *t = luawav_toint64(L,1);
    }
    else {
        *t = 0;
    }

    luaL_setmetatable(L,luawav_int64_mt);
    return 1;
}

static const struct luaL_Reg luawav_int64_metamethods[] = {
    { "__add", luawav_int64__add },
    { "__sub", luawav_int64__sub },
    { "__mul", luawav_int64__mul },
    { "__div", luawav_int64__div },
    { "__idiv", luawav_int64__div },
    { "__mod", luawav_int64__mod },
    { "__pow", luawav_int64__pow },
    { "__unm", luawav_int64__unm },
    { "__band", luawav_int64__band },
    { "__bor", luawav_int64__bor },
    { "__bxor", luawav_int64__bxor },
    { "__bnot", luawav_int64__bnot },
    { "__shl", luawav_int64__shl },
    { "__shr", luawav_int64__shr },
    { "__eq", luawav_int64__eq },
    { "__lt", luawav_int64__lt },
    { "__le", luawav_int64__le },
    { "__tostring", luawav_int64__tostring },
    { "__concat", luawav_int64__concat },
    { NULL, NULL },
};

static const struct luaL_Reg luawav_uint64_metamethods[] = {
    { "__add", luawav_uint64__add },
    { "__sub", luawav_uint64__sub },
    { "__mul", luawav_uint64__mul },
    { "__div", luawav_uint64__div },
    { "__idiv", luawav_uint64__div },
    { "__mod", luawav_uint64__mod },
    { "__pow", luawav_uint64__pow },
    { "__unm", luawav_uint64__unm },
    { "__band", luawav_uint64__band },
    { "__bor", luawav_uint64__bor },
    { "__bxor", luawav_uint64__bxor },
    { "__bnot", luawav_uint64__bnot },
    { "__shl", luawav_uint64__shl },
    { "__shr", luawav_uint64__shr },
    { "__eq", luawav_uint64__eq },
    { "__lt", luawav_uint64__lt },
    { "__le", luawav_uint64__le },
    { "__tostring", luawav_uint64__tostring },
    { "__concat", luawav_uint64__concat },
    { NULL, NULL },
};

static void
register_metatables(lua_State *L) {
    /* whether we're loading int64 or uint64, ensure
     * metatables always exist since they depend on
     * eachother */

    luaL_newmetatable(L,luawav_int64_mt);
    luaL_setfuncs(L,luawav_int64_metamethods,0);
    lua_pop(L,1);

    luaL_newmetatable(L,luawav_uint64_mt);
    luaL_setfuncs(L,luawav_uint64_metamethods,0);
    lua_pop(L,1);
}

LUAWAV_PRIVATE
drwav_uint64 luawav_touint64(lua_State *L, int idx) {
    drwav_uint64 *t = NULL;
    drwav_int64 *r = NULL;
    drwav_uint64 tmp  = 0;
    const char *str = NULL;
    const char *s = NULL;
    char *end = NULL;

    switch(lua_type(L,idx)) {
        case LUA_TNONE: {
            return 0;
        }
        case LUA_TNIL: {
            return 0;
        }
        case LUA_TBOOLEAN: {
            return (drwav_uint64)lua_toboolean(L,idx);
        }
        case LUA_TNUMBER: {
            return (drwav_uint64)lua_tointeger(L,idx);
        }
        case LUA_TUSERDATA: {
            t = luaL_testudata(L,idx,luawav_uint64_mt);
            if(t != NULL) {
                return *t;
            }
            r = luaL_testudata(L,idx,luawav_int64_mt);
            if(r != NULL) {
                if(*r < 0) {
                    luaL_error(L,"out of range");
                    return 0;
                }
                return (drwav_uint64)*r;
            }
            break;
        }
        /* we'll try converting to a string */
        default: break;
    }

    str = lua_tostring(L,idx);
    if(str == NULL) {
        luaL_error(L,"invalid value");
        return 0;
    }

    s = str;
    while(*s) {
        if(isspace(*s)) {
            s++;
        } else {
            break;
        }
    }

    if(!*s) { /* empty string, or just spaces */
        luaL_error(L,"invalid string, %s has no characters",str);
        return 0;
    }
    if(*s == '-') {
        luaL_error(L,"invalid string, %s is negative",str);
        return 0;
    }
    errno = 0;
    tmp = strtoull(s, &end, 10);
    if(errno) {
        luaL_error(L,"invalid integer string");
        return 0;
    }
    if(s == end) {
        luaL_error(L,"invalid string");
        return 0;
    }
    return tmp;
}

LUAWAV_PRIVATE
drwav_int64 luawav_toint64(lua_State *L, int idx) {
    drwav_int64 *t = NULL;
    drwav_uint64 *r = NULL;
    drwav_int64 tmp  = 0;
    const char *str = NULL;

    switch(lua_type(L,idx)) {
        case LUA_TNONE: {
            return 0;
        }
        case LUA_TNIL: {
            return 0;
        }
        case LUA_TBOOLEAN: {
            return (drwav_int64)lua_toboolean(L,idx);
        }
        case LUA_TNUMBER: {
            return (drwav_int64)lua_tointeger(L,idx);
        }
        case LUA_TUSERDATA: {
            t = luaL_testudata(L,idx,luawav_int64_mt);
            if(t != NULL) {
                return *t;
            }
            r = luaL_testudata(L,idx,luawav_uint64_mt);
            if(r != NULL) {
                if(*r > 0x7FFFFFFFFFFFFFFF) {
                    luaL_error(L,"out of range");
                    return 0;
                }
                return (drwav_int64) *r;
            }
            break;
        }
        /* we'll try converting to a string */
        default: break;
    }

    str = lua_tostring(L,idx);
    if(str == NULL) {
        luaL_error(L,"invalid value");
        return 0;
    }
    errno = 0;
    tmp = strtoull(str, NULL, 10);
    if(errno) {
        luaL_error(L,"invalid integer string");
        return 0;
    }
    return tmp;
}

LUAWAV_PRIVATE
void luawav_pushuint64(lua_State *L, drwav_uint64 v) {
    drwav_uint64 *d = NULL;
    d = lua_newuserdata(L,sizeof(drwav_uint64));
    if(d == NULL) {
        luaL_error(L,"out of memory");
    }
    *d = v;
    luaL_setmetatable(L,luawav_uint64_mt);
}

LUAWAV_PRIVATE
void luawav_pushint64(lua_State *L, drwav_int64 v) {
    drwav_int64 *d = NULL;
    d = lua_newuserdata(L,sizeof(drwav_int64));
    if(d == NULL) {
        luaL_error(L,"out of memory");
    }
    *d = v;
    luaL_setmetatable(L,luawav_int64_mt);
}

LUAWAV_PUBLIC
int luaopen_luawav_int64(lua_State *L) {
    register_metatables(L);
    lua_pushcclosure(L,luawav_int64,0);
    return 1;
}

LUAWAV_PUBLIC
int luaopen_luawav_uint64(lua_State *L) {
    register_metatables(L);
    lua_pushcclosure(L,luawav_uint64,0);
    return 1;
}

