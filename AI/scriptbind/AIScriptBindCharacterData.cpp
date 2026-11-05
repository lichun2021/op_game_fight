//
//  AIScriptBindCharacterData.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindCharacterData.hpp"
#include "lua.hpp"
#include "AIScriptBindHelper.h"
#include "AIScriptBindEffectCaster.hpp"
#include "AIScriptBindSkillController.hpp"
#include "AIScriptBindProjectile.hpp"
#include "AIScriptBindSummonCaster.hpp"
#include "../AICharacterData.h"
#include "../AIContext.h"
#include "../AIProjectile.h"
#include "../action_hero/HeroActionController.h"

namespace OnePiece {

// MARK: -- Lua binding character
    
enum CharacterDataScriptBindOpCode {
    CD_OPCODE_INVALID = 0,
    CD_MYID = 1,
    CD_TOTALTICK = 2,
    CD_LOCALTICK = 3,
    CD_STARTTICK = 4,
    CD_K = 5,
    CD_TEAMSIDE = 6,
    CD_DIRECTION = 7,
    CD_ALTACCUTICK = 8,
    CD_CURRENTACTION = 9,
    CD_SEX = 10,
    CD_LASTHURTEXEC_INITHURTDELTA = 11,
    CD_LASTHURTEXEC_REALHURTDELTA = 12,
    CD_LASTHURTEXEC_ORIGINCHARACTER = 13,
    CD_NORMAL_SKILL_ID = 14,
    CD_LASTHURTEXEC_ID = 15,
    CD_KILLER = 16,
    CD_KILLER_SKILL_ID = 17,
    CD_ATTACKER = 18,
    CD_ENERGYADD_ONHIT = 19,
    CD_TRAVERSAL_STARTTICK = 20,
    CD_IS_EFF_181_RUNNING = 21,
    CD_SUMMON_PROP_ID = 22,
    CD_JUEXING_LEVEL = 23,

    CD_CURRENT_ENERGY_SKILL_ID,
    CD_CURRENT_ENERGY_SKILL_TICK,
    CD_CURRENT_REPEAT_SKILL_ID,
    CD_CURRENT_REPEAT_SKILL_TICK,
    CD_FLAG_FIRST_FOUND_26TARGET,
    CD_FIRST_ROUND_CONFIRM_26TARGET,
    CD_CURRENT_REPEAT_SKILL_MAX_REPEAT_TICK,
    CD_ATTACK_CONFIRMED_TARGET,
    CD_CAN_ULTRA_SKILL,
    CD_IF_ULTRA_LOCK,
    CD_CLUSTER_NUM,
    CD_ACCUMULATE_STATE,
    CD_ACTION_CAUSER_MYID,
    CD_STATIONARY_DIRECTION,
    CD_STATIONARY_X,
    CD_STATIONARY_Y,
    CD_HIT_FLY_EXTEND_TICK,
    CD_OLD_Y,
    CD_COMBAT_POWER,
    CD_TRANVERSAL_LAST_PICK_TICK,

    /// 13.0
    CD_CANNOTBEFOUND_BY_TEAMRELATIVETYPECNT,
    CD_INNEREFFECT_DEBUFF_COUNT,
    CD_INNEREFFECT_PROTECT_COUNT,
    ///

    CD_OPCODE_NUM
};

static inline const char *luatypename_characterdata() { return "aicharacterdata"; }
    
static int s_c_key = 0;
    
void luahelper_characterdata_reset_register(lua_State *L)
{
    lua_pushnil(L);
    lua_rawsetp(L, LUA_REGISTRYINDEX, &s_c_key);
}

//void luastack_pushcharacterdata(lua_State *L, CharacterData *source)
//{
//    size_t sz = sizeof(CharacterData *);
//    CharacterData **the_fulluserdata = static_cast<CharacterData **>(lua_newuserdata(L, sz));
//    *the_fulluserdata = source;
//
//    luaL_getmetatable(L, luatypename_characterdata());
//    lua_setmetatable(L, -2);
//}

void luastack_pushcharacterdata(lua_State *L, CharacterData *source)
{
//    int ltop = lua_gettop(L);
    lua_rawgetp(L, LUA_REGISTRYINDEX, &s_c_key);
    
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_rawsetp(L, LUA_REGISTRYINDEX, &s_c_key);
    }

    // stack: ..., table

    lua_rawgetp(L, -1, source);

    if (lua_isuserdata(L, -1)) {
        lua_remove(L, -2);

        // stack: ..., userdata
    } else {
        lua_pop(L, 1);

        size_t sz = sizeof(CharacterData *);
        CharacterData **the_fulluserdata = static_cast<CharacterData **>(lua_newuserdata(L, sz));
        *the_fulluserdata = source;

        luaL_getmetatable(L, luatypename_characterdata());
        lua_setmetatable(L, -2);

        lua_pushvalue(L, -1);

        // stack: ..., table, userdata, userdata
        lua_rawsetp(L, -3, source);

        lua_remove(L, -2);

        // stack: ..., userdata
    }

//    if (lua_gettop(L) - ltop != 1) {
//        luaL_error(L, "stack not balanced");
//    }
}

static int luabind_characterdata_new_ref(lua_State *L)
{
    size_t sz = sizeof(CharacterData *);
    
    if (lua_islightuserdata(L, -1)) {
        CharacterData *light_ptr = static_cast<CharacterData *>(lua_touserdata(L, -1));
        CharacterData **the_fulluserdata = static_cast<CharacterData **>(lua_newuserdata(L, sz));
        *the_fulluserdata = light_ptr;
        
        luaL_getmetatable(L, luatypename_characterdata());
        lua_setmetatable(L, -2);
    }
    // TODO: lua stack error
    return 1;
}

CharacterData *luastack_getcharacterdata(lua_State *L, int idx)
{
    if (lua_isnil(L, idx))
        return nullptr;
    CharacterData **lua_userdata = static_cast<CharacterData **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luabind_characterdata_get_myid(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->myID);
    return 1;
}

static int luabind_characterdata_get_attr(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_attr = lua_tointeger(L, -1);
    int retval = 0;
    if (0 <= the_attr && the_attr < ATT_NUM)
        retval = the_character->attributes[the_attr];
    lua_pushinteger(L, retval);
    return 1;
}

static int luabind_characterdata_set_attr(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_attr = lua_tointeger(L, -2);
    int the_val = lua_tointeger(L, -1);
    if (0 <= the_attr && the_attr < ATT_NUM)
        the_character->attributes[the_attr] = the_val;
    return 0;
}

static int luabind_characterdata_get_attr_origin(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int idx = lua_tointeger(L, -1);
    int retval = CharacterFunc::GetCharacterAttr(c->attributesOrigin.get(), idx);
    lua_pushinteger(L, retval);
    return 1;
}

