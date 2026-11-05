//
//  AIScriptBindEffectCaster.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindEffectCaster.hpp"
#include "lua.hpp"
#include "AIScriptBindHelper.h"
#include "AIScriptBindCharacterData.hpp"
#include "AIScriptBindProjectile.hpp"
#include "AIScriptBindHurtCalculate.hpp"
#include "../AIContext.h"
#include "../AICharacterData.h"
#include "../AIContinuousEffectCaster.h"

namespace OnePiece {
    
// MARK: -- Bind EffectCaster

static inline const char *luatypename_effectcaster() { return "aieffectcaster"; }

void luastack_pusheffectcaster(lua_State *L, AIContinuousEffectCaster *acaster)
{
    size_t sz = sizeof(AIContinuousEffectCaster *);
    AIContinuousEffectCaster **the_fulluserdata = static_cast<AIContinuousEffectCaster **>(lua_newuserdata(L, sz));
    *the_fulluserdata = acaster;

    luaL_getmetatable(L, luatypename_effectcaster());
    lua_setmetatable(L, -2);
}

AIContinuousEffectCaster *luastack_geteffectcaster(lua_State *L, int idx)
{
    if (lua_isnil(L, idx))
        return nullptr;
    AIContinuousEffectCaster **lua_userdata = static_cast<AIContinuousEffectCaster **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luabind_check_exert_special_effect(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    int etype = lua_tointeger(L, 2);
    int param = lua_tointeger(L, 3);
    int retval = AIContinuousEffectCaster::check_exert_Special_Effect(*AIContext::getInstance(), *a_character, etype, param);
    lua_pushinteger(L, retval);
    return 1;
}

static int luabind_check_do_float(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int effid = lua_tointeger(L, 2);
    float baseCHP = lua_tonumber(L, 3);
    float luaret = EffectHelper::checkExcuteFloatEffect(target, effid, baseCHP);
    lua_pushnumber(L, luaret);
    return 1;
}

static int luabind_check_exert_mixchangetype_effect(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    int etype = lua_tointeger(L, 2);
    int param = lua_tointeger(L, 3);
    int luaret = EffectHelper::checkExcuteMixChangeTypeEff(a_character, etype, param);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_zero_special_effect(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int etype = lua_tointeger(L, 2);
    AIContinuousEffectCaster::zero_Special_Effect(*AIContext::getInstance(), *character, etype);
    return 0;
}
    
static int luabind_change_special_effect(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int etype = lua_tointeger(L, 2);
    int changeval = lua_tointeger(L, 3);
    AIContinuousEffectCaster::change_Special_Effect(*AIContext::getInstance(), *character, etype, changeval);
    return 0;
}

static int luabind_is_anyinnerexec_incd(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int hitid = lua_tointeger(L, 2);
    bool luaret = EffectHelper::isAnyInnerExecInCD(target, hitid);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_start_innereffect_cd(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    auto aneff = luastack_getinnereffect(L, 2);
    AIContinuousEffectCaster::startEffectCD(*character, aneff->innerExec);
    return 0;
}

static int luabind_apply_attr_swap_effect(lua_State *L)
{
    int uid = lua_tointeger(L, 1);
    int attridx = lua_tointeger(L, 2);
    auto c = luastack_getcharacterdata(L, 3);
    int attrval = lua_tointeger(L, 4);

    EffectHelper::applyAttrSwapEffect(uid, attridx, c, attrval);
    return 0;
}

static int luabind_deapply_attr_effect(lua_State *L)
{
    int uid = lua_tointeger(L, 1);
    int attridx = lua_tointeger(L, 2);
    auto c = luastack_getcharacterdata(L, 3);
    EffectHelper::deapplyAttrEffect(uid, attridx, c);
    return 0;
}

static int luabind_innereffect_apply_hpchange(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto proj = luastack_getprojectile(L, 2);
    auto target = luastack_getcharacterdata(L, 3);
    int skillid = lua_tointeger(L, 4);
    int skilllevel = lua_tointeger(L, 5);
    int attridx = lua_tointeger(L, 6);
    int changeval = lua_tointeger(L, 7);
    int markid = lua_tointeger(L, 8);
    int casterid = lua_tointeger(L, 9);
    auto eff = luastack_getinnereffect(L, 10);
    int luaret = EffectHelper::applyHPChangeNative(*AIContext::getInstance(), source, proj, target, skillid, skilllevel, attridx, changeval, eff->innerExec, markid, casterid, eff);
    lua_pushinteger(L, luaret);
    return 1;
}
    
static int luabind_kill_follow_skill_tick_effect(lua_State *L)
{
    auto t = luastack_getcharacterdata(L, -1);
    if (t)
        AIContinuousEffectCaster::killFollowSkillEffect(*AIContext::getInstance(), t);
    return 0;
}

static int luabind_dispel_innereffect(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int parenteffectcasterid = lua_tointeger(L, 2);
    int innereffectid = lua_tointeger(L, 3);
    AIContinuousEffectCaster::dispelInnerEffect(*AIContext::getInstance(), c, parenteffectcasterid, innereffectid);
    return 0;
}

static int luabind_innereffect_apply_nonehpchange(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int attridx = lua_tointeger(L, 2);
    int val = lua_tointeger(L, 3);
    int luaret = EffectHelper::applyNoneHPChange(c, attridx, val);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_is_caster_group_covered(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int execid = lua_tointeger(L, 2);
    bool retval = AIContinuousEffectCaster::isCasterGroupCovered(c, execid);
    lua_pushboolean(L, retval);
    return 1;
}

static int luabind_check_cover_group_casters(lua_State *L)
{
    CharacterData *target = luastack_getcharacterdata(L, 1);
    int execid = lua_tointeger(L, 2);
    bool iscleaninnereffect = lua_toboolean(L, 3);
    int ret = AIContinuousEffectCaster::checkCoverGroupCasters(*AIContext::getInstance(), target, execid, iscleaninnereffect);
    lua_pushinteger(L, ret);
    return 1;
}

static int luabind_get_effect_value(lua_State *L)
{
    CharacterData *c = luastack_getcharacterdata(L, 1);
    int execid = lua_tointeger(L, 2);
    int param = lua_tointeger(L, 3);
    int ret = AIContinuousEffectCaster::getEffectValue(c, execid, param);
    lua_pushinteger(L, ret);
    return 1;
}

static int luabind_get_stateanimation_result(lua_State *L)
{
    AIContinuousEffectCaster *caster = luastack_geteffectcaster(L, 1);
    CharacterData *t = luastack_getcharacterdata(L, 2);
    int ret = caster->getStateAnimationResult(t);
    lua_pushinteger(L, ret);
    return 1;
}

static int luabind_cast_innereffect_by_idx(lua_State *L)
{
    auto e = luastack_geteffectcaster(L, 1);
    int idx = lua_tointeger(L, 2);
    auto c = luastack_getcharacterdata(L, 3);
    if (e) e->castInnerEffectByIdx(idx, c);
    return 0;
}

static int luabind_effectcaster_cleanup_innereffect(lua_State *L)
{
    AIContinuousEffectCaster *e = luastack_geteffectcaster(L, 1);
    if (e)
        e->cleanupInnerEffect();
    return 0;
}

static int luabind_eff172_has_activated_eff(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    long worldtick = lua_tointeger(L, 2);
    auto source = luastack_getcharacterdata(L, 3);
    bool luaret = AIFatalHurtTransferEffect::getActivatedFatalTransEff(target, worldtick, source->myID);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_eff172_get_valid_eff(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int t = lua_tointeger(L, 2);
    auto s = luastack_getcharacterdata(L, 3);
    AIInnerEffect *e = AIFatalHurtTransferEffect::getValidFatalTransferEffect(c, t, s->myID);
    if (e) luastack_pushinnereffect(L, e);
    else lua_pushnil(L);
    return 1;
}

static int luabind_eff172_get_activated_eff(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int t = lua_tointeger(L, 2);
    auto s = luastack_getcharacterdata(L, 3);
    AIInnerEffect *e = AIFatalHurtTransferEffect::getActivatedFatalTransEff(c, t, s->myID);
    if (e) luastack_pushinnereffect(L, e);
    else lua_pushnil(L);
    return 1;
}

static int luabind_eff172_trigger(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int t = lua_tointeger(L, 2);
    auto s = luastack_getcharacterdata(L, 3);
    AIFatalHurtTransferEffect *e = (AIFatalHurtTransferEffect *)luastack_getinnereffect(L, 4);
    AIFatalHurtTransferEffect::activateFatalTransferEffect(c, t, s->myID, e);
    return 0;
}

static int luabind_eff172_get_unactivated_eff(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int t = lua_tointeger(L, 2);
    auto s = luastack_getcharacterdata(L, 3);
    AIInnerEffect *e = AIFatalHurtTransferEffect::getUnactivatedFatalTransEff(c, t, s->myID);
    if (e) luastack_pushinnereffect(L, e);
    else lua_pushnil(L);
    return 1;
}

static int luabind_trigger_178(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    bool is_simu = lua_toboolean(L, 3);
    bool luaret = AILinkenSphereEffect::activateLinkenSphereEffect(source, target, is_simu);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_trigger_179(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int hpdelta = lua_tointeger(L, 2);
    bool is_simu = lua_toboolean(L, 3);
    int luaret = AIFlimsyBuryingEffect::activateFilmsyBuringEffect(target, hpdelta, is_simu);
    lua_pushinteger(L, luaret);
    return 1;
}
    
static int luabind_eff180_get_firstvalid_eff(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    auto eff = AIShareHurtEffect::getFirstValidShareHurt(c);
    if (eff)
        luastack_pushinnereffect(L, eff);
    else
        lua_pushnil(L);
    return 1;
}
    
static int luabind_eff180_trigger(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    AIShareHurtEffect::activateShareHurt(AIShareHurtEffect::getFirstValidShareHurt(c));
    return 0;
}

static int luabind_try_trigger_181(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    auto luaret = AICritHitNoFailEffect::checkActivateCritHitNoFailEffect(a_character);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_trigger_181(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    bool luaval = lua_toboolean(L, -1);
    AICritHitNoFailEffect::activateCritHitNoFailEffect(a_character, luaval);
    return 0;
}

static int luabind_get_eff_183_rangeskillindex(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int luaret = AIPickRangeSkillEffect::getRangeSkillIndex(character);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_trigger_202(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    AIEscMissExcuteEffect::checkActivateEscMissExcute(a_character, 202);
    return 0;
}

static int luabind_trigger_203(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    AIEscMissExcuteEffect::checkActivateEscMissExcute(a_character, 203);
    return 0;
}

static int luabind_trigger_204(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    AIEscMissExcuteEffect::checkActivateEscMissExcute(a_character, 204);
    return 0;
}
    
static int luabind_trigger_211(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    auto reboundtarget = luastack_getcharacterdata(L, 2);
    int skillid = lua_tointeger(L, 3);
    int skilllevel = lua_tointeger(L, 4);
    EffectHelper::activateResistNReboundEff(*AIContext::getInstance(), target, reboundtarget, skillid, skilllevel);
    return 0;
}

static int luabind_get_eff_214_altattackseq(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool isuseseq2 = lua_toboolean(L, 2);
    auto altseq = AIAttackListAddSomeOneEffect::getAlternateAttackSeq(character, isuseseq2);
    if (altseq)
    {
        lua_createtable(L, altseq->size(), 0);
        int idx = lua_gettop(L);
        int i = 1;
        for (auto seqindex : *altseq)
        {
            lua_pushinteger(L, seqindex);
            lua_rawseti(L, idx, i);
            i++;
        }
    }
    else
    {
        lua_pushnil(L);
    }
    return 1;
}

static int luabind_can_trigger_247(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int hpdelta = lua_tointeger(L, 2);
    bool luaret = AIResistFatalHurtNCastEffect::canCast(*AIContext::getInstance(), target, hpdelta);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_trigger_247(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    AIResistFatalHurtNCastEffect::checkExcute(*AIContext::getInstance(), target);
    return 0;
}

static int luabind_trigger_271(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    AIEffect271::checkExecute(*AIContext::getInstance(), target);
    return 0;
}

static int luabind_can_trigger_272(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    int hpdelta = lua_tointeger(L, 2);
    bool luaret = AIEffect272::canCast(*AIContext::getInstance(), a_character, hpdelta);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_trigger_272(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    AIEffect272::checkExecute(*AIContext::getInstance(), a_character);
    return 0;
}

static int luabind_can_clean_272(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    bool luaret = AIEffect272::checkZeroEffect(*AIContext::getInstance(), a_character);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_trigger_273(lua_State *L)
{
    auto a_character = luastack_getcharacterdata(L, 1);
    int param = lua_tointeger(L, 2);
    int luaret = AIEffect273::checkExecute(*AIContext::getInstance(), a_character, param);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_trigger_161(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    AIContinuousEffectCaster::executeOnHealTriggerEffect(*AIContext::getInstance(), source, target);
    return 0;
}

static int luabind_check_do_repeleff(lua_State *L)
{
    int effid = lua_tointeger(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    int hpdelta = lua_tointeger(L, 3);
    bool is_simu = lua_toboolean(L, 4);
    int luaret = AIRepelEffect::checkExecute(effid, *AIContext::getInstance(), target, hpdelta, is_simu);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_174(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    int hurtprop = lua_tointeger(L, 2);
    int hpdelta = lua_tointeger(L, 3);
    bool is_simu = lua_toboolean(L, 4);
    int luaret = EffectHelper::checkExcuteCauseAddPropertyHurtResult(source, hurtprop, hpdelta, is_simu);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_207(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int hurtprop = lua_tointeger(L, 2);
    int hpdelta = lua_tointeger(L, 3);
    int luaret = EffectHelper::checkActivatePropHurtAdd(character, hurtprop, hpdelta);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_213(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool luaret = AIAttackIgnoreNoDieEffect::checkExcute(*AIContext::getInstance(), character);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_check_do_217(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    int hpdelta = lua_tointeger(L, 3);
    int luaret = EffectHelper::getAddHurtOnDistChangeValue(*AIContext::getInstance(), source, target, hpdelta);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_218(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    int hpdelta = lua_tointeger(L, 3);
    int hurttype = lua_tointeger(L, 4);
    int luaret = AIAddHurtOnSameTargetEffect::checkExcute(source, target, hpdelta, hurttype);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_221(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    bool luaret = AICritOnAttrEffect::checkExcute(source, target);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_check_do_230(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    AIExcuteOnHurtResultEffect::checkExcute(character);
    return 0;
}

static int luabind_check_do_231(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    std::vector<int> seq;
    if (!lua_isnil(L, 2))
    {
        auto seqlen = lua_rawlen(L, 2);
        for (int i = 1; i <= seqlen; i++)
        {
            lua_rawgeti(L, 2, i);
            seq.push_back(lua_tointeger(L, -1));
            lua_pop(L, 1);
        }
    }
    int currentindex = lua_tointeger(L, 3);
    int luaret = AISkipSkillSeqEffect::checkExcute(character, seq, currentindex);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_239(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    auto aneff = AISlipCastEffect::checkExcute(*AIContext::getInstance(), character);
    if (aneff)
        luastack_pushinnereffect(L, aneff);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_check_do_240(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    int luaret = AISkipCDCastEffect::checkExcute(*AIContext::getInstance(), character);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_242(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    HurtCalcParam hurtparam(0, 0, 0, 0, 0, 0);
    int err = luastack_gethurtcalcparam(L, 3, hurtparam);
    assert(err == 0);
    int luaret = AISacrificeHurtSend::checkExcute(*AIContext::getInstance(), source, target, hurtparam);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_254(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    int hurttype = lua_tointeger(L, 3);
    AIDurHitCritCastEffect::checkExcute(*AIContext::getInstance(), source, target, hurttype);
    return 0;
}

static int luabind_check_do_259(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int markid = lua_tointeger(L, 2);
    int hpdelta = lua_tointeger(L, 3);
    int luaret = AIEffect259::checkExecute(*AIContext::getInstance(), target, markid, hpdelta);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_262(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int skillid = lua_tointeger(L, 2);
    int luaret = AICritIfHitEffect::checkExecute(target, skillid);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_eff264_check_do(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    auto casuermyid = lua_tointeger(L, 2);
    int act = lua_tointeger(L, 3);
    AIEffect264::checkExecute(*AIContext::getInstance(), c, casuermyid, act);
    return 0;
}

static int luabind_check_do_265(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int hpdelta = lua_tointeger(L, 2);
    bool is_simu = lua_toboolean(L, 3);
    int luaret = AIEffect265::checkExecute(*AIContext::getInstance(), target, hpdelta, is_simu);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_check_do_266(lua_State *L)
{
    auto character = luastack_getcharacterdata(L, 1);
    bool luaret = AIEffect266::checkExecute(*AIContext::getInstance(), character);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_eff266_save_remain_tick(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    AIEffect266::checkSaveRemainTick(*AIContext::getInstance(), c);
    return 0;
}

static int luabind_check_do_267(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    auto attacker = luastack_getcharacterdata(L, 2);
    int luaret = AIEffect267::checkExecute(*AIContext::getInstance(), target, attacker);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_get_eff_269_extrahurt(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int hpdelta = lua_tointeger(L, 2);
    int luaret = AIEffect269::getExtraHurt(*AIContext::getInstance(), target, hpdelta);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_get_eff_269_hitlabel(lua_State *L)
{
    auto target = luastack_getcharacterdata(L, 1);
    int luaret = AIEffect269::getHitLabel(*AIContext::getInstance(), target);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_eff279_check_do_enter_action(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int act = lua_tointeger(L, 2);
    AIEffect279::checkExecuteEnterAction(*AIContext::getInstance(), c, act);
    return 0;
}

static int luabind_eff279_check_do_exit_action(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    int act = lua_tointeger(L, 2);
    AIEffect279::checkExecuteExitAction(*AIContext::getInstance(), c, act);
    return 0;
}

static int luabind_get_extra_hpsuck_278(lua_State *L)
{
    auto source = luastack_getcharacterdata(L, 1);
    auto target = luastack_getcharacterdata(L, 2);
    int luaret = AIEffect278::getExtraHpSuck(*AIContext::getInstance(), source, target);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_eff281_check_do(lua_State *L)
{
    auto c = luastack_getcharacterdata(L, 1);
    AIEffect281::checkExecuteEnterAction(*AIContext::getInstance(), c);
    return 0;
}

static int luabind_effectcaster_get_myid(lua_State *L)
{
    auto caster = luastack_geteffectcaster(L, 1);
    lua_pushinteger(L, caster->myID);
    return 1;
}

static int luabind_effectcaster_get_effectid(lua_State *L)
{
    auto caster = luastack_geteffectcaster(L, 1);
    lua_pushinteger(L, caster->effectID);
    return 1;
}

static int luabind_effectcaster_get_localtick(lua_State *L)
{
    auto caster = luastack_geteffectcaster(L, 1);
    lua_pushinteger(L, caster->localTick);
    return 1;
}

static int luabind_effectcaster_schedule_innereff(lua_State *L)
{
    auto ec = luastack_geteffectcaster(L, 1);
    int tick = lua_tointeger(L, 2);
    auto e = luastack_getinnereffect(L, 3);
    ec->scheduleInnerEff(tick, e);
    return 0;
}

static int luabind_effectcaster_unschedule_innereff(lua_State *L)
{
    auto ec = luastack_geteffectcaster(L, 1);
    int uid = lua_tointeger(L, 2);
    ec->unscheduleInnerEff(uid);
    return 0;
}

static int luabind_effectcaster_append_schedule_innereff(lua_State *L)
{
    auto ec = luastack_geteffectcaster(L, 1);
    int tick = lua_tointeger(L, 2);
    auto e = luastack_getinnereffect(L, 3);
    ec->appendScheduleInnerEff(tick, e);
    return 0;
}

int luaopen_aieffectcaster(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "check_exert_special_effect", luabind_check_exert_special_effect },
        { "check_do_float", luabind_check_do_float },
        { "check_exert_mixchangetype_effect", luabind_check_exert_mixchangetype_effect },
        { "zero_special_effect", luabind_zero_special_effect },
        { "change_special_effect", luabind_change_special_effect },
        { "is_anyinnerexec_incd", luabind_is_anyinnerexec_incd },
        { "start_innereffect_cd", luabind_start_innereffect_cd },
        { "apply_attr_swap_effect", luabind_apply_attr_swap_effect },
        { "deapply_attr_effect", luabind_deapply_attr_effect },
        { "innereffect_apply_hpchange", luabind_innereffect_apply_hpchange },
        { "kill_follow_skill_tick_effect", luabind_kill_follow_skill_tick_effect }, // 12.0
        { "dispel_innereffect", luabind_dispel_innereffect }, // 13.0
        { "innereffect_apply_nonehpchange", luabind_innereffect_apply_nonehpchange }, // 13.0
        { "is_caster_group_covered", luabind_is_caster_group_covered },
        { "check_cover_group_casters", luabind_check_cover_group_casters },
        { "get_effect_value", luabind_get_effect_value }, // OP2

        { "trigger_161", luabind_trigger_161},
        { "eff172_has_activated_eff", luabind_eff172_has_activated_eff },
        { "eff172_get_valid_eff", luabind_eff172_get_valid_eff },
        { "eff172_get_activated_eff", luabind_eff172_get_activated_eff },
        { "eff172_trigger", luabind_eff172_trigger },
        { "eff172_get_unactivated_eff", luabind_eff172_get_unactivated_eff },
        { "trigger_178", luabind_trigger_178},
        { "trigger_179", luabind_trigger_179},
        { "eff180_get_firstvalid_eff", luabind_eff180_get_firstvalid_eff },
        { "eff180_trigger", luabind_eff180_trigger },
        { "try_trigger_181", luabind_try_trigger_181},
        { "trigger_181", luabind_trigger_181},
        { "get_eff_183_rangeskillindex", luabind_get_eff_183_rangeskillindex },
        { "trigger_202", luabind_trigger_202},
        { "trigger_203", luabind_trigger_203},
        { "trigger_204", luabind_trigger_204},
        { "trigger_211", luabind_trigger_211 },
        { "get_eff_214_altattackseq", luabind_get_eff_214_altattackseq },
        { "can_trigger_247", luabind_can_trigger_247},
        { "trigger_247", luabind_trigger_247},
        { "trigger_271", luabind_trigger_271},
        { "can_trigger_272", luabind_can_trigger_272},
        { "trigger_272", luabind_trigger_272},
        { "can_clean_272", luabind_can_clean_272},
        { "trigger_273", luabind_trigger_273},
        { "check_do_repeleff", luabind_check_do_repeleff},
        { "check_do_174", luabind_check_do_174},
        { "check_do_207", luabind_check_do_207},
        { "check_do_213", luabind_check_do_213},
        { "check_do_217", luabind_check_do_217},
        { "check_do_218", luabind_check_do_218},
        { "check_do_221", luabind_check_do_221},
        { "check_do_230", luabind_check_do_230},
        { "check_do_231", luabind_check_do_231},
        { "check_do_239", luabind_check_do_239},
        { "check_do_240", luabind_check_do_240},
        { "check_do_242", luabind_check_do_242},
        { "check_do_254", luabind_check_do_254},
        { "check_do_259", luabind_check_do_259},
        { "check_do_262", luabind_check_do_262},
        { "eff264_check_do", luabind_eff264_check_do },
        { "check_do_265", luabind_check_do_265},
        { "check_do_266", luabind_check_do_266},
        { "eff266_save_remain_tick", luabind_eff266_save_remain_tick },
        { "check_do_267", luabind_check_do_267},
        { "get_eff_269_extrahurt", luabind_get_eff_269_extrahurt},
        { "get_eff_269_hitlabel", luabind_get_eff_269_hitlabel},
        { "eff279_check_do_enter_action", luabind_eff279_check_do_enter_action },
        { "eff279_check_do_exit_action", luabind_eff279_check_do_exit_action },
        { "get_extra_hpsuck_278", luabind_get_extra_hpsuck_278},
        { "eff281_check_do", luabind_eff281_check_do },
        { NULL, NULL }
    };

    static const luaL_Reg reg_m[] = {
        { "get_myid", luabind_effectcaster_get_myid },
        { "get_effectid", luabind_effectcaster_get_effectid },
        { "get_localtick", luabind_effectcaster_get_localtick },
        { "schedule_innereff", luabind_effectcaster_schedule_innereff },
        { "unschedule_innereff", luabind_effectcaster_unschedule_innereff },
        { "append_schedule_innereff", luabind_effectcaster_append_schedule_innereff },
        { "cast_innereffect_by_idx", luabind_cast_innereffect_by_idx },
        { "cleanup_innereffect", luabind_effectcaster_cleanup_innereffect },
        { "get_stateanimation_result", luabind_get_stateanimation_result },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, luatypename_effectcaster());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    
    return 1;
}
    
// MARK: Bind EffectCaster --

// MARK: -- Bind InnerEffect

static inline const char *luatypename_innereffect() { return "aiinnereffect"; }

void luastack_pushinnereffect(lua_State *L, AIInnerEffect *aneff)
{
    size_t sz = sizeof(AIInnerEffect *);
    AIInnerEffect **the_fulluserdata = static_cast<AIInnerEffect **>(lua_newuserdata(L, sz));
    *the_fulluserdata = aneff;

    luaL_getmetatable(L, luatypename_innereffect());
    lua_setmetatable(L, -2);
}

AIInnerEffect *luastack_getinnereffect(lua_State *L, int idx)
{
    if (lua_isnil(L, idx))
        return nullptr;
    AIInnerEffect **lua_userdata = static_cast<AIInnerEffect **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luabind_innereffect_get_uid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->getUID());
    return 1;
}

static int luabind_innereffect_get_changevalue(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->getChangeValue());
    return 1;
}

static int luabind_innereffect_set_changevalue(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    int luaval = lua_tointeger(L, 2);
    aneff->setChangeValue(luaval);
    return 0;
}

static int luabind_innereffect_get_originmyid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->getOriginMyId());
    return 1;
}

static int luabind_innereffect_get_targetmyid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->getTargetMyId());
    return 1;
}

static int luabind_innereffect_get_parentcaster(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    luastack_pusheffectcaster(L, aneff->parentCaster);
    return 1;
}

static int luabind_innereffect_get_origin(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    luastack_pushcharacterdata(L, aneff->origin);
    return 1;
}

static int luabind_innereffect_get_target(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    luastack_pushcharacterdata(L, aneff->target);
    return 1;
}

static int luabind_innereffect_get_originsafe(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    auto origin = aneff->aiContext.getCharacterByID(aneff->getOriginMyId());
    if (origin)
        luastack_pushcharacterdata(L, origin);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_innereffect_get_targetsafe(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    auto target = aneff->getTargetSafe();
    if (target)
        luastack_pushcharacterdata(L, target);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_innereffect_get_projectile(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    auto projectile = aneff->projectile;
    if (projectile)
        luastack_pushprojectile(L, projectile);
    else
        lua_pushnil(L);
    return 1;
}

static int luabind_innereffect_get_innerexec(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->parentCaster->effectID);
    lua_pushinteger(L, aneff->innerExec->ID);
    lua_pushinteger(L, aneff->innerExec->idx);
    return 3;
}

static int luabind_innereffect_get_is_ignorehitlabel(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushboolean(L, aneff->isIgnoreHitLabel());
    return 1;
}

static int luabind_innereffect_set_is_ignorehitlabel(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    bool luaval = lua_toboolean(L, 2);
    aneff->setIsIgnoreHitLabel(luaval);
    return 0;
}

static int luabind_innereffect_get_is_debuffed(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushboolean(L, aneff->isDebuffed());
    return 1;
}
    
static int luabind_innereffect_get_skill_id(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->skillID);
    return 1;
}
    
static int luabind_innereffect_get_skill_level(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushinteger(L, aneff->skillLevel);
    return 1;
}

static int luabind_innereffect_get_innerdata(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    std::string datakey = lua_tostring(L, 2);
    int luaret = aneff->getInnerData(datakey);
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_innereffect_set_innerdata(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    std::string datakey = lua_tostring(L, 2);
    int dataval = lua_tointeger(L, 3);
    aneff->setInnerData(datakey, dataval);
    return 0;
}

static int luabind_innereffect_has_innerdata(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    std::string datakey = lua_tostring(L, 2);
    bool luaret = aneff->hasInnerData(datakey);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_innereffect_check_clean_same_type_exec(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    aneff->checkCleanSameTypeExec();
    return 0;
}

bool luahelper_innereffect_onenter(lua_State *L, AIInnerEffect *aneff)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, aneff->innerExec->ID);
    if (lua_pcall(L, 1, 1, 0))
    {
        luahelper_errorlog(L);
        return false;
    }

    if (!lua_isnil(L, -1))
    {
        lua_getfield(L, -1, "on_enter");
        luastack_pushinnereffect(L, aneff);
        if (lua_pcall(L, 1, 1, 0))
        {
            luahelper_errorlog(L);
            return false;
        }
        
        bool luaval = lua_toboolean(L, -1);
        lua_settop(L, 0);
        return luaval;
    }

    lua_pop(L, 1);
    return false;
}

void luahelper_innereffect_onexit(lua_State *L, AIInnerEffect *aneff)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, aneff->innerExec->ID);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return;
    }

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }
    
    lua_getfield(L, -1, "on_exit");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    luastack_pushinnereffect(L, aneff);
    if (lua_pcall(L, 1, 0, 0)) {
        luahelper_errorlog(L);
    }
    
    lua_settop(L, 0);
}

void luahelper_innereffect_onscheduletick(lua_State *L, AIInnerEffect *aneff)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, aneff->innerExec->ID);
    if (lua_pcall(L, 1, 1, 0))
    {
        luahelper_errorlog(L);
        return;
    }

    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_getfield(L, -1, "on_scheduletick");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    luastack_pushinnereffect(L, aneff);
    if (lua_pcall(L, 1, 0, 0)) {
        luahelper_errorlog(L);
    }
    
    lua_settop(L, 0);
}

bool luahelper_innereffect_check_delayed(lua_State *L, AIContinuousEffectCaster *e, CharacterData *c, int i)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "innereffect_check_delayed");

    luastack_pusheffectcaster(L, e);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, i);

    if (lua_pcall(L, 3, 1, 0))
    {
        luahelper_errorlog(L);
        return false;
    }

    bool ret = lua_toboolean(L, -1);
    lua_settop(L, 0);
    return ret;
}

bool luahelper_innereffect_is_delayed(lua_State *L, AIContinuousEffectCaster *e, CharacterData *c, int i)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "innereffect_is_delayed");

    luastack_pusheffectcaster(L, e);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, i);

    if (lua_pcall(L, 3, 1, 0))
    {
        luahelper_errorlog(L);
        return false;
    }

    bool ret = lua_toboolean(L, -1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_apply_hpchange(lua_State *L, CharacterData *source, AIProjectile *proj, CharacterData *target, int skillid, int skilllevel, int attridx, int changeval, int markid, int casterid, AIInnerEffect *eff)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "innereffect_apply_hpchange");

    luastack_pushcharacterdata(L, source);

    if (proj)
        luastack_pushprojectile(L, proj);
    else
        lua_pushnil(L);

    if (target)
        luastack_pushcharacterdata(L, target);
    else
        lua_pushnil(L);

    lua_pushinteger(L, skillid);
    lua_pushinteger(L, skilllevel);
    lua_pushinteger(L, attridx);
    lua_pushinteger(L, changeval);
    lua_pushinteger(L, markid);
    lua_pushinteger(L, casterid);

    if (eff)
        luastack_pushinnereffect(L, eff);
    else
        lua_pushnil(L);

    if (lua_pcall(L, 10, 1, 0))
    {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = lua_tointeger(L, -1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_273_trigger(lua_State *L, CharacterData *c, int param, bool is_simu)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    
    lua_pushinteger(L, 273);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "trigger_withparam");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, param);
    lua_pushboolean(L, is_simu);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);

    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_282_check_do(lua_State *L, CharacterData *s, CharacterData *t)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 282);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, s);
    luastack_pushcharacterdata(L, t);
    if (lua_pcall(L, 2, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);
    
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_283_check_do(lua_State *L, CharacterData *c, int hurttype, int hpdelta)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 283);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, hurttype);
    lua_pushinteger(L, hpdelta);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);
    
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

void luahelper_innereffect_287_on_onetimehurt(lua_State *L, CharacterData *c)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 287);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
    }

    lua_getfield(L, -1, "on_onetimehurt");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    if (lua_pcall(L, 1, 0, 0)) {
        luahelper_errorlog(L);
    }
    
    lua_settop(L, 0);
}

CharacterData *luahelper_innereffect_288_get_first_mosthpdelta_attacker(lua_State *L, CharacterData *c)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 288);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return nullptr;
    }

    lua_getfield(L, -1, "get_first_mosthpdelta_attacker");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return nullptr;
    }

    auto t = luastack_getcharacterdata(L, -1);
    lua_pop(L, 1);
    lua_settop(L, 0);
    return t;
}

int luahelper_innereffect_288_check_do_simu(lua_State *L, CharacterData *s, CharacterData *t, int param)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 288);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "check_do_simu");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, s);
    luastack_pushcharacterdata(L, t);
    lua_pushinteger(L, param);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_288_check_do(lua_State *L, CharacterData *s, CharacterData *t, int param)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 288);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, s);
    luastack_pushcharacterdata(L, t);
    lua_pushinteger(L, param);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

