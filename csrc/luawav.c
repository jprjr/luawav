#include "luawav.h"
#include "luawav_internal.h"
#include <assert.h>
#include <string.h>

#include <stdio.h>

#define F32_BUFFER 4096
#define S32_BUFFER F32_BUFFER
#define S16_BUFFER S32_BUFFER * 2
#define WAV_MIN(a,b) ( (a) < (b) ? (a) : (b) )

LUAWAV_PRIVATE
const char * const luawav_mt = "drwav";

/* used on the read, seek, write callbacks */
struct luawav_stream_userdata_s {
    lua_State *L;
    int table_ref;
};

typedef struct luawav_stream_userdata_s  luawav_stream_userdata;

/* used on the read, seek, write callbacks */
struct luawav_chunk_userdata_s {
    lua_State *L;
    int table_ref;
};

typedef struct luawav_chunk_userdata_s  luawav_chunk_userdata;

struct luawav_userdata_s {
    luawav_stream_userdata stream;
    luawav_chunk_userdata chunk;
    drwav wav;
    drwav_data_format format;
    float pcm_float[F32_BUFFER];
    drwav_int32 *pcm_int32;
    drwav_int16 *pcm_int16;
    int (*write)(lua_State *L, struct luawav_userdata_s *u);
};

typedef struct luawav_userdata_s luawav_userdata;

struct luawav_const_s {
    const char *name;
    int value;
};

typedef struct luawav_const_s luawav_const;

#define LUAWAV_CONST(x) { #x, x }

static int
luawav_write_pcm_frames_f32(lua_State *L,luawav_userdata *u) {
    drwav_uint64 samplesToWrite = 0;
    drwav_uint64 r = 0;
    drwav_uint64 t = 0;
    drwav_uint64 n = 0;
    drwav_uint64 i = 0;

    samplesToWrite = lua_rawlen(L,2);
    if(samplesToWrite % u->wav.channels != 0) {
        return luaL_error(L,"incomplete frame given");
    }

    while(r<samplesToWrite) {
        n = WAV_MIN( samplesToWrite - r, F32_BUFFER );
        i = 0;
        while(i<n) {
            lua_rawgeti(L,2,1 + i + r);
            u->pcm_float[i] = lua_tonumber(L,-1);
            lua_pop(L,1);
            i++;
        }
        t = drwav_write_pcm_frames(&u->wav,n / u->wav.channels,u->pcm_float);
        if(n != t * u->wav.channels) break;
        r += n;
    }

    luawav_pushuint64(L,r);
    return 1;
}

static int
luawav_write_pcm_frames_s32(lua_State *L,luawav_userdata *u) {
    drwav_uint64 samplesToWrite = 0;
    drwav_uint64 r = 0;
    drwav_uint64 t = 0;
    drwav_uint64 n = 0;
    drwav_uint64 i = 0;

    samplesToWrite = lua_rawlen(L,2);
    if(samplesToWrite % u->wav.channels != 0) {
        return luaL_error(L,"incomplete frame given");
    }

    while(r<samplesToWrite) {
        n = WAV_MIN( samplesToWrite - r, S32_BUFFER );
        i = 0;
        while(i<n) {
            lua_rawgeti(L,2,1 + i + r);
            u->pcm_int32[i] = lua_tointeger(L,-1);
            lua_pop(L,1);
            i++;
        }
        t = drwav_write_pcm_frames(&u->wav,n / u->wav.channels,u->pcm_int32);
        if(n != t * u->wav.channels) break;
        r += n;
    }

    luawav_pushuint64(L,r);
    return 1;
}

static int
luawav_write_pcm_frames_s16(lua_State *L,luawav_userdata *u) {
    drwav_uint64 samplesToWrite = 0;
    drwav_uint64 r = 0;
    drwav_uint64 t = 0;
    drwav_uint64 n = 0;
    drwav_uint64 i = 0;

    samplesToWrite = lua_rawlen(L,2);
    if(samplesToWrite % u->wav.channels != 0) {
        return luaL_error(L,"incomplete frame given");
    }

    while(r<samplesToWrite) {
        n = WAV_MIN( samplesToWrite - r, S16_BUFFER );
        i = 0;
        while(i<n) {
            lua_rawgeti(L,2,1 + i + r);
            u->pcm_int16[i] = lua_tointeger(L,-1);
            lua_pop(L,1);
            i++;
        }
        t = drwav_write_pcm_frames(&u->wav,n / u->wav.channels,u->pcm_int16);
        if(n != t * u->wav.channels) break;
        r += n;
    }

    luawav_pushuint64(L,r);
    return 1;
}