static int luabind_characterdata_get_attr_afterbegin(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int idx = lua_tointeger(L, -1);
    int retval = CharacterFunc::GetCharacterAttr(c->attributesAfterBegin.get(), idx);
    lua_pushinteger(L, retval);
    return 1;
}

static int luabind_characterdata_get_actionatt(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int attidx = lua_tointeger(L, 2);
    int luaret = 0;
    if (0 <= attidx && attidx < ACTION_ATT_NUM)
        luaret = character->actionAtt[attidx];
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_characterdata_set_actionatt(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int attidx = lua_tointeger(L, 2);
    int luaval = lua_tointeger(L, 3);
    if (0 <= attidx && attidx < ACTION_ATT_NUM)
        character->actionAtt[attidx] = luaval;
    return 0;
}

static void luahelper_character_val_getter(lua_State *L, CharacterData *c, int opcode)
{
    switch (opcode)
    {
    case CD_MYID: 
        lua_pushinteger(L, c->myID); 
        break;
    case CD_K: 
        lua_pushinteger(L, c->k); 
        break;
    case CD_TEAMSIDE: 
        lua_pushinteger(L, c->teamSide); 
        break;
    case CD_TOTALTICK: 
        lua_pushinteger(L, c->totalTick); 
        break;
    case CD_LOCALTICK: 
        lua_pushinteger(L, c->localTick); 
        break;
    case CD_STARTTICK: 
        lua_pushinteger(L, c->startTick); 
        break;
    case CD_DIRECTION: 
        lua_pushinteger(L, c->direction); 
        break;
    case CD_ALTACCUTICK: 
        lua_pushinteger(L, c->getAltAccuTick()); 
        break;
    case CD_CURRENTACTION: 
        lua_pushinteger(L, c->currentAction); 
        break;
    case CD_SEX: 
        lua_pushinteger(L, c->getSex()); 
        break;
    case CD_NORMAL_SKILL_ID: 
        lua_pushinteger(L, c->normalSkill.first); 
        break;
    case CD_ENERGYADD_ONHIT: 
        lua_pushinteger(L, c->_beHittedAddEnergy); 
        break;
    case CD_TRAVERSAL_STARTTICK: 
        lua_pushinteger(L, c->getTraversalStartTick()); 
        break;
    case CD_IS_EFF_181_RUNNING: 
        lua_pushboolean(L, c->isCritNoFailEffRunning); 
        break;
    case CD_SUMMON_PROP_ID:
        if (c->summonProperty) 
            lua_pushinteger(L, c->summonProperty->summonId); 
        else 
            lua_pushinteger(L, 0); 
        break;
    case CD_JUEXING_LEVEL: 
        lua_pushinteger(L, c->juexingLevel); 
        break;
    case CD_CURRENT_ENERGY_SKILL_ID: 
        lua_pushinteger(L, c->currentEnergySkillId); 
        break;
    case CD_CURRENT_ENERGY_SKILL_TICK: 
        lua_pushinteger(L, c->currentEnergySkillTick); 
        break;
    case CD_CURRENT_REPEAT_SKILL_ID: 
        lua_pushinteger(L, c->currentRepeatSkillId); 
        break;
    case CD_CURRENT_REPEAT_SKILL_TICK: 
        lua_pushinteger(L, c->currentRepeatSkillTick);
        break;
    case CD_FLAG_FIRST_FOUND_26TARGET: 
        lua_pushboolean(L, c->GetFlagFirstFound26Target()); 
        break;
    case CD_FIRST_ROUND_CONFIRM_26TARGET: {
            auto tt = c->GetFirstRoundConfirm26Target();
            if (tt) 
                luastack_pushcharacterdata(L, tt); 
            else 
                lua_pushnil(L);
        } 
        break;
    case CD_CURRENT_REPEAT_SKILL_MAX_REPEAT_TICK: 
        lua_pushinteger(L, c->currentRepeatSkillMaxRepeatTick); 
        break;
    case CD_ATTACK_CONFIRMED_TARGET: 
        lua_pushboolean(L, c->mAttackConfirmedTarget); 
        break;
    case CD_CAN_ULTRA_SKILL: 
        lua_pushinteger(L, c->canUltraSkill); 
        break;
    case CD_IF_ULTRA_LOCK: 
        lua_pushinteger(L, c->ifUltraLock); 
        break;
    case CD_CLUSTER_NUM: 
        lua_pushinteger(L, c->clusterNum); 
        break;
    case CD_ACCUMULATE_STATE: 
        lua_pushinteger(L, (int)c->getAccumulateState()); 
        break;
    case CD_ACTION_CAUSER_MYID: 
        lua_pushinteger(L, c->actionCauserMyID); 
        break;
    case CD_STATIONARY_DIRECTION: 
        lua_pushinteger(L, c->mStationaryDirection); 
        break;
    case CD_STATIONARY_X: 
        lua_pushinteger(L, c->mStationaryX); 
        break;
    case CD_STATIONARY_Y: 
        lua_pushinteger(L, c->mStationaryY); 
        break;
    case CD_HIT_FLY_EXTEND_TICK: 
        lua_pushinteger(L, c->hitFlyExtendTick); 
        break;
    case CD_OLD_Y: 
        lua_pushinteger(L, c->oldY); 
        break;
    case CD_COMBAT_POWER: 
        lua_pushinteger(L, c->combatPower); 
        break;
    case CD_ATTACKER: 
        if (c->attacker) 
            luastack_pushcharacterdata(L, c->attacker);
        else 
            lua_pushnil(L); 
        break;
    case CD_TRANVERSAL_LAST_PICK_TICK: 
        lua_pushinteger(L, c->getTraversalLastPickTick()); 
        break;
    case CD_CANNOTBEFOUND_BY_TEAMRELATIVETYPECNT:
        lua_pushinteger(L, c->getCannotBeFoundByTeamRelativeTypeCnt(lua_tointeger(L, -1)));
        break;
    case CD_INNEREFFECT_DEBUFF_COUNT:
        lua_pushinteger(L, CharacterFunc::GetInnerEffectDebuffCount(c, lua_tointeger(L, -2), lua_tointeger(L, -1)));
        break;
    case CD_INNEREFFECT_PROTECT_COUNT:
        lua_pushinteger(L, CharacterFunc::GetInnerEffectProtectCount(c, lua_tointeger(L, -2), lua_tointeger(L, -1)));
        break;

    default: 
        CCLOG("ai:lua:opcode-%d-get not support", opcode); 
        lua_pushnil(L); 
        break;
    }
}

static void luahelper_character_val_setter(lua_State *L, CharacterData *c, int opcode)
{
    switch (opcode)
    {
    case CD_TEAMSIDE:
        c->teamSide = TeamSide(lua_tointeger(L, -1));
        break;
    case CD_CLUSTER_NUM:
        c->clusterNum = lua_tointeger(L, -1);
        break;
    case CD_TOTALTICK: 
        c->totalTick = lua_tointeger(L, -1); 
        break;
    case CD_DIRECTION: 
        c->direction = lua_tointeger(L, -1); 
        break;
    case CD_LASTHURTEXEC_INITHURTDELTA: 
        c->lastHurtExecInitHurtDelta = lua_tointeger(L, -1); 
        break;
    case CD_LASTHURTEXEC_REALHURTDELTA: 
        c->lastHurtExecRealHurtDelta = lua_tointeger(L, -1); 
        break;
    case CD_LASTHURTEXEC_ORIGINCHARACTER: 
        c->pLastHurtOriginCharacter = luastack_getcharacterdata(L, -1); 
        break;
    case CD_LASTHURTEXEC_ID: 
        c->lastHurtExecId = lua_tointeger(L, -1); 
        break;
    case CD_KILLER: 
        c->killerUID = luastack_getcharacterdata(L, -1)->myID; 
        break;
    case CD_KILLER_SKILL_ID: 
        c->killerSkillID = lua_tointeger(L, -1); 
        break;
    case CD_ATTACKER: 
        c->attacker = luastack_getcharacterdata(L, -1); 
        break;
    case CD_TRAVERSAL_STARTTICK: 
        c->setTraversalStartTick(lua_tointeger(L, -1)); 
        break;
    case CD_IS_EFF_181_RUNNING: 
        c->isCritNoFailEffRunning = lua_toboolean(L, -1); 
        break;
    case CD_CURRENT_ENERGY_SKILL_ID: 
        c->currentEnergySkillId = lua_tointeger(L, -1); 
        break;
    case CD_CURRENT_ENERGY_SKILL_TICK: 
        c->currentEnergySkillTick = lua_tointeger(L, -1); 
        break;
    case CD_CURRENT_REPEAT_SKILL_ID: 
        c->currentRepeatSkillId = lua_tointeger(L, -1); 
        break;
    case CD_CURRENT_REPEAT_SKILL_TICK: 
        c->currentRepeatSkillTick = lua_tointeger(L, -1); 
        break;
    case CD_FLAG_FIRST_FOUND_26TARGET: 
        c->SetFlagFirstFound26Target(lua_toboolean(L, -1)); 
        break;
    case CD_FIRST_ROUND_CONFIRM_26TARGET: 
        c->SetFirstRoundConfirm26Target(luastack_getcharacterdata(L, -1)); 
        break;
    case CD_CURRENT_REPEAT_SKILL_MAX_REPEAT_TICK: 
        c->currentRepeatSkillMaxRepeatTick = lua_tointeger(L, -1); 
        break;
    case CD_ATTACK_CONFIRMED_TARGET: 
        c->mAttackConfirmedTarget = lua_toboolean(L, -1); 
        break;
    case CD_CAN_ULTRA_SKILL: 
        c->canUltraSkill = lua_tointeger(L, -1); 
        break;
    case CD_ACTION_CAUSER_MYID: 
        c->actionCauserMyID = lua_tointeger(L, -1); 
        break;
    case CD_STATIONARY_DIRECTION: 
        c->mStationaryDirection = lua_tointeger(L, -1); 
        break;
    case CD_STATIONARY_X: 
        c->mStationaryX = lua_tointeger(L, -1); 
        break;
    case CD_STATIONARY_Y: 
        c->mStationaryY = lua_tointeger(L, -1); 
        break;
    case CD_HIT_FLY_EXTEND_TICK: 
        c->hitFlyExtendTick = lua_tointeger(L, -1); 
        break;
    case CD_OLD_Y: 
        c->oldY = lua_tointeger(L, -1); 
        break;
    case CD_COMBAT_POWER: 
        c->combatPower = lua_tointeger(L, -1); 
        break;
    case CD_CANNOTBEFOUND_BY_TEAMRELATIVETYPECNT:
        c->setCannotBeFoundByTeamRelativeTypeCnt(lua_tointeger(L, -2), lua_tointeger(L, -1));
        break;
    case CD_INNEREFFECT_DEBUFF_COUNT:
        CharacterFunc::SetInnerEffectDebuffCount(c, lua_tointeger(L, -3), lua_tointeger(L, -2), lua_tointeger(L, -1));
        break;
    case CD_INNEREFFECT_PROTECT_COUNT:
        CharacterFunc::SetInnerEffectProtectCount(c, lua_tointeger(L, -3), lua_tointeger(L, -2), lua_tointeger(L, -1));
        break;

    default: 
        CCLOG("ai:lua:opcode-%d-set not support", opcode);
        break;
    }
}

static int luabind_characterdata_get_data(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    if (lua_isnil(L, 2)) {
        CCLOG("ai:lua error:nil opcode");
        lua_pushnil(L);
        return 1;
    }

    int opcode = lua_tointeger(L, 2);
    luahelper_character_val_getter(L, c, opcode);
    return 1;
}

static int luabind_characterdata_set_data(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    if (lua_isnil(L, 2)) {
        CCLOG("ai:lua error:nil opcode");
        lua_pushnil(L);
        return 1;
    }
    int opcode = lua_tointeger(L, 2);
    luahelper_character_val_setter(L, c, opcode);
    return 0;
}

static std::unordered_map<std::string, int> luahelper_characterdata_val_keys = {
    { "myid", CD_MYID },
    { "totaltick", CD_TOTALTICK },
    { "localtick", CD_LOCALTICK },
    { "starttick", CD_STARTTICK },
    { "k", CD_K },
    { "teamside", CD_TEAMSIDE },
    { "direction", CD_DIRECTION },
    { "altaccutick", CD_ALTACCUTICK },
    { "currentaction", CD_CURRENTACTION },
    { "sex", CD_SEX },
    { "lasthurtexecinithurtdelta", CD_LASTHURTEXEC_INITHURTDELTA },
    { "lasthurtexecrealhurtdelta", CD_LASTHURTEXEC_REALHURTDELTA },
    { "lasthurtexecorigincharacter", CD_LASTHURTEXEC_ORIGINCHARACTER },
    { "normalskillid", CD_NORMAL_SKILL_ID },
    { "lasthurtexecid", CD_LASTHURTEXEC_ID },
    { "killer", CD_KILLER },
    { "killerskillid", CD_KILLER_SKILL_ID },
    { "energyaddonhit", CD_ENERGYADD_ONHIT },
    { "traversalstarttick", CD_TRAVERSAL_STARTTICK },
    { "iseff181running", CD_IS_EFF_181_RUNNING },
    { "summonpropid", CD_SUMMON_PROP_ID },
    { "currentenergyskillid", CD_CURRENT_ENERGY_SKILL_ID },
    { "currentenergyskilltick", CD_CURRENT_ENERGY_SKILL_TICK },
    { "currentrepeatskillid", CD_CURRENT_REPEAT_SKILL_ID },
    { "currentrepeatskilltick", CD_CURRENT_REPEAT_SKILL_TICK },
    { "flagfirstfound26target", CD_FLAG_FIRST_FOUND_26TARGET },
    { "firstroundconfirm26target", CD_FIRST_ROUND_CONFIRM_26TARGET },
    { "currentrepeatskillmaxrepeattick", CD_CURRENT_REPEAT_SKILL_MAX_REPEAT_TICK },
    { "mattackconfirmedtarget", CD_ATTACK_CONFIRMED_TARGET },
    { "canultraskill", CD_CAN_ULTRA_SKILL },
    { "juexinglevel", CD_JUEXING_LEVEL },
    { "ifultralock", CD_IF_ULTRA_LOCK },
    { "clusternum", CD_CLUSTER_NUM },
    { "accumulatestate", CD_ACCUMULATE_STATE },
    { "actioncausermyid", CD_ACTION_CAUSER_MYID },
    { "stationarydirection", CD_STATIONARY_DIRECTION },
    { "stationaryx", CD_STATIONARY_X },
    { "stationaryy", CD_STATIONARY_Y },
    { "hitflyextendtick", CD_HIT_FLY_EXTEND_TICK },
    { "oldy", CD_OLD_Y },
    { "combatpower", CD_COMBAT_POWER },
    { "attacker", CD_ATTACKER },
    { "tranversallastpicktick", CD_TRANVERSAL_LAST_PICK_TICK },
    { "cannotbefoundbyteamrelativetypecnt", CD_CANNOTBEFOUND_BY_TEAMRELATIVETYPECNT },
    { "innereffectdebuffcount", CD_INNEREFFECT_DEBUFF_COUNT },
    { "innereffectprotectcount", CD_INNEREFFECT_PROTECT_COUNT }
};

static int luabind_characterdata_get_val(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    std::string thekey = lua_tostring(L, 2);
    auto iter = luahelper_characterdata_val_keys.find(thekey);
    if (iter == luahelper_characterdata_val_keys.end())
    {
        lua_pushnil(L);
    }
    else
    {
        auto opcode = iter->second;
        luahelper_character_val_getter(L, c, opcode);
    }
    return 1;
}

static int luabind_characterdata_set_val(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    std::string thekey = lua_tostring(L, 2);
    auto iter = luahelper_characterdata_val_keys.find(thekey);
    if (iter != luahelper_characterdata_val_keys.end())
    {
        auto opcode = iter->second;
        luahelper_character_val_setter(L, c, opcode);
    }
    return 0;
}

static int luabind_characterdata_get_skill_list(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_createtable(L, (int)character->skillList.size(), 0);
    int idx = lua_gettop(L);

    for (int i = 0; i < character->skillList.size(); i++)
    {
        auto &&a_sk = character->skillList[i];
        lua_createtable(L, 2, 0);
        int an_idx = lua_gettop(L);
        lua_pushinteger(L, a_sk.first);
        lua_rawseti(L, an_idx, 1);
        lua_pushinteger(L, a_sk.second);
        lua_rawseti(L, an_idx, 2);

        lua_rawseti(L, idx, i + 1);
    }

    return 1;
}

static int luabind_characterdata_get_innereffects(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int innereffid = lua_tointeger(L, 2);
    auto iter = character->m_aiEffectMap.find(innereffid);
    if (iter == character->m_aiEffectMap.end())
    {
        lua_pushnil(L);
        return 1;
    }
    auto &&innereffs = iter->second;
    lua_createtable(L, (int)innereffs.size(), 0);
    int idx = lua_gettop(L);
    int len = 1;
    for (int i = 0; i < innereffs.size(); i++)
    {
        auto aninnereff = innereffs[i];
        if (aninnereff)
        {
            luastack_pushinnereffect(L, innereffs[i]);
            lua_rawseti(L, idx, len);
            len += 1;
        }
    }
    return 1;
}

static int luabind_characterdata_get_direction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->direction);
    return 1;
}