bool luahelper_innereffect_290_check_do(lua_State *L, CharacterData *s, CharacterData *t)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 290);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return false;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, s);
    luastack_pushcharacterdata(L, t);
    if (lua_pcall(L, 2, 1, 0)) {
        luahelper_errorlog(L);
        return false;
    }

    bool ret = false;
    if (lua_isboolean(L, -1)) {
        ret = lua_toboolean(L, -1);
    }
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_291_check_do(lua_State *L, CharacterData *s, CharacterData *t, int param)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 291);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, s);
    luastack_pushcharacterdata(L, t);
    lua_pushinteger(L, param);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);
    
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

bool luahelper_innereffect_294_is_skillidblocked(lua_State *L, CharacterData *c, int skill_id)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 294);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "is_skillidblocked");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, skill_id);
    if (lua_pcall(L, 2, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    bool ret = false;
    if (lua_isboolean(L, -1))
        ret = lua_toboolean(L, -1);
    
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

bool luahelper_innereffect_297_is_findable(lua_State *L, CharacterData *t, CharacterData *searcher)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 297);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "is_findable");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, t);
    luastack_pushcharacterdata(L, searcher);
    if (lua_pcall(L, 2, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    bool ret = false;
    if (lua_isboolean(L, -1))
        ret = lua_toboolean(L, -1);
    
    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

void luahelper_innereffect_302_check_do(lua_State *L, CharacterData *c)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);
    lua_pushinteger(L, 302);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    if (lua_pcall(L, 1, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

int luahelper_innereffect_295_checkdo(lua_State *L, CharacterData *c, int curaction)
{
	lua_getglobal(L, "g_effecthelper");
	lua_getfield(L, -1, "get_eff");
	lua_remove(L, -2);
	lua_pushinteger(L, 295);
	if (lua_pcall(L, 1, 1, 0)) {
		luahelper_errorlog(L);
		return 0;
	}

	lua_getfield(L, -1, "check_do");
	lua_remove(L, -2);
	luastack_pushcharacterdata(L, c);
	lua_pushinteger(L, curaction);

	if (lua_pcall(L, 2, 1, 0)) {
		luahelper_errorlog(L);
		return 0;
	}

	int ret = 0;
	if (lua_isinteger(L, -1))
		ret = lua_tointeger(L, -1);

	lua_pop(L, 1);
    lua_settop(L, 0);
	return ret;
}

int luahelper_innereffect_312_trigger(lua_State *L, CharacterData *c, int hurttype, int hpdelta, bool is_simu)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 312);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "trigger");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, hurttype);
    lua_pushinteger(L, hpdelta);
    lua_pushboolean(L, is_simu);
    if (lua_pcall(L, 4, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);

    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_315_trigger(lua_State *L, CharacterData *c, int endelta)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 315);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "trigger");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, endelta);
    if (lua_pcall(L, 2, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);

    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_316_trigger(lua_State *L, CharacterData *c, int healdelta)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 316);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "trigger");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, healdelta);
    if (lua_pcall(L, 2, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);

    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

void luahelper_innereffect_317_checkdo(lua_State *L, CharacterData *c, int dt)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 317);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, dt);
    if (lua_pcall(L, 2, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

void luahelper_innereffect_318_checkdo(lua_State *L, CharacterData *c, int dt)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 318);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_getfield(L, -1, "check_do");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, dt);
    if (lua_pcall(L, 2, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

int luahelper_innereffect_320_trigger(lua_State *L, CharacterData *c, int hurttype, int hurtprop, int hpdelta, bool is_simu)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 320);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "trigger");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, hurttype);
    lua_pushinteger(L, hurtprop);
    lua_pushinteger(L, hpdelta);
    lua_pushboolean(L, is_simu);
    if (lua_pcall(L, 5, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);

    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}