static void
luawav_push_fmt(lua_State *L, const drwav_fmt *fmt) {
    lua_newtable(L);
    lua_pushinteger(L,fmt->formatTag);
    lua_setfield(L,-2,"formatTag");
    lua_pushinteger(L,fmt->channels);
    lua_setfield(L,-2,"channels");
    lua_pushinteger(L,fmt->sampleRate);
    lua_setfield(L,-2,"sampleRate");
    lua_pushinteger(L,fmt->avgBytesPerSec);
    lua_setfield(L,-2,"avgBytesPerSec");
    lua_pushinteger(L,fmt->blockAlign);
    lua_setfield(L,-2,"blockAlign");
    lua_pushinteger(L,fmt->bitsPerSample);
    lua_setfield(L,-2,"bitsPerSample");
    lua_pushinteger(L,fmt->extendedSize);
    lua_setfield(L,-2,"extendedSize");
    lua_pushinteger(L,fmt->validBitsPerSample);
    lua_setfield(L,-2,"validBitsPerSample");
    lua_pushinteger(L,fmt->channelMask);
    lua_setfield(L,-2,"channelMask");
    lua_pushlstring(L,(const char *)fmt->subFormat,16);
    lua_setfield(L,-2,"subFormat");
}

static void
luawav_tofmt(lua_State *L, int idx, drwav_data_format *fmt) {

    lua_getfield(L,idx,"container");
    fmt->container = lua_tointeger(L,-1);
    lua_pop(L,1);

    lua_getfield(L,idx,"format");
    fmt->format = lua_tointeger(L,-1);
    lua_pop(L,1);

    lua_getfield(L,idx,"channels");
    fmt->channels = lua_tointeger(L,-1);
    lua_pop(L,1);

    lua_getfield(L,idx,"sampleRate");
    fmt->sampleRate = lua_tointeger(L,-1);
    lua_pop(L,1);

    lua_getfield(L,idx,"bitsPerSample");
    fmt->bitsPerSample = lua_tointeger(L,-1);
    lua_pop(L,1);

}

static size_t luawav_read_proc(void *userdata, void *bufferout, size_t bytesToRead) {
    luawav_stream_userdata *u = (luawav_stream_userdata *)userdata;
    const char *data = 0;
    size_t datalen = 0;

    lua_rawgeti(u->L,LUA_REGISTRYINDEX,u->table_ref);
    lua_getfield(u->L,-1,"onRead");
    lua_getfield(u->L,-2,"userData");
    lua_pushinteger(u->L,bytesToRead);
    lua_call(u->L,2,1);

    data = lua_tolstring(u->L,-1,&datalen);
    if(datalen > 0) {
        memcpy(bufferout,data,datalen);
    }
    lua_pop(u->L,2);
    return datalen;
}

static size_t luawav_write_proc(void *userdata, const void *bufferout, size_t bytesToWrite) {
    luawav_stream_userdata *u = (luawav_stream_userdata *)userdata;
    size_t written = 0;

    lua_rawgeti(u->L,LUA_REGISTRYINDEX,u->table_ref);
    lua_getfield(u->L,-1,"onWrite");
    lua_getfield(u->L,-2,"userData");
    lua_pushlstring(u->L,bufferout,bytesToWrite);
    lua_call(u->L,2,1);

    written = lua_tointeger(u->L,-1);

    lua_pop(u->L,2);
    return written;
}

static drwav_bool32 luawav_seek_proc(void *userdata, int offset, drwav_seek_origin origin) {
    luawav_stream_userdata *u = (luawav_stream_userdata *)userdata;
    drwav_bool32 r = 0;
    lua_rawgeti(u->L,LUA_REGISTRYINDEX, u->table_ref);
    lua_getfield(u->L,-1,"onSeek");
    lua_getfield(u->L,-2,"userData");
    if(origin == drwav_seek_origin_start) {
        lua_pushliteral(u->L,"set");
    } else {
        lua_pushliteral(u->L,"cur");
    }
    lua_pushinteger(u->L,offset);
    lua_call(u->L,3,1);
    r = lua_toboolean(u->L,-1);
    lua_pop(u->L,2);
    return r;
}

