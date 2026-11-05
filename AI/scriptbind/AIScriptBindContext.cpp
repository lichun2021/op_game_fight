//
//  AIScriptBindContext.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindContext.hpp"
#include <vector>
#include "lua.hpp"
#include "cocos2d.h"
#include "AIScriptBindHelper.h"
#include "AIScriptBindCharacterData.hpp"
#include "AIScriptBindProjectile.hpp"
#include "AIScriptBindCallback.hpp"
#include "AIScriptBindHurtCalculate.hpp"
#include "../AIContext.h"
#include "../action_hero/HeroSkillController.h"
#include "../CommonType.h"

namespace OnePiece {

// MARK: -- Lua binding aicontext

static inline const char *luatypename_aicontext() { return "aicontext"; }

//    static int luabind_aicontext_get_instance(lua_State *L)
//    {
//        size_t sz = sizeof(AIContext *);
//
//        AIContext **the_fulluserdata = static_cast<AIContext **>(lua_newuserdata(L, sz));
//        *the_fulluserdata = AIContext::getInstance();
//
//        luaL_getmetatable(L, luatypename_aicontext());
//        lua_setmetatable(L, -2);
//
//        return 1;
//    }

static int luabind_aicontext_get_currenttick(lua_State *L)
{
    lua_pushinteger(L, (int)AIContext::getInstance()->currentTick);
    return 1;
}

static int luabind_aicontext_get_randomnumber(lua_State *L)
{
    lua_pushinteger(L, (int)AIContext::getInstance()->localRandom->nextInt(0, 100));
    return 1;
}

static int luabind_aicontext_get_randomnumber_within_range(lua_State *L)
{
    int thehighnum = lua_tointeger(L, -1);
    lua_pushinteger(L, (int)AIContext::getInstance()->localRandom->nextInt(0, thehighnum));
    return 1;
}

static int luabind_aicontext_get_battlefield_center_x(lua_State *L)
{
    lua_pushinteger(L, AIContext::getInstance()->battleFieldCenterX);
    return 1;
}

static int luabind_aicontext_set_battlefield_center_x(lua_State *L)
{
    int cx = lua_tointeger(L, 1);
    AIContext::getInstance()->battleFieldCenterX = cx;
    return 0;
}

static int luabind_aicontext_get_cluster_range_to_centerx(lua_State *L)
{
    lua_pushinteger(L, AIContext::getInstance()->clusterRangeToCenterX);
    return 1;
}

static int luabind_aicontext_set_cluster_range_to_centerx(lua_State *L)
{
    int cr = lua_tointeger(L, 1);
    AIContext::getInstance()->clusterRangeToCenterX = cr;
    return 0;
}

static int luabind_aicontext_get_delta_max_y(lua_State *L)
{
    lua_pushinteger(L, (int)AIContext::getInstance()->deltaMaxY);
    return 1;
}

static int luabind_aicontext_get_delta_min_y(lua_State *L)
{
    lua_pushinteger(L, (int)AIContext::getInstance()->deltaMinY);
    return 1;
}

static int luabind_aicontext_get_businesstype(lua_State *L)
{
    lua_pushinteger(L, AIContext::getInstance()->businessType);
    return 1;
}

static int luabind_aicontext_get_simu_calc_hurt(lua_State *L)
{
    auto the_target = luastack_getcharacterdata(L, -1);
    lua_pop(L, 1);
    
    int the_skill_level = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_pop(L, 1);
    
    int the_hurt_id = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    auto the_projectile = luastack_getprojectile(L, -1);
    lua_pop(L, 1);
    
    auto the_source = luastack_getcharacterdata(L, -1);
    lua_pop(L, 1);
    
    int thehurt = HeroHurtcalculate::simulateCalcHurt(*AIContext::getInstance(), the_source, the_projectile, the_hurt_id, 0, the_skill_level, the_target);
    lua_pushinteger(L, thehurt);
    return 1;
}

static int luabind_aicontext_hurtcalculate_triggermark(lua_State *L)
{
    //(target, skillid, skill_level, hurttype, hurtprop, hpdelta, hittype, source, hurtid)
    auto target = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skill_level = lua_tointeger(L, 3);
    int hurttype = lua_tointeger(L, 4);
    int hurtprop = lua_tointeger(L, 5);
    int hpdelta = lua_tointeger(L, 6);
    int hittype = lua_tointeger(L, 7);
    auto trigger = luastack_getcharacterdata(L, 8);
    int hurtid = lua_tointeger(L, 9);
    HeroHurtcalculate::ToTriggerMarkCasterFun(*AIContext::getInstance(), *target, skillid, skill_level, hurttype, hurtprop, hpdelta, hittype, trigger, hurtid);
    return 0;
}

static int luabind_aicontext_execute_exec(lua_State *L)
{
    //(execid, skillid, skill_level, source, projectile, target, 0, false, markid)
    int execid = lua_tointeger(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skill_level = lua_tointeger(L, 3);
    auto source = luastack_getcharacterdata(L, 4);
    auto projectile = luastack_getprojectile(L, 5);
    auto target = luastack_getcharacterdata(L, 6);
    int parenttick = lua_tointeger(L, 7);
    bool uselogicpos = lua_toboolean(L, 8);
    int markid = lua_tointeger(L, 9);
    AIActionController::executeExec(*AIContext::getInstance(), execid, skillid, skill_level, source, projectile, target, parenttick, uselogicpos, markid);
    return 0;
}

static int luabind_aicontext_handle_hurtresult(lua_State *L)
{
    //(source, target, hurtid)
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    int hurtid = lua_tointeger(L, 3);
    auto hurtexec = AIDataHelper::getHurtExec(hurtid);
    HeroHurtcalculate::handleHurtResult(*AIContext::getInstance(), source, target, hurtexec);
    return 0;
}

static int luabind_aicontext_on_releasehpskill(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    HeroSkillController::checkReleaseHPSkill(*AIContext::getInstance(), target);
    return 0;
}

//    static int luabind_aicontext_hurttarget(lua_State *L)
//    {
//        auto source = luastack_getcharacterdata(L, 1);
//        auto target = luastack_getcharacterdata(L, 2);
//        HurtCalcParam hurtparam(0, 0, 0, 0, 0, 0);
//        int err = luastack_gethurtcalcparam(L, 3, hurtparam);
//        assert(err == 0);
//        HeroHurtcalculate::HurtFuncHurtTarget(*AIContext::getInstance(), source, target, hurtparam);
//        return 0;
//    }

static int luabind_aicontext_schedule_execute(lua_State *L)
{
    int theExecID = lua_tointeger(L, 1);
    int theSkillID = lua_tointeger(L, 2);
    int theSkillLevel = lua_tointeger(L, 3);
    auto theSource = luastack_getcharacterdata(L, 4);
    auto theProjectile = luastack_getprojectile(L, 5);
    auto theTarget = luastack_getcharacterdata(L, 6);
    int theParentTicklen = lua_tointeger(L, 7);
    bool theUseLogicPos = lua_toboolean(L, 8);
    int theWaitTickLen = lua_tointeger(L, 9);
    AIContext::getInstance()->scheduleExcute(theExecID, theSkillID, theSkillLevel, theSource, theProjectile, theTarget, theParentTicklen, theUseLogicPos, theWaitTickLen);
    return 0;
}
    
static int luabind_aicontext_get_string_from_file(lua_State *L)
{
    std::string filename = lua_tostring(L, 1);
#ifdef SERVER_VERIFYING
    std::string str = "";
#else
    std::string str = cocos2d::FileUtils::getInstance()->getStringFromFile(filename);
#endif
    lua_pushstring(L, str.c_str());
    return 1;
}

static int luabind_aicontext_get_character_by_myid(lua_State *L)
{
    int myid = lua_tointeger(L, 1);
    auto character = AIContext::getInstance()->getCharacterByID(myid);
    if (character)
        luastack_pushcharacterdata(L, character);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_aicontext_get_autofight(lua_State *L)
{
    lua_pushinteger(L, AIContext::getInstance()->autoFight);
    return 1;
}

static int luabind_aicontext_is_monster_can_move(lua_State *L)
{
    lua_pushboolean(L, AIContext::getInstance()->monsterCanMove);
    return 1;
}

static int luabind_aicontext_trigger_marks_on_character_dead(lua_State *L)
{
    AIContext::getInstance()->onMarkReleaseFun();
    return 0;
}

static int luabind_aicontext_remove_marks_on_character_dead(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    AIContext::getInstance()->onCharacterDeadIfMarkClearFun(character);
    return 0;
}

static int luabind_aicontext_on_event(lua_State *L)
{
    std::string name = lua_tostring(L, 1);
    auto character = luastack_getcharacterdata(L, 2);
    AIContext::getInstance()->onEvent(name, character);
    return 0;
}

static int luabind_aicontext_is_characterinbattlefield(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool luaret = AIContext::getInstance()->getOwnerIsInVaildBarrier(character);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_aicontext_get_is_fighting(lua_State *L)
{
    lua_pushboolean(L, AIContext::getInstance()->isFighting);
    return 1;
}

static int luabind_aicontext_set_is_fighting(lua_State *L)
{
    bool luaval = lua_toboolean(L, 1);
    AIContext::getInstance()->isFighting = luaval;
    return 0;
}

static int luabind_aicontext_add_event_listener(lua_State *L)
{
    std::string key = lua_tostring(L, 1);
    auto character = luastack_getcharacterdata(L, 2);
    AIContext::getInstance()->addEventListener(key, character);
    return 0;
}

static int luabind_aicontext_get_friend_alive_nums(lua_State *L)
{
    auto a = AIContext::getInstance();
    lua_pushinteger(L, a->friend_num[0]);
    lua_pushinteger(L, a->friend_num[1]);
    lua_pushinteger(L, a->friend_num[2]);
    return 3;
}

static int luabind_aicontext_get_rival_alive_nums(lua_State *L)
{
    auto a = AIContext::getInstance();
    lua_pushinteger(L, a->rival_num[0]);
    lua_pushinteger(L, a->rival_num[1]);
    lua_pushinteger(L, a->rival_num[2]);
    return 3;
}

static int luabind_aicontext_add_trackultra(lua_State *L)
{
    int tick = lua_tointeger(L, 1);
    int myid = lua_tointeger(L, 2);
    AIContext::getInstance()->trackUltra.push_back(std::make_pair(tick, myid));
    return 0;
}

static int luabind_aicontext_pushfront_ultrafreeze(lua_State *L)
{
    int len = lua_rawlen(L, 1);
    for (int i = 1; i <= len; i++)
        lua_rawgeti(L, 1, i);
    int offset = 2;
    int* ultraFreeze = new int[10]();
    ultraFreeze[0] = lua_tointeger(L, offset + 0);
    ultraFreeze[1] = lua_tointeger(L, offset + 1);
    ultraFreeze[2] = lua_tointeger(L, offset + 2);
    ultraFreeze[3] = lua_tointeger(L, offset + 3);
    ultraFreeze[4] = lua_tointeger(L, offset + 4);
    ultraFreeze[5] = lua_tointeger(L, offset + 5);
    ultraFreeze[6] = lua_tointeger(L, offset + 6);

    ultraFreeze[7] = lua_tointeger(L, offset + 7);
    ultraFreeze[8] = lua_tointeger(L, offset + 8);
    ultraFreeze[9] = lua_tointeger(L, offset + 9);
    AIContext::getInstance()->ultraFreezeList.push_front(ultraFreeze);
    if (len > 0)
        lua_pop(L, len);
    return 0;
}

static int luabind_aicontext_get_aicallback(lua_State *L)
{
    auto pcallback = AIContext::getInstance()->aiCallback;
    if (pcallback)
        luastack_pushaicallback(L, pcallback);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_aicontext_get_adjustingdistance(lua_State *L)
{
    lua_pushinteger(L, AIContext::getInstance()->adjustingDistance);
    return 1;
}

static int luabind_aicontext_set_adjustingdistance(lua_State *L)
{
    AIContext::getInstance()->adjustingDistance = lua_tointeger(L, 1);
    return 0;
}

static int luabind_aicontext_has_winlose(lua_State *L)
{
    lua_pushboolean(L, AIContext::getInstance()->hasWinlose());
    return 1;
}

static int luabind_aicontext_get_winlose(lua_State *L)
{
    lua_pushinteger(L, AIContext::getInstance()->winlose);
    return 1;
}

static int luabind_aicontext_set_winlose(lua_State *L)
{
    AIContext::getInstance()->winlose = lua_tointeger(L, 1);
    return 0;
}

static int luabind_aicontext_stop_ultrafreeze(lua_State *L)
{
    int cmyid = lua_tointeger(L, 1);
    AIContext::getInstance()->stopUltraFreeze(cmyid);
    return 0;
}
    
static int luabind_aicontext_get_configdata_json(lua_State *L)
{
    int data_type = lua_tointeger(L, 1);
    int data_id = lua_tointeger(L, 2);
    std::string data = AIDataHelper::scriptGetConfigData(data_type, data_id);

    if (!data.empty()) {
        lua_pushstring(L, data.c_str());
    }
    else {
        lua_pushnil(L);
    }
    return 1;
}

static int luabind_aicontext_is_being_final_clear(lua_State *L)
{
    lua_pushboolean(L, AIContext::getInstance()->isBeingFinalClear());
    return 1;
}

static int luabind_aicontext_add_character_to_cluster(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int teamlist_type = lua_tointeger(L, 2);
    int teamside = lua_tointeger(L, 3);
    int clusternum = lua_tointeger(L, 4);
    bool is_excludesummon = lua_toboolean(L, 5);
    AIContext::getInstance()->addCharacterToCluster(c, teamlist_type, teamside, clusternum, is_excludesummon);
    return 0;
}

static int luabind_aicontext_remove_character_from_cluster(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int teamlist_type = lua_tointeger(L, 2);
    int teamside = lua_tointeger(L, 3);
    int clusternum = lua_tointeger(L, 4);
    bool is_excludesummon = lua_toboolean(L, 5);
    AIContext::getInstance()->removeCharacterFromCluster(c, teamlist_type, teamside, clusternum, is_excludesummon);
    return 0;
}

static int luabind_aicontext_check_update_alive_cnt(lua_State *L)
{
    int teamside = lua_tointeger(L, 1);
    int clusternum = lua_tointeger(L, 2);
    AIContext::getInstance()->checkUpdateTeamAliveInfo(teamside, clusternum);
    return 0;
}

static int luabind_aicontext_set_team_alive_info(lua_State *L)
{
    int teamside = lua_tointeger(L, 1);
    int clusternum = lua_tointeger(L, 2);
    int alivecnt = lua_tointeger(L, 3);
    int deathtriggercnt = lua_tointeger(L, 4);
    AIContext::getInstance()->setTeamAliveInfo(teamside, clusternum, alivecnt, deathtriggercnt);
    return 0;
}

static int luabind_aicontext_get_team_alive_info(lua_State *L)
{
    int teamside = lua_tointeger(L, 1);
    int clusternum = lua_tointeger(L, 2);
    int alivecnt = 0;
    int deathtriggercnt = 0;
    AIContext::getInstance()->getTeamAliveInfo(teamside, clusternum, alivecnt, deathtriggercnt);
    lua_pushinteger(L, alivecnt);
    lua_pushinteger(L, deathtriggercnt);
    return 2;
}

static int luabind_aicontext_get_current_team_clusternum(lua_State *L)
{
    int teamside = lua_tointeger(L, 1);
    int clusternum = 0;
    if (teamside == TeamSide::kOur)
        clusternum = AIContext::getInstance()->current_ClusterNumOur;
    else if (teamside == TeamSide::kEnemy)
        clusternum = AIContext::getInstance()->current_ClusterNumEnemy;
    lua_pushinteger(L, clusternum);
    return 1;
}

static int luabind_aicontext_get_teamlist(lua_State *L)
{
    int teamlist_type = lua_tointeger(L, 1);
    int teamside = lua_tointeger(L, 2);
    int clusternum = lua_tointeger(L, 3);
    bool is_excludesummon = lua_toboolean(L, 4);

    CharacterDataListPtr list = AIContext::getInstance()->getTeamList(teamlist_type, teamside, clusternum, is_excludesummon);

    if (list) {
        lua_newtable(L);
        int tableidx = lua_gettop(L);
        int idx = 1;
        for (CharacterData *c : *list) {
            if (is_excludesummon && c->isSummon())
                continue;

            luastack_pushcharacterdata(L, c);
            lua_rawseti(L, tableidx, idx);
            idx++;
        }
    }
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_aicontext_delete_caster(lua_State *L)
{
    int character_myid = lua_tointeger(L, 1);
    int caster_type = lua_tointeger(L, 2);
    int caster_myid = lua_tointeger(L, 3);
    AIContext::getInstance()->deleteCaster(character_myid, caster_type, caster_myid);
    return 0;
}

static int luabind_aicontext_is_todeletecasterinfo_existed(lua_State *L)
{
    int caster_myid = lua_tointeger(L, 1);
    bool ret = AIContext::getInstance()->isToDeleteCasterInfoExisted(caster_myid);
    lua_pushboolean(L, ret);
    return 1;
}

static int luabind_aicontext_add_todeletecasterinfo(lua_State *L)
{
    int character_myid = lua_tointeger(L, 1);
    int caster_type = lua_tointeger(L, 2);
    int caster_myid = lua_tointeger(L, 3);
    AIContext::getInstance()->addToDeleteCasterInfo(character_myid, caster_type, caster_myid);
    return 0;
}

static int luabind_aicontext_cleanup_todeletecasterinfo(lua_State *L)
{
    AIContext::getInstance()->cleanupToDeleteCasterInfo();
    return 0;
}

static int luabind_aicontext_check_delete_todeletecasters(lua_State *L)
{
    AIContext::getInstance()->checkDeleteToDeleteCasters();
    return 0;
}

static int luabind_aicontext_hurt_shield(lua_State *L)
{
	AICharacterData *source = luastack_getcharacterdata(L, 1);
	AICharacterData *target = luastack_getcharacterdata(L, 2);
	HurtCalcParam hurtparam(0, 0, 0, 0, 0, 0);
	luastack_gethurtcalcparam(L, 3, hurtparam);

	std::vector<int> shieldids;

	int retidx = 4;
	if (lua_istable(L, retidx))
	{
		auto tablesize = lua_rawlen(L, retidx);
		for (int i = 1; i <= tablesize; i++)
		{
			lua_geti(L, retidx, i);
			int shieldid = lua_tointeger(L, -1);
			shieldids.push_back(shieldid);
			lua_pop(L, 1);
		}
	}

	int hurtnum = 0;
	HeroHurtcalculate::HurtShield(*AIContext::getInstance(), source, target, hurtparam, shieldids, hurtnum);
	lua_pushinteger(L, hurtparam.HPDelta);
	lua_pushinteger(L, hurtnum);
	return 2;
}

int luaopen_aicontext(lua_State *L)
{
    static const luaL_Reg reg[] = {
        //{ "instance", luabind_aicontext_get_instance },
        { "get_currenttick", luabind_aicontext_get_currenttick },
        { "get_randomnumber", luabind_aicontext_get_randomnumber },
        { "get_randomnumber_within_range", luabind_aicontext_get_randomnumber_within_range },
        { "get_battlefield_center_x", luabind_aicontext_get_battlefield_center_x },
        { "set_battlefield_center_x", luabind_aicontext_set_battlefield_center_x },
        { "get_cluster_range_to_centerx", luabind_aicontext_get_cluster_range_to_centerx },
        { "set_cluster_range_to_centerx", luabind_aicontext_set_cluster_range_to_centerx },
        { "get_delta_max_y", luabind_aicontext_get_delta_max_y },
        { "get_delta_min_y", luabind_aicontext_get_delta_min_y },
        { "get_businesstype", luabind_aicontext_get_businesstype },
        { "get_simu_calc_hurt", luabind_aicontext_get_simu_calc_hurt },
        { "hurtcalculate_triggermark", luabind_aicontext_hurtcalculate_triggermark },
        { "execute_exec", luabind_aicontext_execute_exec },
        { "handle_hurtresult", luabind_aicontext_handle_hurtresult },
        { "on_releasehpskill", luabind_aicontext_on_releasehpskill },
        //{ "hurttarget", luabind_aicontext_hurttarget},
        { "schedule_execute", luabind_aicontext_schedule_execute },
        { "get_string_from_file", luabind_aicontext_get_string_from_file },
        { "get_character_by_myid", luabind_aicontext_get_character_by_myid },
        { "get_autofight", luabind_aicontext_get_autofight },
        { "is_monster_can_move", luabind_aicontext_is_monster_can_move },
        { "trigger_marks_on_character_dead", luabind_aicontext_trigger_marks_on_character_dead },
        { "remove_marks_on_character_dead", luabind_aicontext_remove_marks_on_character_dead },
        { "on_event", luabind_aicontext_on_event },
        { "is_characterinbattlefield", luabind_aicontext_is_characterinbattlefield },
        { "get_is_fighting", luabind_aicontext_get_is_fighting },
        { "set_is_fighting", luabind_aicontext_set_is_fighting },
        { "add_event_listener", luabind_aicontext_add_event_listener },
        { "get_friend_alive_nums", luabind_aicontext_get_friend_alive_nums },
        { "get_rival_alive_nums", luabind_aicontext_get_rival_alive_nums },
        { "add_trackultra", luabind_aicontext_add_trackultra },
        { "pushfront_ultrafreeze", luabind_aicontext_pushfront_ultrafreeze },
        { "get_aicallback", luabind_aicontext_get_aicallback },
        { "get_adjustingdistance", luabind_aicontext_get_adjustingdistance },
        { "set_adjustingdistance", luabind_aicontext_set_adjustingdistance },
        { "has_winlose", luabind_aicontext_has_winlose },
        { "get_winlose", luabind_aicontext_get_winlose },
        { "set_winlose", luabind_aicontext_set_winlose },
        { "stop_ultrafreeze", luabind_aicontext_stop_ultrafreeze },
        { "get_configdata_json", luabind_aicontext_get_configdata_json },
        /// 12.0
        { "is_being_final_clear", luabind_aicontext_is_being_final_clear },
        ///
        /// 14.0
        { "add_character_to_cluster", luabind_aicontext_add_character_to_cluster },
        { "remove_character_from_cluster", luabind_aicontext_remove_character_from_cluster },
        { "check_update_alive_cnt", luabind_aicontext_check_update_alive_cnt },
        { "set_team_alive_info", luabind_aicontext_set_team_alive_info },
        { "get_team_alive_info", luabind_aicontext_get_team_alive_info },
        { "get_current_team_clusternum", luabind_aicontext_get_current_team_clusternum },
        { "get_teamlist", luabind_aicontext_get_teamlist },
        { "delete_caster", luabind_aicontext_delete_caster },
        { "is_todeletecasterinfo_existed", luabind_aicontext_is_todeletecasterinfo_existed },
        { "add_todeletecasterinfo", luabind_aicontext_add_todeletecasterinfo },
        { "cleanup_todeletecasterinfo", luabind_aicontext_cleanup_todeletecasterinfo },
        { "check_delete_todeletecasters", luabind_aicontext_check_delete_todeletecasters },
        ///
		/// 20.1
		{ "hurt_shield", luabind_aicontext_hurt_shield },
		///
        { NULL, NULL }
    };
    
    //static const luaL_Reg reg_m[] = {
    //    { "get_currenttick", luabind_aicontext_get_currenttick },
    //    { NULL, NULL }
    //};
    
    luaL_newmetatable(L, luatypename_aicontext());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    //luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    
    return 1;
}
    
    bool luahelper_aicontext_clear(lua_State *L)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_clear");
        
        if (lua_pcall(L, 0, 1, 0)) {
            luahelper_errorlog(L);
            return false;
        }

        bool luaval = lua_toboolean(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    bool luahelper_aicontext_clear_client(lua_State *L)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_clear_client");
        
        if (lua_pcall(L, 0, 1, 0)) {
            luahelper_errorlog(L);
            return false;
        }

        bool luaval = lua_toboolean(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    bool luahelper_aicontext_clear_server(lua_State *L)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_clear_server");
        
        if (lua_pcall(L, 0, 1, 0)) {
            luahelper_errorlog(L);
            return false;
        }

        bool luaval = lua_toboolean(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    void luahelper_aicontext_addcharacter(lua_State *L, CharacterData *character)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_addcharacter");
        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0)) {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_aicontext_addcaptain(lua_State *L, CharacterData *captain)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_addcaptain");
        luastack_pushcharacterdata(L, captain);

        if (lua_pcall(L, 1, 0, 0)) {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_aicontext_to_nextcluster_end(lua_State *L, int adjust, int centerx)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_to_nextcluster_end");
        lua_pushinteger(L, adjust);
        lua_pushinteger(L, centerx);

        if (lua_pcall(L, 2, 0, 0)) {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_aicontext_on_battle_finished(lua_State *L)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_battle_finished");

        if (lua_pcall(L, 0, 0, 0)) {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_aicontext_on_update_end(lua_State *L)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_update_end");

        if (lua_pcall(L, 0, 0, 0))
            luahelper_errorlog(L);
        
        lua_settop(L, 0);
    }

    void luahelper_aicontext_on_to_nextcluster_init_cluster(lua_State *L, int clusternum_our, int clusternum_enemy)
    {
        lua_getglobal(L, "g_aihelper");
        lua_getfield(L, -1, "on_to_nextcluster_init_cluster");

        lua_pushinteger(L, clusternum_our);
        lua_pushinteger(L, clusternum_enemy);
        if (lua_pcall(L, 2, 0, 0))
            luahelper_errorlog(L);

        lua_settop(L, 0);
    }

// MARK: Lua binding aicontext --

}