int luahelper_innereffect_325_checkdo(lua_State *L, CharacterData *c, int idx)
{
	lua_getglobal(L, "g_effecthelper");
	lua_getfield(L, -1, "get_eff");
	lua_remove(L, -2);

	lua_pushinteger(L, 325);
	if (lua_pcall(L, 1, 1, 0)) {
		luahelper_errorlog(L);
		return 0;
	}

	lua_getfield(L, -1, "check_do");
	lua_remove(L, -2);
	luastack_pushcharacterdata(L, c);
	lua_pushinteger(L, idx);
	lua_pushinteger(L, c->attributes[idx]);
	if (lua_pcall(L, 3, 1, 0)) {
		luahelper_errorlog(L);
		return 0;
	}

	int ret = 0;
	if (lua_isinteger(L, -1))
		ret = lua_tointeger(L, -1);

	lua_pop(L, 1);
	lua_settop(L, 0);
	return ret;
}

void luahelper_innereffect_326_trigger(lua_State *L, CharacterData *s, CharacterData *t, int skid, int sklv)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 326);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_getfield(L, -1, "trigger");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, s);
    luastack_pushcharacterdata(L, t);
    lua_pushinteger(L, skid);
    lua_pushinteger(L, sklv);
    if (lua_pcall(L, 4, 0, 0)) {
        luahelper_errorlog(L);
        return;
    }

    lua_settop(L, 0);
}