static drwav_uint64
luawav_chunk_proc(void *chunkUserData, drwav_read_proc onRead, drwav_seek_proc onSeek, void *readSeekUserData, const drwav_chunk_header *pChunkHeader, drwav_container container, const drwav_fmt *fmt) {
    luawav_chunk_userdata *u = (luawav_chunk_userdata *)chunkUserData;
    luawav_stream_userdata *s = (luawav_stream_userdata *)readSeekUserData;
    drwav_uint64 r = 0;

    lua_rawgeti(u->L,LUA_REGISTRYINDEX, u->table_ref);
    lua_getfield(u->L,-1,"onChunk");
    lua_getfield(u->L,-2,"chunkUserData");

    lua_rawgeti(s->L,LUA_REGISTRYINDEX,s->table_ref);

    lua_newtable(u->L); /* chunk_header */

    luawav_pushuint64(u->L,pChunkHeader->sizeInBytes);
    lua_setfield(u->L,-2,"sizeInBytes");

    lua_pushinteger(u->L,pChunkHeader->paddingSize);
    lua_setfield(u->L,-2,"paddingSize");

    if(container == drwav_container_riff || container == drwav_container_rf64) {
        lua_pushlstring(u->L,(const char *)pChunkHeader->id.fourcc,4);
        lua_setfield(u->L,-2,"fourcc");
    } else {
        lua_pushlstring(u->L,(const char *)pChunkHeader->id.guid,16);
        lua_setfield(u->L,-2,"guid");
    }

    luawav_push_fmt(u->L,fmt);

    lua_call(u->L,4,1);
    r = luawav_touint64(u->L,-1);
    lua_pop(u->L,2);

    (void)onRead;
    (void)onSeek;

    return r;
}

static void
copydown(lua_State *L, const char *tablename) {
    lua_getglobal(L,"require");
    lua_pushstring(L,tablename);
    lua_call(L,1,1);

    /* copies keys from table on top of stack to table below */
    lua_pushnil(L);
    while(lua_next(L,-2) != 0) {
        /* -1 = value
         * -2 = key */
        lua_pushvalue(L,-2);
        lua_insert(L,-2);
        lua_settable(L,-5);
    }
    lua_pop(L,1);
}

static int
luawav_drwav(lua_State *L) {
    luawav_userdata *u = NULL;
    u = lua_newuserdata(L,sizeof(luawav_userdata));
    if(u == NULL) {
        return luaL_error(L,"out of memory");
    }
    luaL_setmetatable(L,luawav_mt);

    u->stream.L = NULL;
    u->chunk.L = NULL;

    u->stream.table_ref = LUA_NOREF;
    u->chunk.table_ref = LUA_NOREF;

    memset(u->pcm_float,0,sizeof(float) * F32_BUFFER);
    u->pcm_int32 = (drwav_int32 *)u->pcm_float;
    u->pcm_int16 = (drwav_int16 *)u->pcm_float;
    u->write = NULL;

    return 1;
}

static int
luawav_uninit(lua_State *L) {
    luawav_userdata *u = NULL;
    u = luaL_checkudata(L,1,luawav_mt);

    drwav_uninit(&u->wav);

    if(u->stream.table_ref != LUA_NOREF) {
        luaL_unref(L,LUA_REGISTRYINDEX,u->stream.table_ref);
        u->stream.table_ref = LUA_NOREF;
    }

    if(u->chunk.table_ref != LUA_NOREF) {
        luaL_unref(L,LUA_REGISTRYINDEX,u->chunk.table_ref);
        u->chunk.table_ref = LUA_NOREF;
    }

    return 0;
}