static int luabind_characterdata_get_currentaction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->currentAction);
    return 1;
}

static int luabind_characterdata_set_currentaction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int action = lua_tointeger(L, 2);
    character->currentAction = ActionType(action);
    return 0;
}

static int luabind_characterdata_get_teamside(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->teamSide);
    return 1;
}

static int luabind_characterdata_canbeattack(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto the_attacker = luastack_getcharacterdata(L, 2);
    bool is_ignore_canbeattack = lua_toboolean(L, 3);
    CharacterData *the_skill_caster = nullptr;
    if (lua_isuserdata(L, 4))
        the_skill_caster = luastack_getcharacterdata(L, 4); // 13.0
    bool can_beattack = the_character->canBeAttacked(the_attacker, is_ignore_canbeattack, the_skill_caster);
    lua_pushboolean(L, can_beattack);
    return 1;
}

static int luabind_characterdata_has_effect(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_effect_type = lua_tointeger(L, -1);
    bool ret_val = the_character->hasEffect(the_effect_type);
    lua_pushboolean(L, ret_val);
    return 1;
}

static int luabind_characterdata_has_mark(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_markid = lua_tointeger(L, -1);
    lua_pushboolean(L, the_character->hasMark(the_markid));
    return 1;
}

static int luabind_characterdata_reset_traversal_targetrecord(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    the_character->resetTraversalTargetRecord();
    return 0;
}

