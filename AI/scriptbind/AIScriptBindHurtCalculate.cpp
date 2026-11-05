//
//  AIScriptBindHurtCalculate.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindHurtCalculate.hpp"
#include "lua.hpp"
#include "AIScriptBindHelper.h"
#include "AIScriptBindProjectile.hpp"
#include "AIScriptBindCharacterData.hpp"
#include "AIScriptBindEffectCaster.hpp"
#include "../HeroHurtcalculate.h"
#include "../AIContext.h"
#include "../CommonType.h"

namespace OnePiece {

void luastack_pushhurtcalcparam(lua_State *L, const HurtCalcParam &param)
{
    lua_newtable(L);
    int paramidx = lua_gettop(L);
    
    lua_pushstring(L, "projectile");
    if (param.projectile)
        luastack_pushprojectile(L, param.projectile);
    else
        lua_pushnil(L);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "skillid");
    lua_pushinteger(L, param.skillID);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "skill_level");
    lua_pushinteger(L, param.skillLevel);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "hittype");
    lua_pushinteger(L, param.hitType);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "hpdelta");
    lua_pushinteger(L, param.HPDelta);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "hurtprop");
    lua_pushinteger(L, param.hurtProperty);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "hurttype");
    lua_pushinteger(L, param.hurtType);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "hurtid");
    if (param.hurtExec)
        lua_pushinteger(L, param.hurtExec->ID);
    else
        lua_pushnil(L);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "is_skiprebound");
    lua_pushboolean(L, param.shouldSkipRebound);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "is_ignorehitlabel");
    lua_pushboolean(L, param.ignoreHurtLabel);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "is_fromeffect");
    lua_pushboolean(L, param.isFromEffectCaster);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "is_rawhurt");
    lua_pushboolean(L, param.isRawHurt);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "is_simu");
    lua_pushboolean(L, param.isSimulateCalc);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "is_fromsacrificerecv");
    lua_pushboolean(L, param.isFromSacrificeRecv);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "stathurtsourcealt");
    if (param.statHurtSourceAlt)
        luastack_pushcharacterdata(L, param.statHurtSourceAlt);
    else
        lua_pushnil(L);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "markid");
    lua_pushinteger(L, param.markID);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "casterid");
    lua_pushinteger(L, param.casterID);
    lua_settable(L, paramidx);
    
    lua_pushstring(L, "alternatehitlabel");
    lua_pushinteger(L, param.alternateHitLabel);
    lua_settable(L, paramidx);

    if (param.innerEffect)
    {
        lua_pushstring(L, "innereffect");
        luastack_pushinnereffect(L, param.innerEffect);
        lua_settable(L, paramidx);
    }
}