static int
luawav_init_write(lua_State *L) {
    luawav_userdata *u = NULL;
    const char *filename = NULL;
    int seq = 0;
    drwav_uint64 totalSamples = 0;

    u = luaL_checkudata(L,1,luawav_mt);

    if(lua_isstring(L,2)) {
        filename = lua_tostring(L,2);
    } else if(!lua_istable(L,2)) {
        return luaL_error(L,"missing required parameter: filename or callback table");
    }

    if(!lua_istable(L,3)) {
        return luaL_error(L,"missing format table");
    }
    luawav_tofmt(L,3,&u->format);

    u->write = NULL;
    if(u->format.format == DR_WAVE_FORMAT_IEEE_FLOAT) {
        u->write = luawav_write_pcm_frames_f32;
    } else if(u->format.format == DR_WAVE_FORMAT_PCM) {
        if(u->format.bitsPerSample == 32) {
            u->write = luawav_write_pcm_frames_s32;
        } else if(u->format.bitsPerSample == 16) {
            u->write = luawav_write_pcm_frames_s16;
        }
    }

    if(u->write == NULL) {
        return luaL_error(L,"format not supported");
    }

    if(filename == NULL) {
        u->stream.L = L;
        if(u->stream.table_ref != LUA_NOREF) {
            luaL_unref(L,LUA_REGISTRYINDEX,u->stream.table_ref);
        }
        lua_newtable(L);

        lua_getfield(L,2,"onWrite");
        if(lua_isnil(L,-1)) {
            return luaL_error(L,"missing required onWrite function");
        }
        lua_setfield(L,-2,"onWrite");

        lua_getfield(L,2,"totalSamples");
        if(!lua_isnil(L,-1)) {
            seq = 1;
            totalSamples = luawav_touint64(L,-1);
        }
        lua_pop(L,1);

        lua_getfield(L,2,"totalFrames");
        if(!lua_isnil(L,-1)) {
            seq = 2;
            totalSamples = luawav_touint64(L,-1);
        }
        lua_pop(L,1);

        if(seq== 0) {
            lua_getfield(L,2,"onSeek");
            if(lua_isnil(L,-1)) {
                return luaL_error(L,"missing required onSeek function");
            }
            lua_setfield(L,-2,"onSeek");
        }

        lua_getfield(L,2,"userData");
        lua_setfield(L,-2,"userData");

        u->stream.table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

        if(seq == 0) {
            lua_pushboolean(L,drwav_init_write(&u->wav,
                &u->format,
                luawav_write_proc,
                luawav_seek_proc,
                &u->stream,
                NULL));
        } else if(seq == 1) {
            lua_pushboolean(L,drwav_init_write_sequential(&u->wav,
                &u->format,
                totalSamples,
                luawav_write_proc,
                &u->stream,
                NULL));
        } else if(seq == 2) {
            lua_pushboolean(L,drwav_init_write_sequential_pcm_frames(&u->wav,
                &u->format,
                totalSamples,
                luawav_write_proc,
                &u->stream,
                NULL));
        }
    }
    else {
        lua_pushboolean(L,drwav_init_file_write(&u->wav,
          filename,
          &u->format,
          NULL));
    }

    return 1;
}

/* can be called as
 * wav:init(filename) or
 * wav:init({
 *   filename = filename,
 *   onChunk = onChunk
 * }) or
 * wav:init({
 *   onRead = onRead,
 *   onSeek = onSeek,
 *   onChunk = onChunk
 * }) */

static int
luawav_init_file(lua_State *L, luawav_userdata *u, const char *filename) {
    /* checks if parameter 2 is a file, or table with an onChunk callback and
     * flags */
    int ex = 0;
    drwav_uint32 flags = 0;
    int r = 0;

    if(lua_istable(L,2)) {
        lua_getfield(L,2,"onChunk");
        if(!lua_isnil(L,-1)) {
            lua_newtable(L);
            lua_insert(L,-2);
            lua_setfield(L,-2,"onChunk");
            lua_getfield(L,2,"chunkUserData");
            lua_setfield(L,-2,"chunkUserData");
            lua_getfield(L,2,"flags");
            if(!lua_isnil(L,-1)) {
                flags = lua_tointeger(L,-1);
            }
            lua_pop(L,1);
            u->chunk.table_ref = luaL_ref(L,LUA_REGISTRYINDEX);
            ex = 1;
        } else {
            lua_pop(L,1);
        }
    }

    if(ex) {
        r = drwav_init_file_ex(&u->wav,filename,luawav_chunk_proc,&u->chunk, flags, NULL);
    } else {
        r = drwav_init_file(&u->wav,filename,NULL);
    }
    return r;
}

static int
luawav_init_stream(lua_State *L, luawav_userdata *u) {
    int ex = 0;
    int r = 0;
    drwav_uint32 flags = 0;

    lua_newtable(L);

    lua_getfield(L,2,"onRead");
    if(lua_isnil(L,-1)) {
        return luaL_error(L,"missing required onRead function");
    }
    lua_setfield(L,-2,"onRead");

    lua_getfield(L,2,"onSeek");
    if(lua_isnil(L,-1)) {
        return luaL_error(L,"missing required onSeek function");
    }
    lua_setfield(L,-2,"onSeek");

    lua_getfield(L,2,"userData");
    lua_setfield(L,-2,"userData");

    u->stream.table_ref = luaL_ref(L,LUA_REGISTRYINDEX);

    lua_getfield(L,2,"flags");
    if(lua_isnumber(L,-1)) {
        flags = lua_tointeger(L,-1);
    }
    lua_pop(L,1);

    lua_getfield(L,2,"onChunk");
    if(!lua_isnil(L,-1)) {
        ex = 1;
        lua_newtable(L);
        lua_insert(L,-2);
        lua_setfield(L,-2,"onChunk");
        lua_getfield(L,2,"chunkUserData");
        lua_setfield(L,-2,"chunkUserData");
        u->chunk.table_ref = luaL_ref(L,LUA_REGISTRYINDEX);
    } else {
        lua_pop(L,1);
    }

    if(ex) {
        r = drwav_init_ex(&u->wav,
          luawav_read_proc,
          luawav_seek_proc,
          luawav_chunk_proc,
          &u->stream,
          &u->chunk,
          flags,
          NULL);
    } else {
        r = drwav_init(&u->wav,
          luawav_read_proc,
          luawav_seek_proc,
          &u->stream,
          NULL);
    }
    return r;
}