static int luabind_characterdata_is_targetlockedbytraversal(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto the_target = luastack_getcharacterdata(L, -1);
    lua_pushboolean(L, the_character->isTargetLockedByTraversal(the_target->myID));
    return 1;
}

static int luabind_characterdata_pick_traversal_target(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto the_target = luastack_getcharacterdata(L, -2);
    int the_tick = lua_tointeger(L, -1);
    the_character->pickTraversalTarget(the_target->myID, the_tick);
    return 0;
}

static int  luabind_characterdata_is_entereffectaction(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    bool luaret = the_character->isEnterEffectAction();
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_characterdata_is_in_accu_state(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, character->isInAccumulateState());
    return 1;
}

static int luabind_characterdata_enter_accumulatestate(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    c->enterAccumulateState(skillid);
    return 0;
}

static int luabind_characterdata_end_accumulatestate(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    c->endAccumulateState();
    return 0;
}

static int luabind_characterdata_exit_accumulatestate(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    c->exitAccumulateState();
    return 0;
}

static int luabind_characterdata_is_summon(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, the_character->isSummon());
    return 1;
}

static int luabind_characterdata_get_summonsource(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    if (the_character->getSummonSource())
        luastack_pushcharacterdata(L, the_character->getSummonSource());
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_get_fakesummontype(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, the_character->getFakeSummonType());
    return 1;
}

