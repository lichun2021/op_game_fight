//
//  AIScriptBindSkillController.cpp
//  op_game
//
//  Created by h7ing on 29/08/2017.
//
//

#include "AIScriptBindSkillController.hpp"
#include "lua.hpp"
#include "AIScriptBindHelper.h"
#include "AIScriptBindCharacterData.hpp"
#include "../action_hero/HeroSkillController.h"

namespace OnePiece {

// MARK: -- luabind specialskillinfo

static inline const char *luatypename_specialskillinfo() { return "aispecialskillinfo"; }

void luastack_pushspecialskillinfo(lua_State *L, SpecialSkillInfo *info)
{
    size_t sz = sizeof(SpecialSkillInfo *);
    SpecialSkillInfo **the_fulluserdata = static_cast<SpecialSkillInfo **>(lua_newuserdata(L, sz));
    *the_fulluserdata = info;
    
    luaL_getmetatable(L, luatypename_specialskillinfo());
    lua_setmetatable(L, -2);
}

SpecialSkillInfo *luastack_getspecialskillinfo(lua_State *L, int idx)
{
    if (lua_isnil(L, idx)) return nullptr;
    SpecialSkillInfo **lua_userdata = static_cast<SpecialSkillInfo **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luabind_specialskillinfo_get_skillid(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    lua_pushinteger(L, info->skillID);
    return 1;
}

static int luabind_specialskillinfo_get_skillidindex(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    lua_pushinteger(L, info->skillIDIndex);
    return 1;
}

static int luabind_specialskillinfo_get_skilllevel(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    lua_pushinteger(L, info->skillLevel);
    return 1;
}

static int luabind_specialskillinfo_get_releasecount(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    lua_pushinteger(L, info->releaseCount);
    return 1;
}

static int luabind_specialskillinfo_can_cast(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    auto character = luastack_getcharacterdata(L, 2);
    lua_pushboolean(L, info->canCast(character));
    return 1;
}

static int luabind_specialskillinfo_on_condition(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    info->onConditon();
    return 0;
}

static int luabind_specialskillinfo_on_cast(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    info->onCast();
    return 0;
}

static int luabind_specialskillinfo_reset_condition(lua_State *L)
{
    auto info = luastack_getspecialskillinfo(L, 1);
    info->resetCondition();
    return 0;
}

int luaopen_specialskillinfo(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { NULL, NULL }
    };
    
    static const luaL_Reg reg_m[] = {
        { "get_skillid", luabind_specialskillinfo_get_skillid },
        { "get_skillidindex", luabind_specialskillinfo_get_skillidindex },
        { "get_skilllevel", luabind_specialskillinfo_get_skilllevel },
        { "get_releasecount", luabind_specialskillinfo_get_releasecount },
        { "can_cast", luabind_specialskillinfo_can_cast },
        { "on_condition", luabind_specialskillinfo_on_condition },
        { "on_cast", luabind_specialskillinfo_on_cast },
        { "reset_condition", luabind_specialskillinfo_reset_condition },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, luatypename_specialskillinfo());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}

// MARK: luabind specialskillinfo --

// MARK: -- luabind skillcontroller

static int luabind_skillcontroller_is_targetignoresilencebyskillid(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    bool luaret = HeroSkillController::isTargetIgnoreSilenceBySkill(*AIContext::getInstance(), c, skillid);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_skillcontroller_is_targetignoresilencebyskillindex(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillindex = lua_tointeger(L, 2);
    bool luaret = HeroSkillController::isTargetIgnoreSilenceBySkillIndex(*AIContext::getInstance(), character, skillindex);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_skillcontroller_calculate_skilllength(lua_State *L)
{
    int skillid = lua_tointeger(L, 1);
    int luaret = HeroSkillController::calculateSkillLength(skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_skillcontroller_calculate_accu_skill_length(lua_State *L)
{
    int skillid = lua_tointeger(L, 1);
    int altaccutick = lua_tointeger(L, 2);
    int luaret = HeroSkillController::calculateAccuSkillLength(skillid, altaccutick);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_skillcontroller_calculate_repeat_skill_first_round_tick_length(lua_State *L)
{
    int skillid = lua_tointeger(L, 1);
    int luaret = HeroSkillController::calculateRepeatSkillFirstRoundTickLength(skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_skillcontroller_calculate_energy_skill_first_round_tick_length(lua_State *L)
{
    int skillid = lua_tointeger(L, 1);
    int luaret = HeroSkillController::calculateEnergySkillFirstRoundTickLength(skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_skillcontroller_calculate_accumulate_skill_release_length(lua_State *L)
{
    int skillid = lua_tointeger(L, 1);
    int luaret = HeroSkillController::calculateAccumulateSkillReleaseLength(skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_skillcontroller_calculate_exec_length(lua_State *L)
{
    int execid = lua_tointeger(L, 1);
    lua_pushinteger(L, HeroSkillController::calculateExecLength(execid));
    return 1;
}

static int luabind_skillcontroller_get_repeatskillmaxrepeattick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int luaret = HeroSkillController::getRepeatSkillMaxRepeatTick(character, skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_skillcontroller_execute_execidintick(lua_State *L)
{
    int skillid = lua_tointeger(L, 1);
    int skilllv = lua_tointeger(L, 2);
    int tick = lua_tointeger(L, 3);
    auto c = luastack_getcharacterdata(L, 4);
    auto t = luastack_getcharacterdata(L, 5);
    HeroSkillController::executeExecIDInTick(*AIContext::getInstance(), skillid, skilllv, tick, *c, *t);
    return 0;
}

int luaopen_aiskillcontroller(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "is_targetignoresilencebyskillid", luabind_skillcontroller_is_targetignoresilencebyskillid },
        { "is_targetignoresilencebyskillindex", luabind_skillcontroller_is_targetignoresilencebyskillindex },
        { "calculate_skilllength", luabind_skillcontroller_calculate_skilllength },
        { "calculate_accu_skill_length", luabind_skillcontroller_calculate_accu_skill_length },
        { "calculate_repeat_skill_first_round_tick_length", luabind_skillcontroller_calculate_repeat_skill_first_round_tick_length },
        { "calculate_energy_skill_first_round_tick_length", luabind_skillcontroller_calculate_energy_skill_first_round_tick_length },
        { "calculate_accumulate_skill_release_length", luabind_skillcontroller_calculate_accumulate_skill_release_length },
        { "calculate_exec_length", luabind_skillcontroller_calculate_exec_length },
        { "get_repeatskillmaxrepeattick", luabind_skillcontroller_get_repeatskillmaxrepeattick },
        { "execute_execidintick", luabind_skillcontroller_execute_execidintick },
        { NULL, NULL }
    };
    
//    static const luaL_Reg reg_m[] = {
//        { NULL, NULL }
//    };
    
    luaL_newmetatable(L, "aiskillcontroller");  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    // luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}

// MARK: luabind skillcontroller --

void luahelper_skillcontroller_pickskill(lua_State *L, CharacterData *character, SkillSeqInfo *skillseqinfo)
{
    lua_getglobal(L, "g_actionhelper");
    lua_getfield(L, -1, "pickskill");
    luastack_pushcharacterdata(L, character);
    if (lua_pcall(L, 1, 4, 0))
    {
        luahelper_errorlog(L);
        lua_settop(L, 0);
        return;
    }

    skillseqinfo->skillIDIndex = lua_tointeger(L, -4);
    skillseqinfo->attackSeqIndex = lua_tointeger(L, -3);
    skillseqinfo->specialSkillID = lua_tointeger(L, -2);
    skillseqinfo->specialSkillCastContiond = lua_tointeger(L, -1);

    lua_settop(L, 0);
}

}