void luahelper_innereffect_327_trigger(lua_State *L, CharacterData *c, CharacterData *attacker, int hurttype, int hurtprop)
{
	int oldtop = lua_gettop(L);
	lua_getglobal(L, "g_effecthelper"); /// [helper]
	lua_getfield(L, -1, "get_eff"); /// [helper,func]
	lua_remove(L, -2); ///[func]

	lua_pushinteger(L, 327); ///[func,327]
	if (lua_pcall(L, 1, 1, 0)) { ///[eff327]
		luahelper_errorlog(L);
		return;
	}

	lua_getfield(L, -1, "trigger"); ///[eff327,trigger]
	lua_remove(L, -2); ///[trigger]
	luastack_pushcharacterdata(L, c); ///[trigger,c]
	luastack_pushcharacterdata(L, attacker); ///[trigger,c,attacker]
	lua_pushinteger(L, hurttype); ///[trigger,c,attacker,hurttype]
	lua_pushinteger(L, hurtprop); ///[trigger,c,attacker,hurttype,hurtprop]

	if (lua_pcall(L, 4, 0, 0)) { ///[]
		luahelper_errorlog(L);
		return;
	}

	lua_settop(L, oldtop);
}

void luahelper_innereffect_328_trigger(lua_State *L, CharacterData *c, int skillid, int skilllv, int sktype, bool isforce, int enterorexit)
{
	int oldtop = lua_gettop(L);
	lua_getglobal(L, "g_effecthelper"); /// [helper]
	lua_getfield(L, -1, "get_eff"); /// [helper,func]
	lua_remove(L, -2); ///[func]

	lua_pushinteger(L, 328); ///[func,328]
	if (lua_pcall(L, 1, 1, 0)) { ///[eff328]
		luahelper_errorlog(L);
		return;
	}

	lua_getfield(L, -1, "trigger"); ///[eff328,trigger]
	lua_remove(L, -2); ///[trigger]
	luastack_pushcharacterdata(L, c); ///[trigger,c]
	lua_pushinteger(L, skillid); ///[trigger,c,skillid]
	lua_pushinteger(L, skilllv); ///[trigger,c,skillid,skilllv]
	lua_pushinteger(L, sktype); ///[trigger,c,skillid,skilllv,sktype]
	lua_pushboolean(L, isforce ? 1 : 0); ///[trigger,c,skillid,skilllv,sktype,isforce]
	lua_pushinteger(L, enterorexit); ///[trigger,c,skillid,skilllv,sktype,isforce,enterorexit]
	if (lua_pcall(L, 6, 0, 0)) { ///[]
		luahelper_errorlog(L);
		return;
	}

	lua_settop(L, oldtop);
}