static int luabind_characterdata_get_summons(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_newtable(L);
    int tableidx = lua_gettop(L);
    int i = 1;
    for (auto pSummonCaster : the_character->summonCasts)
    {
        for (auto summonUID : pSummonCaster->summonUIDList)
        {
            auto pSummon = AIContext::getInstance()->getCharacterByID(summonUID);
            if (pSummon && pSummon->isSummon() && pSummon->getSummonSource()->myID == the_character->myID)
            {
                luastack_pushcharacterdata(L, pSummon);
                lua_rawseti(L, tableidx, i);
                i++;
            }
        }
    }
    return 1;
}

static int luabind_characterdata_get_summon_beattackrate(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    if (the_character->summonProperty)
        lua_pushinteger(L, the_character->summonProperty->beAttackRate);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_is_findabletoattacker(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    CharacterData **lua_userdata = static_cast<CharacterData **>(lua_touserdata(L, 2));
    auto the_attacker = *lua_userdata;
    bool is_ignore_canbeattack = lua_toboolean(L, 3);
    lua_pushboolean(L, the_character->isFindableToAttacker(the_attacker, is_ignore_canbeattack));
    return 1;
}

static int luabind_characterdata_is_findabletoskillcaster(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    CharacterData *sc = luastack_getcharacterdata(L, 2);
    bool is_ignore_canbeattack = lua_toboolean(L, 3);
    lua_pushboolean(L, c->isFindableToSkillCaster(sc, is_ignore_canbeattack));
    return 1;
}

static int luabind_characterdata_has_tag(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    const std::string the_tag = lua_tostring(L, -1);
    lua_pushboolean(L, the_character->hasTag(the_tag));
    return 1;
}

static int luabind_characterdata_add_tag(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    std::string tag = lua_tostring(L, 2);
    character->addTag(tag);
    return 0;
}

static int luabind_characterdata_set_tag(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    std::string tag = lua_tostring(L, 2);
    int val = lua_tointeger(L, 3);
    character->setTagValue(tag, val);
    return 0;
}

static int luabind_characterdata_get_tag(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    std::string tag = lua_tostring(L, 2);
    lua_pushinteger(L, character->getTagValue(tag));
    return 1;
}

static int luabind_characterdata_erase_tag(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    std::string tag = lua_tostring(L, 2);
    character->eraseTag(tag);
    return 0;
}

static int luabind_characterdata_is_induel(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, the_character->isInDuel());
    return 1;
}

static int luabind_characterdata_get_duel_target(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    auto target =  character->getDuelTarget();
    if (target)
        luastack_pushcharacterdata(L, target);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_is_target26foundfirst(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, the_character->GetFlagFirstFound26Target());
    return 1;
}

static int luabind_characterdata_get_target26firstfoundtarget(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    if (the_character->GetFirstRoundConfirm26Target())
        luastack_pushcharacterdata(L, the_character->GetFirstRoundConfirm26Target());
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_get_stationary_direction(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, the_character->mStationaryDirection);
    return 1;
}

static int luabind_characterdata_get_stationary_x(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, the_character->mStationaryX);
    return 1;
}

static int luabind_characterdata_get_stationary_y(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, the_character->mStationaryY);
    return 1;
}

static int luabind_characterdata_is_currenttargetconfirmed(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, the_character->mAttackConfirmedTarget);
    return 1;
}

static int luabind_characterdata_set_is_currenttargetconfirmed(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool luaval = lua_toboolean(L, 2);
    character->mAttackConfirmedTarget = luaval;
    return 0;
}

static int luabind_characterdata_get_currenttarget(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    if (the_character->target)
        luastack_pushcharacterdata(L, the_character->target);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_set_currenttarget(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    character->target = target;
    return 0;
}

static int luabind_characterdata_get_taunttarget(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    if (c->tauntTarget)
        luastack_pushcharacterdata(L, c->tauntTarget);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_set_taunttarget(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    auto t = luastack_getcharacterdata(L, 2);
    c->tauntTarget = t;
    return 0;
}

static int luabind_characterdata_get_remaineffectcd(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int effid = lua_tointeger(L, 2);
    int luaret = the_character->getEffectRemainCD(effid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_characterdata_is_in_taunt(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, character->isInTauntEffect());
    return 1;
}

static int luabind_characterdata_is_in_charm(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, character->isInCharmEffect());
    return 1;
}

static int luabind_characterdata_is_in_half_charm(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, character->isInHalfCharmEffect());
    return 1;
}

static int luabind_characterdata_get_target_for_focus(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    if (character->targetForFocus)
        luastack_pushcharacterdata(L, character->targetForFocus);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_get_can_cast_ultra(lua_State *L)
{
    CharacterData *character = luastack_getcharacterdata(L, 1);
    bool is_external = lua_toboolean(L, 2);
    lua_pushinteger(L, character->canCastUltraSkillNative(is_external));
    return 1;
}

static int luabind_characterdata_get_special_skill(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int specialcond = lua_tointeger(L, 2);
    auto theskillinfo = character->getSpecialSkill(ESkillCastCondition(specialcond));
    if (theskillinfo)
        luastack_pushspecialskillinfo(L, theskillinfo);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_set_skillseqindex(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int seqindex = lua_tointeger(L, 2);
    HeroActionController::setSkillSeq(character->actionAtt, seqindex);
    return 0;
}

static int luabind_characterdata_set_skillseqindex2(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int seqindex = lua_tointeger(L, 2);
    character->m_iSkillSeq2Index = seqindex;
    return 0;
}

static int luabind_characterdata_get_skillseqindex(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int seqindex = HeroActionController::getSkillSeq(character->actionAtt);
    lua_pushinteger(L, seqindex);
    return 1;
}

static int luabind_characterdata_get_skillseqindex2(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->m_iSkillSeq2Index);
    return 1;
}

static int luabind_characterdata_set_actioncauser_myid(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int luaval = lua_tointeger(L, 2);
    character->actionCauserMyID = luaval;
    return 0;
}

static int luabind_characterdata_get_killer_myid(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->killerUID);
    return 1;
}

static int luabind_characterdata_get_starttick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->startTick);
    return 1;
}

static int luabind_characterdata_set_starttick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int luaval = lua_tointeger(L, 2);
    character->startTick = luaval;
    return 0;
}

static int luabind_characterdata_get_localtick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->localTick);
    return 1;
}

static int luabind_characterdata_set_localtick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int luaval = lua_tointeger(L, 2);
    character->localTick = luaval;
    return 0;
}

static int luabind_characterdata_get_totaltick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->totalTick);
    return 1;
}

static int luabind_characterdata_set_totaltick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int luaval = lua_tointeger(L, 2);
    character->totalTick = luaval;
    return 0;
}

static int luabind_characterdata_get_roletype(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->roleType);
    return 1;
}

static int luabind_characterdata_is_needadjusting(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushboolean(L, character->needAdjusting);
    return 1;
}

