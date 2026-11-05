//
//  AIScriptBindActionController.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindActionController.hpp"
#include "lua.hpp"
#include "AIScriptBindCharacterData.hpp"
#include "AIScriptBindProjectile.hpp"
#include "AIScriptBindHelper.h"
#include "../AIContext.h"
#include "../AICharacterData.h"
#include "../AIProjectile.h"
#include "../AITargetHelper.h"
#include "../action_hero/HeroAttackAction.h"
#include "../action_hero/HeroEffectAction.h"
#include "../action_hero/HeroDeadAction.h"
#include "../action_hero/HeroTurnAction.h"
#include "../action_hero/HeroOutofControlAction.h"
#include "../action_hero/HeroMoveAction.h"
#include "../action_hero/HeroActionController.h"
#include "../action_hero/HeroStruckFlyAction.h"
#include "../action_hero/HeroStruckBackAction.h"
#include "../action_hero/HeroStruckHoverAction.h"

namespace OnePiece {

static int luabind_aiactioncontroller_action_selector(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroActionController::actionSelectorNative(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_aiactioncontroller_captain_action_selector(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroActionController::captainActionSelectorNative(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_aiactioncontroller_exit_action(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroActionController::exitActionNative(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_aiactioncontroller_did_enter_action(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroActionController::didEnterActionNative(*character, *AIContext::getInstance());
    return 0;
}

static int luacallback_enter_deadaction(lua_State *L)
{
    auto the_character = luastack_getcharacterdata(L, 1);
    int the_deadtype = lua_tointeger(L, 2);
    bool is_fatal = lua_toboolean(L, 3);
    HeroDeadAction::EnterNative(*the_character, *AIContext::getInstance(), the_deadtype, is_fatal);
    return 0;
}

static int luacallback_force_enter_deadaction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroDeadAction::ForceEnterNative(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_aiactioncontroller_enter_turnaction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroTurnAction::Enter(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_aiactioncontroller_enter_idleaction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int ticklen = lua_tointeger(L, 2);
    auto attacker = luastack_getcharacterdata(L, 3);
    HeroOutofControlAction::Enter(*character, *AIContext::getInstance(), ticklen, ActionType::IDLE_ACTION, *attacker);
    return 0;
}

static int luabind_aiactioncontroller_enter_moveaction(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroMoveAction::Enter(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_aiactioncontroller_distance_to_character(lua_State *L)
{
    auto chara1 = luastack_getcharacterdata(L, 1);
    auto chara2 = luastack_getcharacterdata(L, 2);
    int dist = HeroActionController::distanceToCharacter(chara1, chara2);
    lua_pushinteger(L, dist);
    return 1;
}

static int luabind_aiactioncontroller_cleanup(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool ifhurteff = lua_toboolean(L, 2);
    HeroActionController::cleanup(*character, *AIContext::getInstance(), ifhurteff);
    return 0;
}

static int luabind_invalidatecaster(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int t = lua_tointeger(L, 2);
    int casterid = lua_tointeger(L, 3);
    AIActionController::invalidateCaster(c, t, casterid);
    return 0;
}

static int luabind_addtoremovecaster(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int t = lua_tointeger(L, 2);
    int casterid = lua_tointeger(L, 3);
    AIActionController::addToRemoveCaster(c, t, casterid);
    return 0;
}

static int luabind_attackaction_forceendaccu(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroAttackAction::ForceEndAccumulateNative(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_attackaction_enter(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int seqindex = lua_tointeger(L, 2);
    int skillid = lua_tointeger(L, 3);
    int skilllevel = lua_tointeger(L, 4);
    bool is_resetcd = lua_toboolean(L, 5);
    int tickoffset = lua_tointeger(L, 6);
    bool is_ultra = lua_toboolean(L, 7);
    HeroAttackAction::EnterNative(*character, *AIContext::getInstance(), seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra);
    return 0;
}

static int luabind_attackaction_force_enter(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int seqindex = lua_tointeger(L, 2);
    int skillid = lua_tointeger(L, 3);
    int skilllevel = lua_tointeger(L, 4);
    bool is_resetcd = lua_toboolean(L, 5);
    int tickoffset = lua_tointeger(L, 6);
    bool is_ultra = lua_toboolean(L, 7);
    HeroAttackAction::ForceEnterNative(*character, *AIContext::getInstance(), seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra);
    return 0;
}

static int luabind_attackaction_trigger_combo(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int skilllv = lua_tointeger(L, 3);
    bool luaret = HeroAttackAction::TriggerComboNative(*c, *AIContext::getInstance(), skillid, skilllv);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_attackaction_update(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroAttackAction::UpdateNative(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_attackaction_exit(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroAttackAction::ExitNative(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_deadaction_adjust_teampos(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool luaval = lua_toboolean(L, 2);
    HeroDeadAction::AdjustTeamPos(*character, *AIContext::getInstance(), luaval);
    return 0;
}

static int luabind_deadaction_claim_monster_dead_reward(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroDeadAction::ClaimMonsterDeadReward(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_deadaction_do_hero_dead(lua_State* L)
{
    auto character = luastack_getcharacterdata(L, 1);
    HeroDeadAction::DoHeroDead(*character, *AIContext::getInstance());
    return 0;
}

static int luabind_outofcontrolaction_enter(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int ticklen = lua_tointeger(L, 2);
    int actiontype = lua_tointeger(L, 3);
    auto attacker = luastack_getcharacterdata(L, 4);
    bool is_success = HeroOutofControlAction::Enter(*c, *AIContext::getInstance(), ticklen, ActionType(actiontype), *attacker);
	lua_pushboolean(L, is_success ? 1 : 0);
    return 1;
}

static int luabind_outofcontrolaction_exit(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroOutofControlAction::ExitNative(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_struckflyaction_exit(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroStruckFlyAction::Exit(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_struckbackaction_exit(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroStruckBackAction::Exit(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_struckhoveraction_exit(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroStruckHoverAction::Exit(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_effectaction_exit(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    HeroEffectAction::Exit(*c, *AIContext::getInstance());
    return 0;
}

static int luabind_enter_uieffectaction(lua_State *L)
{
	CharacterData *c = luastack_getcharacterdata(L, 1);
	int stateanim = lua_tointeger(L, 2);
	int r = lua_tointeger(L, 3);
	int g = lua_tointeger(L, 4);
	int b = lua_tointeger(L, 5);
	int shader = lua_tointeger(L, 6);
	AIContext *ai = AIContext::getInstance();
	if (ai->animationProxy) {

		ai->animationProxy->enterStateAnimation(*ai, c, stateanim, r, g, b, shader);
	}
	return 0;
}

static int luabind_exit_uieffectaction(lua_State *L)
{
	CharacterData *c = luastack_getcharacterdata(L, 1);
	int stateanim = lua_tointeger(L, 2);
	AIContext *ai = AIContext::getInstance();
	if (ai->animationProxy) {
		ai->animationProxy->exitStateAnimation(*ai, c, stateanim);
	}
	return 0;
}

static int luabind_aiactioncontroller_search_target_by_skill(lua_State *L)
{
    int skill_id = lua_tointeger(L, 1);
    CharacterData *skill_caster = luastack_getcharacterdata(L, 2);
    CharacterData *source = luastack_getcharacterdata(L, 3);
    CharacterData *target = luastack_getcharacterdata(L, 4);
    bool is_usingskill = lua_toboolean(L, 5);

    AISkill *skill_data = AIDataHelper::getSkillData(skill_id);
    std::list<CharacterData *> targets;

    if (skill_data) {
        AITargetContext ctx;
        ctx.source = source;
        ctx.target = target;
        ctx.skill_caster = skill_caster;
        ctx.projectile = nullptr;
        ctx.search_type = skill_data->targetType;
        ctx.range.x1 = -1 * skill_data->range;
        ctx.range.x2 = skill_data->range;
        ctx.range.y1 = -1 * skill_data->range;
        ctx.range.y2 = skill_data->range;
        ctx.search_params = &skill_data->vTargetParms;
        ctx.skill_id = skill_id;
        ctx.is_using_skill = is_usingskill;
        
        AITargetHelper::searchTarget(&ctx, &targets);
    }

    if (targets.size() > 0) {
        lua_createtable(L, (int)targets.size(), 0);
        int idx = lua_gettop(L);
        int len = 1;
        for (CharacterData *c: targets) {
            if (c) {
                luastack_pushcharacterdata(L, c);
                lua_rawseti(L, idx, len);
                len += 1;
            }
        }
    }
    else
        lua_pushnil(L);

    return 1;
}

int luaopen_aiactioncontroller(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "action_selector", luabind_aiactioncontroller_action_selector },
        { "captain_action_selector", luabind_aiactioncontroller_captain_action_selector },
        { "exit_action", luabind_aiactioncontroller_exit_action },
        { "did_enter_action", luabind_aiactioncontroller_did_enter_action },
        { "distance_to_character", luabind_aiactioncontroller_distance_to_character },
        { "cleanup", luabind_aiactioncontroller_cleanup },
        { "invalidatecaster", luabind_invalidatecaster },
        { "addtoremovecaster", luabind_addtoremovecaster },

        { "enter_turnaction", luabind_aiactioncontroller_enter_turnaction },

        { "enter_idleaction", luabind_aiactioncontroller_enter_idleaction },

        { "enter_moveaction", luabind_aiactioncontroller_enter_moveaction },

        { "attackaction_forceendaccu", luabind_attackaction_forceendaccu },
        { "attackaction_enter", luabind_attackaction_enter },
        { "attackaction_force_enter", luabind_attackaction_force_enter },
        { "attackaction_trigger_combo", luabind_attackaction_trigger_combo },
        { "attackaction_update", luabind_attackaction_update },
        { "attackaction_exit", luabind_attackaction_exit },

        { "enter_deadaction", luacallback_enter_deadaction },
        { "force_enter_deadaction", luacallback_force_enter_deadaction },
        { "deadaction_adjust_teampos", luabind_deadaction_adjust_teampos },
        { "deadaction_claim_monster_dead_reward", luabind_deadaction_claim_monster_dead_reward },
        { "deadaction_do_hero_dead", luabind_deadaction_do_hero_dead },

        { "outofcontrolaction_enter", luabind_outofcontrolaction_enter },
        { "outofcontrolaction_exit", luabind_outofcontrolaction_exit },

        { "struckflyaction_exit", luabind_struckflyaction_exit },

        { "struckbackaction_exit", luabind_struckbackaction_exit },

        { "struckhoveraction_exit", luabind_struckhoveraction_exit },

        { "effectaction_exit", luabind_effectaction_exit },
		{ "enter_uieffectaction", luabind_enter_uieffectaction },
		{ "exit_uieffectaction", luabind_exit_uieffectaction },

        { "search_target_by_skill", luabind_aiactioncontroller_search_target_by_skill },
        { NULL, NULL }
    };
    
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}
    
    void luahelper_searchtarget(lua_State *L, CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, CharacterDataList *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, std::list<CharacterData *> *outTargets)
    {
        lua_getglobal(L, "g_targethelper");
        lua_getfield(L, -1, "search");
        
        lua_createtable(L, 20, 0);
        auto table_idx = lua_gettop(L);
        lua_pushstring(L, "source");
        if (&source != nullptr)
            luastack_pushcharacterdata(L, &source);
        else
            lua_pushnil(L);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "target");
        if (target)
            luastack_pushcharacterdata(L, target);
        else
            lua_pushnil(L);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "projectile");
        if (projectile)
            luastack_pushprojectile(L, projectile);
        else
            lua_pushnil(L);
        lua_settable(L, table_idx);
        
        // lua_pushstring(L, "aicontext");
        // lua_pushnil(L);
        // lua_settable(L, table_idx);
        
        lua_pushstring(L, "search_type");
        lua_pushinteger(L, searchType);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "x1");
        lua_pushinteger(L, x1);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "x2");
        lua_pushinteger(L, x2);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "y1");
        lua_pushinteger(L, y1);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "y2");
        lua_pushinteger(L, y2);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "z1");
        lua_pushinteger(L, z1);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "z2");
        lua_pushinteger(L, z2);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "search_params");
        if (searchParams && searchParams->size() > 0)
        {
            lua_newtable(L);
            int paramidx = lua_gettop(L);
            for (int i = 1; i <= searchParams->size(); i++)
            {
                lua_pushinteger(L, (*searchParams)[i - 1]);
                lua_rawseti(L, paramidx, i);
            }
        }
        else
        {
            lua_pushnil(L);
        }
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "skillcaster");
        if (&skillCaster)
            luastack_pushcharacterdata(L, &skillCaster);
        else
            lua_pushnil(L);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "is_usingskill");
        lua_pushboolean(L, isUsingSkill);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "skill_id");
        lua_pushinteger(L, theSkillId);
        lua_settable(L, table_idx);
        
        //exclude_character, is_hpratiohtexcludecharacter
        lua_pushstring(L, "hurt_id");
        lua_pushinteger(L, hurtID);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "skill_level");
        lua_pushinteger(L, skillLevel);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "filter_list");
        if (filterList && filterList->size() > 0)
        {
            lua_newtable(L);
            int filteridx = lua_gettop(L);
            int i = 1;
            for (auto a_filter_character : *filterList)
            {
                luastack_pushcharacterdata(L, a_filter_character);
                lua_rawseti(L, filteridx, i);
                i++;
            }
        }
        else
        {
            lua_pushnil(L);
        }
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "exclude_character");
        if (excludeCharacter)
            luastack_pushcharacterdata(L, excludeCharacter);
        else
            lua_pushnil(L);
        lua_settable(L, table_idx);
        
        lua_pushstring(L, "is_hpratiohtexcludecharacter");
        lua_pushboolean(L, shouldHpRatioHigherThanExcludeCharacter);
        lua_settable(L, table_idx);
        
        if (lua_pcall(L, 1, 1, 0))
        {
            luahelper_errorlog(L);
        }
        
        auto retidx = lua_gettop(L);
        if (retidx > 0 && lua_istable(L, retidx))
        {
            auto tablesize = lua_rawlen(L, retidx);
            for (int i = 1; i <= tablesize; i++)
            {
                lua_geti(L, retidx, i);
                CharacterData **lua_userdata = static_cast<CharacterData **>(lua_touserdata(L, -1));
                auto a_target = *lua_userdata;
                outTargets->push_back(a_target);
                lua_pop(L, 1);
            }
        }
        
        lua_settop(L, 0);
    }

    void luahelper_searchtargetwith(lua_State *L, CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, std::list<CharacterData *> *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, std::list<CharacterData *> *outTargets)
    {
        lua_getglobal(L, "g_targethelper");
        lua_getfield(L, -1, "search_with");

        // 0-skillcaster, 1-source, 2-target, 3-projectile, 4-search_type, 5-x1, 6-x2, 7-y1, 8-y2, 9-z1, 10-z2, 11-search_params, 12-is_usingskill, 13-skill_id, 14-skill_level, 15-hurt_id, 16-filter_list, 17-exclude_character, 18-is_hpratiohtexcludecharacter
        if (&skillCaster)
            luastack_pushcharacterdata(L, &skillCaster);
        else
            lua_pushnil(L);

        if (&source)
            luastack_pushcharacterdata(L, &source);
        else
            lua_pushnil(L);

        if (target)
            luastack_pushcharacterdata(L, target);
        else
            lua_pushnil(L);

        if (projectile)
            luastack_pushprojectile(L, projectile);
        else
            lua_pushnil(L);

        lua_pushinteger(L, searchType);
        lua_pushinteger(L, x1);
        lua_pushinteger(L, x2);
        lua_pushinteger(L, y1);
        lua_pushinteger(L, y2);
        lua_pushinteger(L, z1);
        lua_pushinteger(L, z2);
        if (searchParams && searchParams->size() > 0)
        {
            lua_newtable(L);
            int paramidx = lua_gettop(L);
            for (int i = 1; i <= searchParams->size(); i++)
            {
                lua_pushinteger(L, (*searchParams)[i - 1]);
                lua_rawseti(L, paramidx, i);
            }
        }
        else
        {
            lua_pushnil(L);
        }
        lua_pushboolean(L, isUsingSkill);
        lua_pushinteger(L, theSkillId);
        lua_pushinteger(L, skillLevel);
        lua_pushinteger(L, hurtID);
        if (filterList && filterList->size() > 0)
        {
            lua_newtable(L);
            int filteridx = lua_gettop(L);
            int i = 1;
            for (auto a_filter_character : *filterList)
            {
                luastack_pushcharacterdata(L, a_filter_character);
                lua_rawseti(L, filteridx, i);
                i++;
            }
        }
        else
        {
            lua_pushnil(L);
        }
        if (excludeCharacter)
            luastack_pushcharacterdata(L, excludeCharacter);
        else
            lua_pushnil(L);
        lua_pushboolean(L, shouldHpRatioHigherThanExcludeCharacter);

        if (lua_pcall(L, 19, 1, 0))
        {
            luahelper_errorlog(L);
        }

        auto retidx = lua_gettop(L);
        if (retidx > 0 && lua_istable(L, retidx))
        {
            auto tablesize = lua_rawlen(L, retidx);
            for (int i = 1; i <= tablesize; i++)
            {
                lua_geti(L, retidx, i);
                CharacterData **lua_userdata = static_cast<CharacterData **>(lua_touserdata(L, -1));
                auto a_target = *lua_userdata;
                outTargets->push_back(a_target);
                lua_pop(L, 1);
            }
        }

        lua_settop(L, 0);
    }
    
    void luahelper_searchtargetwith(lua_State *L, AITargetContext *ctx, std::list<CharacterData *> *out_targets)
    {
        lua_getglobal(L, "g_targethelper");
        lua_getfield(L, -1, "search_with_ctx");

        if (lua_isnil(L, -1)) {
            lua_settop(L, 0);
            return;
        }

        lua_createtable(L, 17, 0);

        int ctx_idx = lua_gettop(L);
        int idx = 0;
        
        // 1-skillcaster
        idx++;
        if (ctx->skill_caster)
            luastack_pushcharacterdata(L, ctx->skill_caster);
        else
            lua_pushnil(L);
        lua_rawseti(L, ctx_idx, idx);

        // 2-source
        idx++;
        if (ctx->source)
            luastack_pushcharacterdata(L, ctx->source);
        else
            lua_pushnil(L);
        lua_rawseti(L, ctx_idx, idx);

        // 3-target
        idx++;
        if (ctx->target)
            luastack_pushcharacterdata(L, ctx->target);
        else
            lua_pushnil(L);
        lua_rawseti(L, ctx_idx, idx);
        
        // 4-projectile
        idx++;
        if (ctx->projectile)
            luastack_pushprojectile(L, ctx->projectile);
        else
            lua_pushnil(L);
        lua_rawseti(L, ctx_idx, idx);

        // 5-search_type
        idx++;
        lua_pushinteger(L, ctx->search_type);
        lua_rawseti(L, ctx_idx, idx);
        
        // 6-x1
        idx++;
        lua_pushinteger(L, ctx->range.x1);
        lua_rawseti(L, ctx_idx, idx);
        
        // 7-x2
        idx++;
        lua_pushinteger(L, ctx->range.x2);
        lua_rawseti(L, ctx_idx, idx);
        
        // 8-y1
        idx++;
        lua_pushinteger(L, ctx->range.y1);
        lua_rawseti(L, ctx_idx, idx);
        
        // 9-y2
        idx++;
        lua_pushinteger(L, ctx->range.y2);
        lua_rawseti(L, ctx_idx, idx);
        
        // 10-search_params
        idx++;
        if (ctx->search_params && ctx->search_params->size()) {
            lua_newtable(L);
            int param_idx = lua_gettop(L);
            for (int i = 1; i <= ctx->search_params->size(); i++) {
                lua_pushinteger(L, (*ctx->search_params)[i - 1]);
                lua_rawseti(L, param_idx, i);
            }
        } else {
            lua_pushnil(L);
        }
        lua_rawseti(L, ctx_idx, idx);
        
        // 11-is_usingskill
        idx++;
        lua_pushboolean(L, ctx->is_using_skill);
        lua_rawseti(L, ctx_idx, idx);
        
        // 12-skill_id
        idx++;
        lua_pushinteger(L, ctx->skill_id);
        lua_rawseti(L, ctx_idx, idx);
        
        // 13-skill_level
        idx++;
        lua_pushinteger(L, ctx->skill_level);
        lua_rawseti(L, ctx_idx, idx);
        
        // 14-hurt_id
        idx++;
        lua_pushinteger(L, ctx->hurt_id);
        lua_rawseti(L, ctx_idx, idx);
        
        // 15-filter_list
        idx++;
        if (ctx->filter_list && ctx->filter_list->size() > 0) {
            lua_newtable(L);
            int filter_idx = lua_gettop(L);
            int i = 1;
            for (auto a_filter_character : *ctx->filter_list) {
                luastack_pushcharacterdata(L, a_filter_character);
                lua_rawseti(L, filter_idx, i);
                i++;
            }
        } else {
            lua_pushnil(L);
        }
        lua_rawseti(L, ctx_idx, idx);
        
        // 16-exclude_character
        idx++;
        if (ctx->exclude_character)
            luastack_pushcharacterdata(L, ctx->exclude_character);
        else
            lua_pushnil(L);
        lua_rawseti(L, ctx_idx, idx);
        
        // 17-is_hpratiohtexcludecharacter
        idx++;
        lua_pushboolean(L, ctx->should_hp_ratio_higher_than_exclude_character);
        lua_rawseti(L, ctx_idx, idx);

        if (lua_pcall(L, 1, 1, 0)) {
            luahelper_errorlog(L);
        } else {
            auto retidx = lua_gettop(L);
            if (retidx > 0 && lua_istable(L, retidx)) {
                auto tablesize = lua_rawlen(L, retidx);
                for (int i = 1; i <= tablesize; i++) {
                    lua_rawgeti(L, retidx, i);
                    auto t = luastack_getcharacterdata(L, -1);
                    if (t)
                        out_targets->push_back(t);
                    lua_pop(L, 1);
                }
            }
        }
        
        lua_settop(L, 0);
    }

    bool luahelper_targethelper_is_target297findabletosearcher(lua_State *L, CharacterData *c, CharacterData *searcher, int realsearchtype)
    {
        lua_getglobal(L, "g_targethelper");
        lua_getfield(L, -1, "is_target297findabletosearcher");

        luastack_pushcharacterdata(L, c);
        luastack_pushcharacterdata(L, searcher);
        lua_pushinteger(L, realsearchtype);

        if (lua_pcall(L, 3, 1, 0))
        {
            luahelper_errorlog(L);
            return false;
        }

        bool luaval = lua_toboolean(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    bool luahelper_actioncontroller_execute_exec(lua_State *L, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int markID)
    {
        lua_getglobal(L, "g_exechelper");
        lua_getfield(L, -1, "on_aiactionconroller_execute_exec");
        
        lua_pushinteger(L, execID);
        lua_pushinteger(L, skillID);
        lua_pushinteger(L, skillLevel);
        luastack_pushcharacterdata(L, source);
        luastack_pushprojectile(L, projectile);
        luastack_pushcharacterdata(L, target);
        lua_pushinteger(L, parentTicklen);
        lua_pushboolean(L, useLogicPos);
        lua_pushinteger(L, markID);
        
        bool luaret = false;
        if (lua_pcall(L, 9, 1, 0))
        {
            luahelper_errorlog(L);
            return false;
        }
        
        luaret = lua_toboolean(L, -1);
        lua_settop(L, 0);
        return luaret;
    }

    void luahelper_actioncontroller_exit_action(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "exit_action");
        
        luastack_pushcharacterdata(L, c);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_actioncontroller_did_enter_action(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "did_enter_action");
        
        luastack_pushcharacterdata(L, c);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }
    
    void luahelper_actioncontroller_on_actionselector_turntotarget(lua_State *L, CharacterData *character)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "on_action_selector_turntotarget");

        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_actioncontroller_on_moveaction_turntotarget(lua_State *L, CharacterData *character)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "on_moveaction_turntotarget");

        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_actioncontroller_enter_deadaction(lua_State *L, CharacterData *character, int deadType, bool isFatal)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "enter_deadaction");

        luastack_pushcharacterdata(L, character);
        lua_pushinteger(L, deadType);
        lua_pushboolean(L, isFatal);

        if (lua_pcall(L, 3, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_actioncontroller_force_enter_deadaction(lua_State *L, CharacterData *character)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "force_enter_deadaction");

        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    void luahelper_actioncontroller_action_selector(lua_State *L, CharacterData *character)
    {
        int ltop = lua_gettop(L);
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "action_selector");
        lua_remove(L, -2);

        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        if (lua_gettop(L) != ltop) {
            luaL_error(L, "stack not balanced:before,%d,now,%d", ltop, lua_gettop(L));
        }
        
        lua_settop(L, 0);
    }

    void luahelper_actioncontroller_captain_action_selector(lua_State *L, CharacterData *character)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "captain_action_selector");

        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    bool luahelper_actioncontroller_is_escape_effect_stateanim(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "is_escape_effect_stateanim");

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

    bool luahelper_actioncontroller_is_escape_hurt_result(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "is_escape_hurt_result");

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

    void luahelper_attackaction_update(lua_State *L, CharacterData *character)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_getfield(L, -1, "update");

        luastack_pushcharacterdata(L, character);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }
    
    void luahelper_attackaction_enter(lua_State *L, CharacterData *c, int seqindex, int skid, int sklv, bool is_resetcd, int tickoffset, bool is_ultra)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_getfield(L, -1, "enter");
        
        luastack_pushcharacterdata(L, c);
        lua_pushinteger(L, seqindex);
        lua_pushinteger(L, skid);
        lua_pushinteger(L, sklv);
        lua_pushboolean(L, is_resetcd);
        lua_pushinteger(L, tickoffset);
        lua_pushboolean(L, is_ultra);
        
        if (lua_pcall(L, 7, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    int luahelper_attackaction_cast_ultra(lua_State *L, CharacterData *character, bool is_external)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_getfield(L, -1, "cast_ultra");

        luastack_pushcharacterdata(L, character);
        lua_pushboolean(L, is_external);

        if (lua_pcall(L, 2, 1, 0))
        {
            luahelper_errorlog(L);
            return 0;
        }

        int luaval = lua_tointeger(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    int luahelper_attackaction_check_target(lua_State *L, CharacterData *character, int castultraskill)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_getfield(L, -1, "check_target");

        luastack_pushcharacterdata(L, character);
        lua_pushinteger(L, castultraskill);

        if (lua_pcall(L, 2, 1, 0))
        {
            luahelper_errorlog(L);
            return 0;
        }

        int luaval = lua_tointeger(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    int luahelper_attackaction_can_cast_ultra(lua_State *L, CharacterData *character, bool is_external)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_remove(L, -2);
        lua_getfield(L, -1, "can_cast_ultra");
        lua_remove(L, -2);

        luastack_pushcharacterdata(L, character);
        lua_pushboolean(L, is_external);

        if (lua_pcall(L, 2, 1, 0))
        {
            luahelper_errorlog(L);
            return 0;
        }

        int luaval = lua_tointeger(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    void luahelper_attackaction_on_exit(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_getfield(L, -1, "on_exit");

        luastack_pushcharacterdata(L, c);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    bool luahelper_attackaction_force_end_accu(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_getfield(L, -1, "force_end_accu");

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

    void luahelper_outofcontrolaction_on_exit(lua_State *L, CharacterData *c)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "outofcontrolaction");
        lua_getfield(L, -1, "on_exit");

        luastack_pushcharacterdata(L, c);

        if (lua_pcall(L, 1, 0, 0))
        {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

	int luahelper_actioncontroller_is_move_action_effect(lua_State *L, CharacterData* mover, int theMoveType)
	{
		lua_getglobal(L, "g_actionhelper");
		lua_getfield(L, -1, "ischaractermovedcantrigger");

		luastack_pushcharacterdata(L, mover);
		lua_pushinteger(L, theMoveType); // 0-移动执行 1-击飞状态 2-击倒状态 3-击退状态 

		
		if (lua_pcall(L, 2, 1, 0))
		{
			luahelper_errorlog(L);
			return false;
		}

		int luaval = lua_tointeger(L, -1);
        lua_settop(L, 0);
        return luaval;
	}

    int luahelper_attackaction_force_cast_ultra(lua_State *L, CharacterData *c, int jx)
    {
        lua_getglobal(L, "g_actionhelper");
        lua_getfield(L, -1, "attackaction");
        lua_remove(L, -2);
        lua_getfield(L, -1, "force_cast_ultra");
        lua_remove(L, -2);

        if (lua_isnil(L, -1)) {
            lua_settop(L, 0);
            return -1;
        }

        luastack_pushcharacterdata(L, c);
        lua_pushinteger(L, jx);

        if (lua_pcall(L, 2, 1, 0))
        {
            luahelper_errorlog(L);
            return -1;
        }

        int luaval = lua_tointeger(L, -1);
        lua_settop(L, 0);
        return luaval;
    }
	
} // namespace