static int
luawav_init(lua_State *L) {
    int r = 0;
    luawav_userdata *u = NULL;
    const char *filename = NULL;

    u = luaL_checkudata(L,1,luawav_mt);

    if(u->stream.table_ref != LUA_NOREF) {
        luaL_unref(L,LUA_REGISTRYINDEX,u->stream.table_ref);
        u->stream.table_ref = LUA_NOREF;
    }
    if(u->chunk.table_ref != LUA_NOREF) {
        luaL_unref(L,LUA_REGISTRYINDEX,u->chunk.table_ref);
        u->chunk.table_ref = LUA_NOREF;
    }
    u->stream.L = L;
    u->chunk.L = L;

    if(lua_isstring(L,2)) {
        filename = lua_tostring(L,2);
        r = luawav_init_file(L,u,filename);
    } else if(lua_istable(L,2)) {
        lua_getfield(L,2,"filename");
        if(!lua_isnil(L,-1)) {
            filename = lua_tostring(L,-1);
            lua_pop(L,1);
            r = luawav_init_file(L,u,filename);
        }
        lua_pop(L,1);
        r = luawav_init_stream(L,u);
    } else {
        return luaL_error(L,"invalid parameters");
    }

    if(!r) {
        lua_pushboolean(L,0);
    } else {
        luawav_push_fmt(L,&u->wav.fmt);
    }
    return 1;

}


static int
luawav_read_pcm_frames_f32(lua_State *L) {
    luawav_userdata *u = NULL;
    drwav_uint64 framesToRead = 0;
    drwav_uint64 r = 0;
    drwav_uint64 t = 0;
    drwav_uint64 n = 0;
    drwav_uint64 i = 0;

    u = luaL_checkudata(L,1,luawav_mt);
    framesToRead = luawav_touint64(L,2);

    lua_createtable(L,framesToRead * u->wav.channels,0);

    while(r<framesToRead) {
        n = WAV_MIN( framesToRead - r, F32_BUFFER / u->wav.channels);
        t = drwav_read_pcm_frames_f32(&u->wav,n,u->pcm_float);
        i = 0;
        while(i<(t * u->wav.channels)) {
            lua_pushnumber(L,u->pcm_float[i]);
            lua_rawseti(L,-2,++i + r);
        }
        if(n != t) break;
        r += t;
    }

    return 1;
}

static int
luawav_read_pcm_frames_s32(lua_State *L) {
    luawav_userdata *u = NULL;
    drwav_uint64 framesToRead = 0;
    drwav_uint64 r = 0;
    drwav_uint64 t = 0;
    drwav_uint64 n = 0;
    drwav_uint64 i = 0;

    u = luaL_checkudata(L,1,luawav_mt);
    framesToRead = luawav_touint64(L,2);

    lua_createtable(L,framesToRead * u->wav.channels,0);

    while(r<framesToRead) {
        n = WAV_MIN( framesToRead - r, S32_BUFFER / u->wav.channels);
        t = drwav_read_pcm_frames_s32(&u->wav,n,u->pcm_int32);
        i = 0;
        while(i<(t * u->wav.channels)) {
            lua_pushinteger(L,u->pcm_int32[i]);
            lua_rawseti(L,-2,++i + r);
        }
        if(n != t) break;
        r += t;
    }

    return 1;
}

static int
luawav_read_pcm_frames_s16(lua_State *L) {
    luawav_userdata *u = NULL;
    drwav_uint64 framesToRead = 0;
    drwav_uint64 r = 0;
    drwav_uint64 t = 0;
    drwav_uint64 n = 0;
    drwav_uint64 i = 0;

    u = luaL_checkudata(L,1,luawav_mt);
    framesToRead = luawav_touint64(L,2);

    lua_createtable(L,framesToRead * u->wav.channels,0);

    while(r<framesToRead) {
        n = WAV_MIN( framesToRead - r, S16_BUFFER / u->wav.channels);
        t = drwav_read_pcm_frames_s16(&u->wav,n,u->pcm_int16);
        i = 0;
        while(i<(t * u->wav.channels)) {
            lua_pushinteger(L,u->pcm_int16[i]);
            lua_rawseti(L,-2,++i + r);
        }
        if(n != t) break;
        r += t;
    }

    return 1;
}


