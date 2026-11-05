//
//  AIScriptBindSummonCaster.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindSummonCaster.hpp"
#include "lua.hpp"
#if LUA_VERSION_NUM < 502
#include "compat-5.3.h"
#endif
#include "AIScriptBindHelper.h"
#include "AIScriptBindCharacterData.hpp"
#include "../AIContinuousSummonCaster.h"
#include "../CommonType.h"
#include "../AIContext.h"

namespace OnePiece {

static inline const char *luatypename_summoncaster() { return "aisummoncaster"; }

void luastack_pushsummoncaster(lua_State *L, AIContinuousSummonCaster *caster)
{
    size_t sz = sizeof(AIContinuousSummonCaster *);
    AIContinuousSummonCaster **the_fulluserdata = static_cast<AIContinuousSummonCaster **>(lua_newuserdata(L, sz));
    *the_fulluserdata = caster;

    luaL_getmetatable(L, luatypename_summoncaster());
    lua_setmetatable(L, -2);
}

AIContinuousSummonCaster *luastack_getsummoncaster(lua_State *L, int idx)
{
    if (lua_isnil(L, idx))
        return nullptr;
    AIContinuousSummonCaster **lua_userdata = static_cast<AIContinuousSummonCaster **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luacallback_can_hurtsummon(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    int hurtid = lua_tointeger(L, 2);
    auto target = luastack_getcharacterdata(L, 3);
    int skillid = lua_tointeger(L, 4);
    bool luaval = AIContinuousSummonCaster::couldHurtSummon(source, AIDataHelper::getHurtExec(hurtid), target, skillid);
    lua_pushboolean(L, luaval);
    return 1;
}

static int luabind_summoncaster_check_execute_deadrun(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int condition = lua_tointeger(L, 2);
    AIContinuousSummonCaster::checkExcuteDeadRun(*AIContext::getInstance(), character, condition);
    return 0;
}

static int luabind_summoncaster_init_summonpool(lua_State *L)
{
    AIContinuousSummonCaster *caster = luastack_getsummoncaster(L, 1);
    CharacterData *summoner = luastack_getcharacterdata(L, 2);
    int summon_id = lua_tointeger(L, 3);
    int pool_type = lua_tointeger(L, 4);
    AIContinuousSummonCaster::initSummonPool(caster, summoner, summon_id, pool_type);
    return 0;
}

static int luabind_summoncaster_get_group_summon_available_count(lua_State *L)
{
    int caster_myid = lua_tointeger(L, 1);
    CharacterData *c = luastack_getcharacterdata(L, 2);
    int exec_id = lua_tointeger(L, 3);
    int ret = AIContinuousSummonCaster::getGroupSummonAvailableCount(caster_myid, c, exec_id);
    lua_pushinteger(L, ret);
    return 1;
}

static int luabind_summoncaster_sleep_group_summon(lua_State *L)
{
    int group_id = lua_tointeger(L, 1);
    int group_weight = lua_tointeger(L, 2);
    CharacterData *target = luastack_getcharacterdata(L, 3);
    int caster_myid = lua_tointeger(L, 4);
    int to_sleep_group_num = lua_tointeger(L, 5);
    AIContinuousSummonCaster::sleepGroupSummon(group_id, group_weight, target, caster_myid, to_sleep_group_num);
    return 0;
}

static int luabind_summoncaster_get_myid(lua_State *L)
{
    AIContinuousSummonCaster *caster = luastack_getsummoncaster(L, 1);
    lua_pushinteger(L, caster->myID);
    return 1;
}

static int luabind_summoncaster_get_execid(lua_State *L)
{
    AIContinuousSummonCaster *caster = luastack_getsummoncaster(L, 1);
    lua_pushinteger(L, caster->summonExec->ID);
    return 1;
}

static int luabind_summoncaster_get_summonid(lua_State *L)
{
    AIContinuousSummonCaster *caster = luastack_getsummoncaster(L, 1);
    lua_pushinteger(L, caster->getSummonID());
    return 1;
}

static int luabind_summoncaster_get_active_count(lua_State *L)
{
    AIContinuousSummonCaster *caster = luastack_getsummoncaster(L, 1);
    lua_pushinteger(L, caster->getActiveCount());
    return 1;
}

static int luabind_summoncaster_remove_summon(lua_State *L)
{
    AIContinuousSummonCaster *caster = luastack_getsummoncaster(L, 1);
    int summon_id = lua_tointeger(L, 2);
    int num = lua_tointeger(L, 3);
    int ret = caster->removeSummon(summon_id, num);
    lua_pushinteger(L, ret);
    return 1;
}

int luaopen_summoncaster(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "can_hurtsummon", luacallback_can_hurtsummon },
        { "check_execute_deadrun", luabind_summoncaster_check_execute_deadrun },
        { "init_summonpool", luabind_summoncaster_init_summonpool },
        { "get_group_summon_available_count", luabind_summoncaster_get_group_summon_available_count },
        { "sleep_group_summon", luabind_summoncaster_sleep_group_summon },
        { NULL, NULL }
    };