int luastack_gethurtcalcparam(lua_State *L, int idx, HurtCalcParam &outparam)
{
    if (!lua_istable(L, idx)) { return 1; }
    
    //--begin
    auto luarettype = lua_getfield(L, idx, "projectile");
    if (luarettype == LUA_TUSERDATA)
        outparam.projectile = luastack_getprojectile(L, -1);
    else
        outparam.projectile = nullptr;
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "skillid");
    outparam.skillID = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "skill_level");
    outparam.skillLevel = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "hittype");
    outparam.hitType = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "hpdelta");
    outparam.HPDelta = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "hurtprop");
    outparam.hurtProperty = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "hurttype");
    outparam.hurtType = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "hurtid");
    int hurtid = lua_tointeger(L, -1);
    if (hurtid > 0)
        outparam.hurtExec = AIDataHelper::getHurtExec(hurtid);
    else
        outparam.hurtExec = nullptr;
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "is_skiprebound");
    outparam.shouldSkipRebound = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "is_ignorehitlabel");
    outparam.ignoreHurtLabel = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "is_fromeffect");
    outparam.isFromEffectCaster = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "is_rawhurt");
    outparam.isRawHurt = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "is_simu");
    outparam.isSimulateCalc = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "is_fromsacrificerecv");
    outparam.isFromSacrificeRecv = lua_toboolean(L, -1);
    lua_pop(L, 1);
    
    luarettype = lua_getfield(L, idx, "stathurtsourcealt");
    if (luarettype == LUA_TUSERDATA)
        outparam.statHurtSourceAlt = luastack_getcharacterdata(L, -1);
    else
        outparam.statHurtSourceAlt = nullptr;
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "markid");
    outparam.markID = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "casterid");
    outparam.casterID = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    lua_getfield(L, idx, "alternatehitlabel");
    outparam.alternateHitLabel = lua_tointeger(L, -1);
    lua_pop(L, 1);
    
    //end--
    
    return 0;
}
    
    int luahelper_exerthurt(lua_State *L, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target, int theMarkID)
    {
        lua_getglobal(L, "g_hurthelper");
        lua_getfield(L, -1, "executehurt");
        
        luastack_pushcharacterdata(L, source);
        luastack_pushprojectile(L, projectile);
        lua_pushinteger(L, hurtID);
        lua_pushinteger(L, skillID);
        lua_pushinteger(L, skillLevel);
        luastack_pushcharacterdata(L, target);
        lua_pushinteger(L, theMarkID);
        
        if (lua_pcall(L, 7, 1, 0)) {
            luahelper_errorlog(L);
            return 0;
        }
        
        int luaret = lua_tointeger(L, -1);
        lua_settop(L, 0);
        return luaret;
    }

    void luahelper_characterdeadbyhurt(lua_State *L, CharacterData *pTarget, CharacterData *pSource, int skillID, int theSkillLevel, HurtExec *pHurtExec, InnerEffectExec* pEffectExec)
    {
        lua_getglobal(L, "g_hurthelper");
        lua_getfield(L, -1, "characterdeadbyhurt");

        luastack_pushcharacterdata(L, pTarget);
        luastack_pushcharacterdata(L, pSource);
        lua_pushinteger(L, skillID);
        lua_pushinteger(L, theSkillLevel);

        if (pHurtExec)
            lua_pushinteger(L, pHurtExec->ID);
        else
            lua_pushinteger(L, 0);

        if (pEffectExec)
        {
            lua_pushinteger(L, pEffectExec->ID);

            if (pEffectExec->vExecDeadExecIDs.size() > 0)
            {
                lua_createtable(L, (int)pEffectExec->vExecDeadExecIDs.size(), 0);
                int idx = lua_gettop(L);
                for (int i = 0; i < pEffectExec->vExecDeadExecIDs.size(); i++)
                {
                    lua_pushinteger(L, pEffectExec->vExecDeadExecIDs[i]);
                    lua_rawseti(L, idx, i+1);
                }
            }
            else
            {
                lua_pushnil(L);
            }
        }
        else
        {
            lua_pushinteger(L, 0);
            lua_pushnil(L);
        }

        if (lua_pcall(L, 7, 0, 0)) {
            luahelper_errorlog(L);
        }
        
        lua_settop(L, 0);
    }

    int luahelper_hurttarget(lua_State *L, CharacterData *source, CharacterData *target, HurtCalcParam &param, HitResult &outresult)
    {
        lua_getglobal(L, "g_hurthelper");
        lua_getfield(L, -1, "hurttarget");
        
        luastack_pushcharacterdata(L, source);
        luastack_pushcharacterdata(L, target);
        luastack_pushhurtcalcparam(L, param);

        if (lua_pcall(L, 3, 3, 0))
        {
            luahelper_errorlog(L);
            return 0;
        }

        outresult.hitResult = HitResult::HitResultType(lua_tointeger(L, -3));
        outresult.blockResult = HitResult::BlockType(lua_tointeger(L, -2));
        int luaval = lua_tointeger(L, -1); // hpdelta
        lua_settop(L, 0);
        return luaval;
    }
}