int luahelper_innereffect_10001_trigger(lua_State *L, CharacterData *c, int hurttype, int hpdelta, bool is_simu ,int hittype)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "get_eff");
    lua_remove(L, -2);

    lua_pushinteger(L, 10001);
    if (lua_pcall(L, 1, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    lua_getfield(L, -1, "trigger");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, c);
    lua_pushinteger(L, hurttype);
    lua_pushinteger(L, hpdelta);
    lua_pushboolean(L, is_simu);
    lua_pushinteger(L, hittype);
    if (lua_pcall(L, 5, 1, 0)) {
        luahelper_errorlog(L);
        return 0;
    }

    int ret = 0;
    if (lua_isinteger(L, -1))
        ret = lua_tointeger(L, -1);

    lua_pop(L, 1);
    lua_settop(L, 0);
    return ret;
}


static int luabind_innereffect_remove_ui_effect(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    if (aneff) aneff->removeUIEffect();
    return 0;
}

static int luabind_innereffect_remove_body_effect(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    if (aneff) aneff->removeBodyEffect();
    return 0;
}

static int luabind_innereffect_cast_ui_effect(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    int ticks = lua_tointeger(L, 2);
    if (aneff) aneff->castUIEffect(ticks);
    return 0;
}

static int luabind_innereffect_cast_body_effect(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    int ticks = lua_tointeger(L, 2);
    if (aneff) aneff->castBodyEffect(ticks);
    return 0;
}