static int
luawav_write_pcm_frames(lua_State *L) {
    luawav_userdata *u = NULL;
    u = luaL_checkudata(L,1,luawav_mt);
    return u->write(L,u);
}

static int
luawav_version(lua_State *L) {
    drwav_uint32 major;
    drwav_uint32 minor;
    drwav_uint32 revis;
    drwav_version(&major,&minor,&revis);
    lua_newtable(L);
    lua_pushinteger(L,major);
    lua_setfield(L,-2,"major");
    lua_pushinteger(L,minor);
    lua_setfield(L,-2,"minor");
    lua_pushinteger(L,revis);
    lua_setfield(L,-2,"revision");
    return 1;
}

static int
luawav_version_string(lua_State *L) {
    lua_pushstring(L,drwav_version_string());
    return 1;
}

static void
luawav_push_f32_samples(lua_State *L, float *samples, drwav_uint64 sampleCount) {
    drwav_uint64 i = 0;
    lua_createtable(L,sampleCount,0);
    while(i<sampleCount) {
        lua_pushnumber(L,samples[i]);
        lua_rawseti(L,-2,++i);
    }
}

static void
luawav_push_s32_samples(lua_State *L, drwav_int32 *samples, drwav_uint64 sampleCount) {
    drwav_uint64 i = 0;
    lua_createtable(L,sampleCount,0);
    while(i<sampleCount) {
        lua_pushinteger(L,samples[i]);
        lua_rawseti(L,-2,++i);
    }
}

static void
luawav_push_s16_samples(lua_State *L, drwav_int16 *samples, drwav_uint64 sampleCount) {
    drwav_uint64 i = 0;
    lua_createtable(L,sampleCount,0);
    while(i<sampleCount) {
        lua_pushinteger(L,samples[i]);
        lua_rawseti(L,-2,++i);
    }
}

typedef void *(*luawav_open_and_read_func)(drwav_read_proc onRead, drwav_seek_proc onSeek, void* pUserData, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut, const drwav_allocation_callbacks* pAllocationCallbacks);
typedef void *(*luawav_open_and_read_file_func)(const char *filename, unsigned int* channelsOut, unsigned int* sampleRateOut, drwav_uint64* totalFrameCountOut, const drwav_allocation_callbacks* pAllocationCallbacks);
typedef void (*luawav_push_samples_func)(lua_State *L, void *samples, drwav_uint64 sampleCount);