static int luabind_characterdata_set_is_needadjusting(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool luaval = lua_toboolean(L, 2);
    character->needAdjusting = luaval;
    return 0;
}

static int luabind_characterdata_has_skill(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    bool luaret = character->hasSkill(skillid);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_characterdata_get_specialskilltarget(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    if (character->specialFoundSkillTarget)
        luastack_pushcharacterdata(L, character->specialFoundSkillTarget);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_set_specialskilltarget(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    character->specialFoundSkillTarget = target;
    return 0;
}

static int luabind_characterdata_get_normalskill(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->normalSkill.first);
    lua_pushinteger(L, character->normalSkill.second);
    return 2;
}

static int luabind_characterdata_get_skilllevel(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skilllevel = character->getSkillLevel(skillid);
    lua_pushinteger(L, skilllevel);
    return 1;
}

static int luabind_characterdata_get_nextalternateaccumaxtick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    lua_pushinteger(L, character->getNextAlternateAccuMaxTick(skillid));
    return 1;
}

static int luabind_characterdata_set_currentalternateaccumaxtick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int alttick = lua_tointeger(L, 3);
    character->setCurrentAlternateAccuMaxTick(skillid, alttick);
    return 0;
}

static int luabind_characterdata_get_currentalternateaccumaxtick(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int luaret = character->getCurrentAlternateAccuMaxTick(skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_characterdata_get_attackspeed(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->attackSpeed);
    return 1;
}

static int luabind_characterdata_set_attackspeed(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int speed = lua_tointeger(L, 2);
    character->attackSpeed = speed;
    return 0;
}

static int luabind_characterdata_update_skilllevel(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skilllv = lua_tointeger(L, 3);
    c->updateSkillLevel(skillid, skilllv);
    return 0;
}

static int luabind_characterdata_backup_skillinfo(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skilllv = lua_tointeger(L, 3);
    c->backupSkillInfo(skillid, skilllv);
    return 0;
}

static int luabind_characterdata_has_skillinfobackup(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    bool luaret = c->hasSkillInfoBackup(skillid);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_characterdata_get_skillinfobackup(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int luaret = c->getSkillInfoBackup(skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_characterdata_get_effectcaster(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int emyid = lua_tointeger(L, 2);
    auto e = c->getEffectCaster(emyid);
    if (e) luastack_pusheffectcaster(L, e);
    else lua_pushnil(L);
    return 1;
}

static int luabind_characterdata_get_summonproperty_treatrate(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int luaret = 0;
    if (c->summonProperty) luaret = c->summonProperty->treatRate;
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_characterdata_get_attackactioninfo(lua_State *L)
{
	auto c = luastack_getcharacterdata(L, 1);
	lua_pushinteger(L, c->attackActionInfo.skillid);
	lua_pushinteger(L, c->attackActionInfo.skilllv);
	lua_pushinteger(L, c->attackActionInfo.skilltype);
	lua_pushboolean(L, c->attackActionInfo.isforce ? 1 : 0);
	return 4;
}

static int luabind_characterdata_get_currentenergyskillid(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    lua_pushinteger(L, character->currentEnergySkillId);
    return 1;
}

static int luabind_characterdata_set_currentenergyskillid(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    character->currentEnergySkillId = skillid;
    return 0;
}

static int luabind_characterdata_get_currentfriendlist(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto friends = the_character->getCurrentFriendsList();
    lua_createtable(L, 0, 0);
    int list_idx = lua_gettop(L);
    int i = 1;
    for (auto a_character : *friends)
    {
        luastack_pushcharacterdata(L, a_character);
        lua_rawseti(L, list_idx, i);
        i++;
    }
    return 1;
}

static int luabind_characterdata_get_currentrivallist(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto rivals = the_character->getCurrentRivalsList();
    lua_createtable(L, 0, 0);
    int list_idx = lua_gettop(L);
    int i = 1;
    for (auto a_character : *rivals)
    {
        luastack_pushcharacterdata(L, a_character);
        lua_rawseti(L, list_idx, i);
        i++;
    }
    return 1;
}

static int luabind_characterdata_get_talentlist(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    lua_newtable(L);
    int list_idx = lua_gettop(L);
    int i = 1;
    for (auto iter = the_character->talentList.begin(); iter != the_character->talentList.end(); ++iter)
    {
        lua_newtable(L);
        int in_list_idx = lua_gettop(L);
        lua_pushinteger(L, iter->first);
        lua_rawseti(L, in_list_idx, 1);
        lua_pushinteger(L, iter->second);
        lua_rawseti(L, in_list_idx, 2);
        
        lua_rawseti(L, list_idx, i);
        i++;
    }
    return 1;
}

static int luabind_characterdata_get_tmp_friends(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto targets = the_character->m_pTmpFriends;
    if (targets)
    {
        lua_createtable(L, targets->size(), 0);
        int list_idx = lua_gettop(L);
        int i = 1;
        for (auto a_character : *targets)
        {
            luastack_pushcharacterdata(L, a_character);
            lua_rawseti(L, list_idx, i);
            i++;
        }
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int luabind_characterdata_get_tmp_rivals(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    auto targets = the_character->m_pTmpRivals;
    if (targets)
    {
        lua_createtable(L, targets->size(), 0);
        int list_idx = lua_gettop(L);
        int i = 1;
        for (auto a_character : *targets)
        {
            luastack_pushcharacterdata(L, a_character);
            lua_rawseti(L, list_idx, i);
            i++;
        }
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int luabind_characterdata_on_escape(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    the_character->onEscape();
    return 0;
}

static int luabind_characterdata_on_hurt(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    bool iseff = lua_toboolean(L, 2);
    int hurtid = lua_tointeger(L, 3);
    int hpdelta = lua_tointeger(L, 4);
    int hurttype = lua_tointeger(L, 5);
    auto the_attacker = luastack_getcharacterdata(L, 6);
    int thecasterid = lua_tointeger(L, 7);
    the_character->onHurt(iseff, hurtid, hpdelta, hurttype, the_attacker, thecasterid);
    return 0;
}

static int luabind_characterdata_on_criticalhitskillrelease(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    the_character->onCruelHitSkillRelease();
    return 0;
}

static int luabind_characterdata_on_castcriticalhurt(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    the_character->onCastCritHurt();
    return 0;
}

static int luabind_characterdata_on_hurthit(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int hurtid = lua_tointeger(L, 2);
    the_character->onHurtHit(hurtid);
    return 0;
}

static int luabind_characterdata_on_hpdelta(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_hpdelta = lua_tointeger(L, 2);
    the_character->onHPDelta(the_hpdelta);
    return 0;
}

static int luabind_characterdata_on_frienddead(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    auto thefriend = luastack_getcharacterdata(L, 2);
    character->onFriendDead(thefriend);
    return 0;
}

static int luabind_characterdata_on_startcastskill(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    c->onStartCastSkill(skillid);
    return 0;
}

static int luabind_characterdata_exit_duel(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    character->exitDuel();
    return 0;
}

static int luabind_characterdata_marks_on_selfenterdead(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    character->marksOnSelfEnterDead();
    return 0;
}

static int luabind_characterdata_forceturntotarget(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    auto t = luastack_getcharacterdata(L, 2);
    c->forceTurnToTarget(t);
    return 0;
}

static int luabind_characterdata_check_target(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int ifjuexingskill = lua_tointeger(L, 2);
    int luaret = c->checkTargetNative(ifjuexingskill);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_characterdata_update_hp(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_hp = lua_tointeger(L, 2);
    the_character->updateHP(the_hp);
    return 0;
}

static int luabind_characterdata_update_energy(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_energy = lua_tointeger(L, 2);
    the_character->updateEnergy(the_energy);
    return 0;
}

static int luabind_characterdata_set_team_list_info(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int c_team_type = lua_tointeger(L, 2);
    int ai_team_type = lua_tointeger(L, 3);
    int ai_team_side = lua_tointeger(L, 4);
    int ai_clusternum = lua_tointeger(L, 5);
    bool ai_is_exclude_summon = lua_toboolean(L, 6);
    CharacterFunc::SetTeamListInfo(c, c_team_type, ai_team_type, ai_team_side, ai_clusternum, ai_is_exclude_summon);
    return 0;
}

static int luabind_characterdata_has_caster(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int caster_type = lua_tointeger(L, 2);
    int caster_id = lua_tointeger(L, 3);
    bool retval = CharacterFunc::CheckHasCaster(c, caster_type, caster_id);
    lua_pushboolean(L, retval);
    return 1;
}

static int luabind_characterdata_get_effect_casters(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    if (c->effectCasts.size() <= 0) {
        lua_pushnil(L);
        return 1;
    }
    lua_createtable(L, (int)c->effectCasts.size(), 0);
    int idx = lua_gettop(L);
    int len = 1;
    for (AIContinuousEffectCaster *a_caster : c->effectCasts) {
        if (a_caster) {
            luastack_pusheffectcaster(L, a_caster);
            lua_rawseti(L, idx, len);
            len += 1;
        }
    }
    return 1;
}

static int luabind_characterdata_get_summon_casters(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    if (c->summonCasts.size() <= 0) {
        lua_pushnil(L);
        return 1;
    }

    lua_createtable(L, (int)c->summonCasts.size(), 0);
    int idx = lua_gettop(L);
    int len = 1;
    for (AIContinuousSummonCaster *a_caster : c->summonCasts) {
        if (a_caster) {
            luastack_pushsummoncaster(L, a_caster);
            lua_rawseti(L, idx, len);
            len += 1;
        }
    }
    return 1;
}

static int luabind_characterdata_has_label(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int label = lua_tointeger(L, 2);
    bool retval = CharacterFunc::HasLabel(c, label);
    lua_pushboolean(L, retval);
    return 1;
}

int luaopen_characterdata(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "new_ref", luabind_characterdata_new_ref },
        { NULL, NULL }
    };
    
    static const luaL_Reg reg_m[] = {
        { "get_myid", luabind_characterdata_get_myid },
        { "get_attr", luabind_characterdata_get_attr },
        { "set_attr", luabind_characterdata_set_attr },
        { "get_attr_origin", luabind_characterdata_get_attr_origin }, // 22.0
        { "get_attr_afterbegin", luabind_characterdata_get_attr_afterbegin }, // 22.0
        { "get_actionatt", luabind_characterdata_get_actionatt },
        { "set_actionatt", luabind_characterdata_set_actionatt },
        { "get_data", luabind_characterdata_get_data },
        { "set_data", luabind_characterdata_set_data },
        { "get_val", luabind_characterdata_get_val },
        { "set_val", luabind_characterdata_set_val },
        { "get_skill_list", luabind_characterdata_get_skill_list },
        { "get_innereffects", luabind_characterdata_get_innereffects },
        
        { "get_direction", luabind_characterdata_get_direction },
        { "get_currentaction", luabind_characterdata_get_currentaction },
        { "set_currentaction", luabind_characterdata_set_currentaction },
        { "get_teamside", luabind_characterdata_get_teamside },
        { "canbeattack", luabind_characterdata_canbeattack },
        { "has_effect", luabind_characterdata_has_effect},
        { "has_mark", luabind_characterdata_has_mark},
        { "is_summon", luabind_characterdata_is_summon},
        { "get_summonsource", luabind_characterdata_get_summonsource},
        { "get_fakesummontype", luabind_characterdata_get_fakesummontype},
        { "get_summons", luabind_characterdata_get_summons },
        { "get_summon_beattackrate", luabind_characterdata_get_summon_beattackrate},
        { "is_findabletoattacker", luabind_characterdata_is_findabletoattacker},
        { "is_findabletoskillcaster", luabind_characterdata_is_findabletoskillcaster }, // 13.0
        { "has_tag", luabind_characterdata_has_tag },
        { "add_tag", luabind_characterdata_add_tag },
        { "set_tag", luabind_characterdata_set_tag },
        { "get_tag", luabind_characterdata_get_tag },
        { "erase_tag", luabind_characterdata_erase_tag },
        { "is_induel", luabind_characterdata_is_induel},
        { "get_duel_target", luabind_characterdata_get_duel_target },
        { "is_target26foundfirst", luabind_characterdata_is_target26foundfirst},
        { "get_target26firstfoundtarget", luabind_characterdata_get_target26firstfoundtarget},
        { "get_stationary_direction", luabind_characterdata_get_stationary_direction},
        { "get_stationary_x", luabind_characterdata_get_stationary_x},
        { "get_stationary_y", luabind_characterdata_get_stationary_y},
        { "is_currenttargetconfirmed", luabind_characterdata_is_currenttargetconfirmed},
        { "set_is_currenttargetconfirmed", luabind_characterdata_set_is_currenttargetconfirmed },
        { "get_currenttarget", luabind_characterdata_get_currenttarget},
        { "set_currenttarget", luabind_characterdata_set_currenttarget },
        { "get_taunttarget", luabind_characterdata_get_taunttarget }, //OP2
        { "set_taunttarget", luabind_characterdata_set_taunttarget }, //OP2
        { "get_remaineffectcd", luabind_characterdata_get_remaineffectcd },
        { "is_in_taunt", luabind_characterdata_is_in_taunt },
        { "is_in_charm", luabind_characterdata_is_in_charm },
        { "is_in_half_charm", luabind_characterdata_is_in_half_charm },
        { "get_target_for_focus", luabind_characterdata_get_target_for_focus },
        { "get_can_cast_ultra", luabind_characterdata_get_can_cast_ultra },
        { "get_special_skill", luabind_characterdata_get_special_skill },
        { "set_skillseqindex", luabind_characterdata_set_skillseqindex },
        { "set_skillseqindex2", luabind_characterdata_set_skillseqindex2 },
        { "get_skillseqindex", luabind_characterdata_get_skillseqindex },
        { "get_skillseqindex2", luabind_characterdata_get_skillseqindex2 },
        { "set_actioncauser_myid", luabind_characterdata_set_actioncauser_myid },
        { "get_killer_myid", luabind_characterdata_get_killer_myid },
        { "get_starttick", luabind_characterdata_get_starttick },
        { "set_starttick", luabind_characterdata_set_starttick },
        { "get_localtick", luabind_characterdata_get_localtick },
        { "set_localtick", luabind_characterdata_set_localtick },
        { "get_totaltick", luabind_characterdata_get_totaltick },
        { "set_totaltick", luabind_characterdata_set_totaltick },
        { "get_roletype", luabind_characterdata_get_roletype },
        { "is_needadjusting", luabind_characterdata_is_needadjusting },
        { "set_is_needadjusting", luabind_characterdata_set_is_needadjusting },
        { "has_skill", luabind_characterdata_has_skill },
        { "get_specialskilltarget", luabind_characterdata_get_specialskilltarget },
        { "set_specialskilltarget", luabind_characterdata_set_specialskilltarget },
        { "get_normalskill", luabind_characterdata_get_normalskill },
        { "get_skilllevel", luabind_characterdata_get_skilllevel },
        { "get_nextalternateaccumaxtick", luabind_characterdata_get_nextalternateaccumaxtick },
        { "set_currentalternateaccumaxtick", luabind_characterdata_set_currentalternateaccumaxtick },
        { "get_currentalternateaccumaxtick", luabind_characterdata_get_currentalternateaccumaxtick },
        { "get_attackspeed", luabind_characterdata_get_attackspeed },
        { "set_attackspeed", luabind_characterdata_set_attackspeed },
        { "update_skilllevel", luabind_characterdata_update_skilllevel },
        { "backup_skillinfo", luabind_characterdata_backup_skillinfo },
        { "has_skillinfobackup", luabind_characterdata_has_skillinfobackup },
        { "get_skillinfobackup", luabind_characterdata_get_skillinfobackup },
        { "get_effectcaster", luabind_characterdata_get_effectcaster },
        { "get_summonproperty_treatrate", luabind_characterdata_get_summonproperty_treatrate },
		{ "get_attackactioninfo", luabind_characterdata_get_attackactioninfo },
        
        { "reset_traversal_targetrecord", luabind_characterdata_reset_traversal_targetrecord},
        { "is_targetlockedbytraversal", luabind_characterdata_is_targetlockedbytraversal},
        { "pick_traversal_target", luabind_characterdata_pick_traversal_target},
        { "is_entereffectaction", luabind_characterdata_is_entereffectaction},
        { "is_in_accu_state", luabind_characterdata_is_in_accu_state },
        { "enter_accumulatestate", luabind_characterdata_enter_accumulatestate },
        { "end_accumulatestate", luabind_characterdata_end_accumulatestate },
        { "exit_accumulatestate", luabind_characterdata_exit_accumulatestate },
        
        { "get_currentfriendlist", luabind_characterdata_get_currentfriendlist},
        { "get_currentrivallist", luabind_characterdata_get_currentrivallist},
        { "get_talentlist", luabind_characterdata_get_talentlist},
        { "get_tmp_friends", luabind_characterdata_get_tmp_friends },
        { "get_tmp_rivals", luabind_characterdata_get_tmp_rivals },
        
        { "on_escape", luabind_characterdata_on_escape},
        { "on_hurt", luabind_characterdata_on_hurt},
        { "on_criticalhitskillrelease", luabind_characterdata_on_criticalhitskillrelease},
        { "on_castcriticalhurt", luabind_characterdata_on_castcriticalhurt},
        { "on_hurthit", luabind_characterdata_on_hurthit},
        { "on_hpdelta", luabind_characterdata_on_hpdelta},
        { "on_frienddead", luabind_characterdata_on_frienddead},
        { "on_startcastskill", luabind_characterdata_on_startcastskill},

        { "exit_duel", luabind_characterdata_exit_duel },
        { "marks_on_selfenterdead", luabind_characterdata_marks_on_selfenterdead },
        { "forceturntotarget", luabind_characterdata_forceturntotarget },
        { "check_target", luabind_characterdata_check_target },
        
        { "update_hp", luabind_characterdata_update_hp},
        { "update_energy", luabind_characterdata_update_energy},

        { "set_team_list_info", luabind_characterdata_set_team_list_info },
        { "has_caster", luabind_characterdata_has_caster },
        { "get_effect_casters", luabind_characterdata_get_effect_casters },
        { "get_summon_casters", luabind_characterdata_get_summon_casters },

        { "has_label", luabind_characterdata_has_label },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, luatypename_characterdata());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    
    return 1;
}

// MARK: Lua binding character --
    
CharacterData *luahelper_characterdata_locktarget(lua_State *L, CharacterData *character, bool isJudgeBy297)
{
    lua_getglobal(L, "g_characterhelper");
    lua_getfield(L, -1, "locktarget");

    luastack_pushcharacterdata(L, character);
    lua_pushboolean(L, isJudgeBy297);

    if (lua_pcall(L, 2, 1, 0))
    {
        luahelper_errorlog(L);
        lua_settop(L, 0);
        return nullptr;
    }

    auto target = luastack_getcharacterdata(L, -1);
    lua_settop(L, 0);
    return target;
}

bool luahelper_characterdata_canbeattackbyskill(lua_State *L, CharacterData *c, CharacterData *attacker, int skillid, AIProjectile *p)
{
    lua_getglobal(L, "g_characterhelper");
    lua_getfield(L, -1, "canbeattackbyskill");

    luastack_pushcharacterdata(L, c);
    luastack_pushcharacterdata(L, attacker);
    lua_pushinteger(L, skillid);
    if (p)
        luastack_pushprojectile(L, p);
    else
        lua_pushnil(L);

    if (lua_pcall(L, 4, 1, 0))
    {
        luahelper_errorlog(L);
        return false;
    }

    bool ret = lua_toboolean(L, -1);
    lua_settop(L, 0);
    return ret;
}

bool luahelper_characterdata_is_rival_team_all_dead_for_check_ultra(lua_State *L, CharacterData *c)
{
    lua_getglobal(L, "g_characterhelper");
    lua_getfield(L, -1, "is_rivalteam_alldead_for_checkultra");
    lua_remove(L, -2);

    luastack_pushcharacterdata(L, c);
    if (lua_pcall(L, 1, 1, 0))
    {
        luahelper_errorlog(L);
        return false;
    }

    bool ret = lua_toboolean(L, -1);
    lua_settop(L, 0);
    return ret;
}

}