static int luabind_innereffect_is_temp_invalid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushboolean(L, aneff->isTempInvalid());
    return 1;
}

static int luabind_innereffect_set_temp_invalid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    bool luaval = lua_toboolean(L, 2);
    if (aneff) aneff->setTempInvalid(luaval);
    return 0;
}

static int luabind_innereffect_apply_attr_on_target(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    int luaret = 0;
    if (attreff)
    {
        auto character = luastack_getcharacterdata(L, 2);
        int attridx = lua_tointeger(L, 3);
        int changeval = lua_tointeger(L, 4);
        luaret = attreff->applyOnTarget(character, attridx, changeval);
    }
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_innereffect_deapply_attr_on_target(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    if (attreff)
    {
        auto character = luastack_getcharacterdata(L, 2);
        int attridx = lua_tointeger(L, 3);
        int changeval = lua_tointeger(L, 4);
        int realchangeval = lua_tointeger(L, 5);
        attreff->deapplyOnTarget(character, attridx, changeval, realchangeval);
    }
    return 0;
}

static int luabind_innereffect_get_attr_realchangevalue(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    int luaret = 0;
    if (attreff)
    {
        luaret = attreff->getAttrRealChangeValue();
    }
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_innereffect_set_attr_realchangevalue(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    if (attreff)
    {
        int luaval = lua_tointeger(L, 2);
        attreff->setAttrRealChangeValue(luaval);
    }
    return 0;
}

static int luabind_innereffect_get_attr_realchangevalue_origin(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    int luaret = 0;
    if (attreff)
    {
        luaret = attreff->getAttrRealChangeValueForOrigin();
    }
    lua_pushinteger(L, luaret);
    return 1;
}

static int luabind_innereffect_set_attr_realchangevalue_origin(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    if (attreff)
    {
        int luaval = lua_tointeger(L, 2);
        attreff->setAttrRealChangeValueForOrigin(luaval);
    }
    return 0;
}

static int luabind_is_attr_effect(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    AIAttributeEffect *attreff = dynamic_cast<AIAttributeEffect *>(aneff);
    bool luaret = (attreff != nullptr);
    lua_pushboolean(L, luaret);
    return 1;
}

static int luabind_innereffect_is_valid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    lua_pushboolean(L, aneff->isValid);
    return 1;
}

static int luabind_innereffect_set_valid(lua_State *L)
{
    auto aneff = luastack_getinnereffect(L, 1);
    bool luaval = lua_toboolean(L, 2);
    aneff->isValid = luaval;
    return 0;
}

static int luabind_innereffect_is_erased(lua_State *L)
{
    auto e = luastack_getinnereffect(L, 1);
    lua_pushboolean(L, e->isErased());
    return 1;
}

int luaopen_aiinnereffect(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { NULL, NULL }
    };

    static const luaL_Reg reg_m[] = {
        { "get_uid", luabind_innereffect_get_uid },
        { "get_changevalue", luabind_innereffect_get_changevalue },
        { "set_changevalue", luabind_innereffect_set_changevalue },
        { "get_originmyid", luabind_innereffect_get_originmyid },
        { "get_targetmyid", luabind_innereffect_get_targetmyid },
        { "get_parentcaster", luabind_innereffect_get_parentcaster },
        { "get_origin", luabind_innereffect_get_origin },
        { "get_target", luabind_innereffect_get_target },
        { "get_originsafe", luabind_innereffect_get_originsafe },
        { "get_targetsafe", luabind_innereffect_get_targetsafe },
        { "get_projectile", luabind_innereffect_get_projectile },
        { "get_innerexec", luabind_innereffect_get_innerexec },
        { "get_is_ignorehitlabel", luabind_innereffect_get_is_ignorehitlabel },
        { "set_is_ignorehitlabel", luabind_innereffect_set_is_ignorehitlabel },
        { "get_is_debuffed", luabind_innereffect_get_is_debuffed },
        { "get_skill_id", luabind_innereffect_get_skill_id },
        { "get_skill_level", luabind_innereffect_get_skill_level },

        { "get_innerdata", luabind_innereffect_get_innerdata },
        { "set_innerdata", luabind_innereffect_set_innerdata },
        { "has_innerdata", luabind_innereffect_has_innerdata },

        { "check_clean_same_type_exec", luabind_innereffect_check_clean_same_type_exec },
        { "remove_ui_effect", luabind_innereffect_remove_ui_effect },
        { "remove_body_effect", luabind_innereffect_remove_body_effect },
        { "cast_ui_effect", luabind_innereffect_cast_ui_effect },
        { "cast_body_effect", luabind_innereffect_cast_body_effect },
        { "is_temp_invalid", luabind_innereffect_is_temp_invalid },
        { "set_temp_invalid", luabind_innereffect_set_temp_invalid },
        { "apply_attr_on_target", luabind_innereffect_apply_attr_on_target },
        { "deapply_attr_on_target", luabind_innereffect_deapply_attr_on_target },
        { "get_attr_realchangevalue", luabind_innereffect_get_attr_realchangevalue },
        { "set_attr_realchangevalue", luabind_innereffect_set_attr_realchangevalue },
        { "get_attr_realchangevalue_origin", luabind_innereffect_get_attr_realchangevalue_origin },
        { "set_attr_realchangevalue_origin", luabind_innereffect_set_attr_realchangevalue_origin },
        { "is_attr_effect", luabind_is_attr_effect },
        { "is_valid", luabind_innereffect_is_valid },
        { "set_valid", luabind_innereffect_set_valid },
        { "is_erased", luabind_innereffect_is_erased },

        { NULL, NULL }
    };

    luaL_newmetatable(L, luatypename_innereffect());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}

bool luahelper_effectcaster_is_group_covered(lua_State *L, CharacterData *source, CharacterData *target, int execid)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "is_group_covered");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, source);
    luastack_pushcharacterdata(L, target);
    lua_pushinteger(L, execid);
    if (lua_pcall(L, 3, 1, 0)) {
        luahelper_errorlog(L);
        return false;
    }

    bool ret = lua_toboolean(L, -1);
    lua_settop(L, 0);
    return ret;
}

void luahelper_effectcaster_check_cover_group_casters(lua_State *L, CharacterData *target, int execid)
{
    lua_getglobal(L, "g_effecthelper");
    lua_getfield(L, -1, "check_cover_group_casters");
    lua_remove(L, -2);
    luastack_pushcharacterdata(L, target);
    lua_pushinteger(L, execid);
    if (lua_pcall(L, 2, 0, 0)) {
        luahelper_errorlog(L);
    }
}

// MARK: Bind InnerEffect --

}