static int
luawav_open_and_read_pcm_frames(lua_State *L) {
    const char *filename = NULL;
    unsigned int channels = 0;
    unsigned int sampleRate = 0;
    drwav_uint64 frameCount = 0;
    void *samples = NULL;
    luawav_stream_userdata u;
    luawav_open_and_read_func f = NULL;
    luawav_open_and_read_file_func file_f = NULL;
    luawav_push_samples_func push = NULL;

    if(lua_isstring(L,1)) {
        filename = lua_tostring(L,1);
    } else if(!lua_istable(L,1)) {
        return luaL_error(L,"missing required parameter: filename or table");
    }

    f = lua_touserdata(L,lua_upvalueindex(1));
    file_f = lua_touserdata(L,lua_upvalueindex(2));
    push = lua_touserdata(L,lua_upvalueindex(3));

    if(filename == NULL) {
        u.L = L;
        lua_newtable(L);

        lua_getfield(L,1,"onRead");
        if(lua_isnil(L,-1)) {
            return luaL_error(L,"missing required onRead function");
        }
        lua_setfield(L,-2,"onRead");

        lua_getfield(L,1,"onSeek");
        if(lua_isnil(L,-1)) {
            return luaL_error(L,"missing required onSeek function");
        }
        lua_setfield(L,-2,"onSeek");

        lua_getfield(L,1,"userData");
        lua_setfield(L,-2,"userData");

        u.table_ref = luaL_ref(L,LUA_REGISTRYINDEX);
        samples = f(
          luawav_read_proc,
          luawav_seek_proc,
          &u,
          &channels,
          &sampleRate,
          &frameCount,
          NULL);
        luaL_unref(L,LUA_REGISTRYINDEX,u.table_ref);
    } else {
        samples = file_f(
          filename,
          &channels,
          &sampleRate,
          &frameCount,
          NULL);
    }

    if(samples) {
        lua_newtable(L);
        lua_pushinteger(L,channels);
        lua_setfield(L,-2,"channels");
        lua_pushinteger(L,sampleRate);
        lua_setfield(L,-2,"sampleRate");
        luawav_pushuint64(L,frameCount);
        lua_setfield(L,-2,"frameCount");
        push(L, samples, channels * frameCount);
        lua_setfield(L,-2,"samples");
        drwav_free(samples, NULL);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static const struct luaL_Reg luawav_functions[] = {
    { "drwav", luawav_drwav },
    { "drwav_version", luawav_version },
    { "drwav_version_string", luawav_version_string },
    { "drwav_uninit", luawav_uninit },
    { "drwav_init", luawav_init },
    { "drwav_init_write", luawav_init_write },
    { "drwav_read_pcm_frames_f32", luawav_read_pcm_frames_f32 },
    { "drwav_read_pcm_frames_s32", luawav_read_pcm_frames_s32 },
    { "drwav_read_pcm_frames_s16", luawav_read_pcm_frames_s16 },
    { "drwav_write_pcm_frames", luawav_write_pcm_frames },
    { NULL, NULL },
};

static const luawav_metamethods luawav_mm[] = {
    { "drwav_uninit", "uninit" },
    { "drwav_init", "init" },
    { "drwav_init_write", "init_write" },
    { "drwav_read_pcm_frames_f32", "read_pcm_frames_f32" },
    { "drwav_read_pcm_frames_s32", "read_pcm_frames_s32" },
    { "drwav_read_pcm_frames_s16", "read_pcm_frames_s16" },
    { "drwav_write_pcm_frames", "write_pcm_frames" },
    { NULL, NULL },
};

static const luawav_const luawav_consts[] = {
    LUAWAV_CONST(DRWAV_SUCCESS),
    LUAWAV_CONST(DRWAV_ERROR),
    LUAWAV_CONST(DRWAV_INVALID_ARGS),
    LUAWAV_CONST(DRWAV_INVALID_OPERATION),
    LUAWAV_CONST(DRWAV_OUT_OF_MEMORY),
    LUAWAV_CONST(DRWAV_OUT_OF_RANGE),
    LUAWAV_CONST(DRWAV_ACCESS_DENIED),
    LUAWAV_CONST(DRWAV_DOES_NOT_EXIST),
    LUAWAV_CONST(DRWAV_ALREADY_EXISTS),
    LUAWAV_CONST(DRWAV_TOO_MANY_OPEN_FILES),
    LUAWAV_CONST(DRWAV_INVALID_FILE),
    LUAWAV_CONST(DRWAV_TOO_BIG),
    LUAWAV_CONST(DRWAV_PATH_TOO_LONG),
    LUAWAV_CONST(DRWAV_NAME_TOO_LONG),
    LUAWAV_CONST(DRWAV_NOT_DIRECTORY),
    LUAWAV_CONST(DRWAV_IS_DIRECTORY),
    LUAWAV_CONST(DRWAV_DIRECTORY_NOT_EMPTY),
    LUAWAV_CONST(DRWAV_END_OF_FILE),
    LUAWAV_CONST(DRWAV_NO_SPACE),
    LUAWAV_CONST(DRWAV_BUSY),
    LUAWAV_CONST(DRWAV_IO_ERROR),
    LUAWAV_CONST(DRWAV_INTERRUPT),
    LUAWAV_CONST(DRWAV_UNAVAILABLE),
    LUAWAV_CONST(DRWAV_ALREADY_IN_USE),
    LUAWAV_CONST(DRWAV_BAD_ADDRESS),
    LUAWAV_CONST(DRWAV_BAD_SEEK),
    LUAWAV_CONST(DRWAV_BAD_PIPE),
    LUAWAV_CONST(DRWAV_DEADLOCK),
    LUAWAV_CONST(DRWAV_TOO_MANY_LINKS),
    LUAWAV_CONST(DRWAV_NOT_IMPLEMENTED),
    LUAWAV_CONST(DRWAV_NO_MESSAGE),
    LUAWAV_CONST(DRWAV_BAD_MESSAGE),
    LUAWAV_CONST(DRWAV_NO_DATA_AVAILABLE),
    LUAWAV_CONST(DRWAV_INVALID_DATA),
    LUAWAV_CONST(DRWAV_TIMEOUT),
    LUAWAV_CONST(DRWAV_NO_NETWORK),
    LUAWAV_CONST(DRWAV_NOT_UNIQUE),
    LUAWAV_CONST(DRWAV_NOT_SOCKET),
    LUAWAV_CONST(DRWAV_NO_ADDRESS),
    LUAWAV_CONST(DRWAV_BAD_PROTOCOL),
    LUAWAV_CONST(DRWAV_PROTOCOL_UNAVAILABLE),
    LUAWAV_CONST(DRWAV_PROTOCOL_NOT_SUPPORTED),
    LUAWAV_CONST(DRWAV_PROTOCOL_FAMILY_NOT_SUPPORTED),
    LUAWAV_CONST(DRWAV_ADDRESS_FAMILY_NOT_SUPPORTED),
    LUAWAV_CONST(DRWAV_SOCKET_NOT_SUPPORTED),
    LUAWAV_CONST(DRWAV_CONNECTION_RESET),
    LUAWAV_CONST(DRWAV_ALREADY_CONNECTED),
    LUAWAV_CONST(DRWAV_NOT_CONNECTED),
    LUAWAV_CONST(DRWAV_CONNECTION_REFUSED),
    LUAWAV_CONST(DRWAV_NO_HOST),
    LUAWAV_CONST(DRWAV_IN_PROGRESS),
    LUAWAV_CONST(DRWAV_CANCELLED),
    LUAWAV_CONST(DRWAV_MEMORY_ALREADY_MAPPED),
    LUAWAV_CONST(DRWAV_AT_END),
    LUAWAV_CONST(DR_WAVE_FORMAT_PCM),
    LUAWAV_CONST(DR_WAVE_FORMAT_ADPCM),
    LUAWAV_CONST(DR_WAVE_FORMAT_IEEE_FLOAT),
    LUAWAV_CONST(DR_WAVE_FORMAT_ALAW),
    LUAWAV_CONST(DR_WAVE_FORMAT_MULAW),
    LUAWAV_CONST(DR_WAVE_FORMAT_DVI_ADPCM),
    LUAWAV_CONST(DR_WAVE_FORMAT_EXTENSIBLE),
    LUAWAV_CONST(DRWAV_MAX_SMPL_LOOPS),
    LUAWAV_CONST(DRWAV_SEQUENTIAL),
    LUAWAV_CONST(drwav_seek_origin_start),
    LUAWAV_CONST(drwav_seek_origin_current),
    LUAWAV_CONST(drwav_container_riff),
    LUAWAV_CONST(drwav_container_w64),
    LUAWAV_CONST(drwav_container_rf64),
    { NULL, 0 },
};

LUAWAV_PUBLIC
int luaopen_luawav(lua_State *L) {
    const luawav_metamethods *mm   = luawav_mm;
    const luawav_const *cc   = luawav_consts;
    lua_newtable(L);

    copydown(L,"luawav.version");

    lua_getglobal(L,"require");
    lua_pushstring(L,"luawav.int64");
    lua_call(L,1,1);
    lua_setfield(L,-2,"drwav_int64");

    lua_getglobal(L,"require");
    lua_pushstring(L,"luawav.uint64");
    lua_call(L,1,1);
    lua_setfield(L,-2,"drwav_uint64");

    luaL_setfuncs(L,luawav_functions,0);

    luaL_newmetatable(L,luawav_mt);
    lua_getfield(L,-2,"drwav_uninit");
    lua_setfield(L,-2,"__gc");
    lua_newtable(L); /* __index */
    while(mm->name != NULL) {
        lua_getfield(L,-3,mm->name);
        lua_setfield(L,-2,mm->metaname);
        mm++;
    }
    lua_setfield(L,-2,"__index");
    lua_pop(L,1);

    while(cc->name != NULL) {
        lua_pushinteger(L,cc->value);
        lua_setfield(L,-2,cc->name);
        cc++;
    }

    lua_pushlightuserdata(L,drwav_open_and_read_pcm_frames_s16);
    lua_pushlightuserdata(L,drwav_open_file_and_read_pcm_frames_s16);
    lua_pushlightuserdata(L,luawav_push_s16_samples);
    lua_pushcclosure(L,luawav_open_and_read_pcm_frames,3);
    lua_setfield(L,-2,"drwav_open_and_read_pcm_frames_s16");

    lua_pushlightuserdata(L,drwav_open_and_read_pcm_frames_s32);
    lua_pushlightuserdata(L,drwav_open_file_and_read_pcm_frames_s32);
    lua_pushlightuserdata(L,luawav_push_s32_samples);
    lua_pushcclosure(L,luawav_open_and_read_pcm_frames,3);
    lua_setfield(L,-2,"drwav_open_and_read_pcm_frames_s32");

    lua_pushlightuserdata(L,drwav_open_and_read_pcm_frames_f32);
    lua_pushlightuserdata(L,drwav_open_file_and_read_pcm_frames_f32);
    lua_pushlightuserdata(L,luawav_push_f32_samples);
    lua_pushcclosure(L,luawav_open_and_read_pcm_frames,3);
    lua_setfield(L,-2,"drwav_open_and_read_pcm_frames_f32");

    return 1;
}
