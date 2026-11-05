//
//  AIScriptBindMarkCaster.cpp
//  op_game
//
//  Created by h7ing on 04/09/2017.
//
//

#include "AIScriptBindMarkCaster.hpp"
#include "lua.hpp"
#if LUA_VERSION_NUM < 502
#include "compat-5.3.h"
#endif
#include "AIScriptBindCharacterData.hpp"
#include "../AIContext.h"
#include "../AICharacterData.h"
#include "../AIContinuousMarkCaster.h"

namespace OnePiece {
// MARK: -- luaopen markcaster
static inline const char *luatypename_markcaster() { return "aimarkcaster"; }

AIContinuousMarkCaster *luastack_getmarkcaster(lua_State *L, int idx)
{
    if (lua_isnil(L, idx)) return nullptr;
    AIContinuousMarkCaster **lua_userdata = static_cast<AIContinuousMarkCaster **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

void luastack_pushmarkcaster(lua_State *L, AIContinuousMarkCaster *m)
{
    size_t sz = sizeof(AIContinuousMarkCaster *);
    AIContinuousMarkCaster **the_fulluserdata = static_cast<AIContinuousMarkCaster **>(lua_newuserdata(L, sz));
    *the_fulluserdata = m;
    
    luaL_getmetatable(L, luatypename_markcaster());
    lua_setmetatable(L, -2);
}

static int luabind_markcaster_check_excute_effect(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skilllv = lua_tointeger(L, 3);
    int hurttype = lua_tointeger(L, 4);
    int hurtprop = lua_tointeger(L, 5);
    int calltype = lua_tointeger(L, 6);
    int hpdelta = lua_tointeger(L, 7);
    int hittype = lua_tointeger(L, 8);
    auto trigger = luastack_getcharacterdata(L, 9);
    AIContinuousMarkCaster::checkExcuteEffect(*AIContext::getInstance(), *c, skillid, skilllv, hurttype, hurtprop, calltype, hpdelta, hittype, trigger);
    return 0;
}

static int luabind_markcaster_check_excute_dead(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skilllv = lua_tointeger(L, 3);
    int hurttype = lua_tointeger(L, 4);
    int hurtprop = lua_tointeger(L, 5);
    auto trigger = luastack_getcharacterdata(L, 6);
    int hurtid = lua_tointeger(L, 7);
    AIContinuousMarkCaster::checkExcuteDead(*AIContext::getInstance(), *c, skillid, skilllv, hurttype, hurtprop, trigger, hurtid);
    return 0;
}

static int luabind_markcaster_check_excute_after_character_dead_release_trigger(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    AIContinuousMarkCaster::checkExcuteAfterCharcterDeadReleaseTrigger(character);
    return 0;
}

static int luabind_markcaster_check_do_character_dead(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    AIContinuousMarkCaster::checkDoCharacterDead(*AIContext::getInstance(), c);
    return 0;
}

static int luabind_markcaster_check_trigger_on_character_dead(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    AIContinuousMarkCaster::checkTriggerOnCharacterDead(*AIContext::getInstance(), c);
    return 0;
}

static int luabind_markcaster_get_myid(lua_State *L)
{
    auto m = luastack_getmarkcaster(L, 1);
    lua_pushinteger(L, m->myID);
    return 1;
}

int luaopen_markcaster(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "check_excute_effect", luabind_markcaster_check_excute_effect },
        { "check_excute_dead", luabind_markcaster_check_excute_dead },
        { "check_excute_after_character_dead_release_trigger", luabind_markcaster_check_excute_after_character_dead_release_trigger },
        { "check_do_character_dead", luabind_markcaster_check_do_character_dead },
        { "check_trigger_on_character_dead", luabind_markcaster_check_trigger_on_character_dead },
        { NULL, NULL }
    };
    
    static const luaL_Reg reg_m[] = {
        { "get_myid", luabind_markcaster_get_myid },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, luatypename_markcaster());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}

// MARK: luaopen markcaster --
}