    static const luaL_Reg reg_m[] = {
        { "get_myid", luabind_summoncaster_get_myid },
        { "get_execid", luabind_summoncaster_get_execid },
        { "get_summonid", luabind_summoncaster_get_summonid },
        { "get_active_count", luabind_summoncaster_get_active_count },
        { "remove_summon", luabind_summoncaster_remove_summon },
        { NULL, NULL }
    };

    luaL_newmetatable(L, luatypename_summoncaster());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}

void luahelper_summoncaster_init_summonpool(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summoner, int summon_id)
{
    lua_getglobal(L, "g_summonhelper");
    lua_getfield(L, -1, "init_summonpool");
    lua_remove(L, -2);
    luastack_pushsummoncaster(L, caster);
    luastack_pushcharacterdata(L, summoner);
    lua_pushinteger(L, summon_id);
    if (lua_pcall(L, 3, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

void luahelper_summoncaster_on_summon_awake(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summon, int summon_id)
{
    lua_getglobal(L, "g_summonhelper");
    lua_getfield(L, -1, "on_summon_awake");
    lua_remove(L, -2);
    luastack_pushsummoncaster(L, caster);
    luastack_pushcharacterdata(L, summon);
    lua_pushinteger(L, summon_id);
    if (lua_pcall(L, 3, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

int luahelper_summoncaster_get_group_summon_available_count(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summoner)
{
    lua_getglobal(L, "g_summonhelper");
    lua_getfield(L, -1, "get_group_summon_available_count");
    lua_remove(L, -2);

    lua_pushinteger(L, caster->myID);
    luastack_pushcharacterdata(L, summoner);
    lua_pushinteger(L, caster->summonExec->ID);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return -1;
    }

    int ret = lua_tointeger(L, -1);
    lua_settop(L, 0);
    return ret;
}

void luahelper_summoncaster_check_sleep_group_summon(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summoner, int exec_id, int group_id, int group_weight, int group_max_num, int current_group_summon_num)
{
    lua_getglobal(L, "g_summonhelper");
    lua_getfield(L, -1, "check_sleep_group_summon");
    lua_remove(L, -2);

    lua_pushinteger(L, caster->myID);
    luastack_pushcharacterdata(L, summoner);
    lua_pushinteger(L, exec_id);
    lua_pushinteger(L, group_id);
    lua_pushinteger(L, group_weight);
    lua_pushinteger(L, group_max_num);
    lua_pushinteger(L, current_group_summon_num);
    if (lua_pcall(L, 7, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

void luahelper_summoncaster_on_finish_awake_summon(lua_State *L, int caster_myid, CharacterData *summoner)
{
    lua_getglobal(L, "g_summonhelper");
    lua_getfield(L, -1, "on_finish_awake_summon");
    lua_remove(L, -2);

    lua_pushinteger(L, caster_myid);
    luastack_pushcharacterdata(L, summoner);
    if (lua_pcall(L, 2, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

}
