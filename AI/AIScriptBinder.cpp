//
//  AIScriptBinder.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBinder.hpp"
#include <string>
#include "lua.hpp"
#include "lua_cjson.hpp"
#include "luasocket.hpp"
#include "cocos2d.h"
#include "AIPlatformHelper.hpp"
#include "scriptbind/AIScriptBindHelper.h"
#include "scriptbind/AIScriptBindCharacterData.hpp"
#include "scriptbind/AIScriptBindProjectile.hpp"
#include "scriptbind/AIScriptBindContext.hpp"
#include "scriptbind/AIScriptBindHitLabel.hpp"
#include "scriptbind/AIScriptBindEffectCaster.hpp"
#include "scriptbind/AIScriptBindSummonCaster.hpp"
#include "scriptbind/AIScriptBindActionController.hpp"
#include "scriptbind/AIScriptBindBattleStat.hpp"
#include "scriptbind/AIScriptBindSkillController.hpp"
#include "scriptbind/AIScriptBindMarkCaster.hpp"
#include "scriptbind/AIScriptBindCallback.hpp"
#include "scriptbind/AIScriptBindExternalDelegate.hpp"

namespace OnePiece {
    
    // MARK: -- ScriptBinder
    
    AIScriptBinder *AIScriptBinder::getInstance()
    {
        static AIScriptBinder g_binder;
        return &g_binder;
    }
    
    void AIScriptBinder::initConfig(lua_State *L)
    {
        if (!L)
        {
            resetDefaultConfig();
            return;
        }
        
        lua_getglobal(L, "g_hostconfig");
        int idx = lua_gettop(L);
        
        lua_getfield(L, idx, "is_hurtcalc_exerthurtbylua");
        if (lua_isboolean(L, -1))
            _isHurtCalcExertHurtByLua = lua_toboolean(L, -1);
        lua_pop(L, 1);
        
        lua_getfield(L, idx, "is_ignore_positive_hurtvalue_hitlabel_and_stat");
        if (lua_isboolean(L, -1))
            _isIgnorePositiveHurtValueHitLabelAndStat = lua_toboolean(L, -1);
        lua_pop(L, 1);
        
        lua_settop(L, 0);
    }
    
    void AIScriptBinder::resetDefaultConfig()
    {
//        _isSearchTargetByLua = false;
        _searchTargetByLua.clear();
        _isHurtCalcExertHurtByLua = false;
        _innerEffectByLua.clear();
        _luaconfig.clear();
        _luaconfigCommonBoolean.clear();
        _luaconfigCommonInteger.clear();
        _casterSyncAttackSpeed.clear();
		_hitlabelDisplayType.clear();
        _isIgnorePositiveHurtValueHitLabelAndStat = false;
    }
    
    bool AIScriptBinder::isSearchTargetByLua(lua_State *L, int searchType)
    {
        auto iter = _searchTargetByLua.find(searchType);
        if (iter != _searchTargetByLua.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_searchtargetbylua");
        lua_remove(L, -2);
        lua_pushinteger(L, searchType);
        
        if (lua_pcall(L, 1, 1, 0)) {
            luahelper_errorlog(L);
            return false;
        }
        
        bool luaval = lua_toboolean(L, -1);
        lua_pop(L, 1);
        
        _searchTargetByLua[searchType] = luaval;
        
        return luaval;
    }

    bool AIScriptBinder::isHurtCalcHurtTargetByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(HURTCALC_HURTTARGET_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;

        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_hurtcalc_hurttarget_bylua");

        bool luaval = false;
        if (lua_isboolean(L, -1))
            luaval = lua_toboolean(L, -1);
        _luaconfig[HURTCALC_HURTTARGET_BYLUA] = luaval;

        lua_pop(L, 2);

        return luaval;
    }
    
    bool AIScriptBinder::isInnerEffectByLua(lua_State *L, int effid)
    {
        auto iter = _innerEffectByLua.find(effid);
        if (iter != _innerEffectByLua.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_innereffectbylua");
        lua_remove(L, -2);
        lua_pushinteger(L, effid);
        
        if (lua_pcall(L, 1, 1, 0))
        {
            luahelper_errorlog(L);
            return false;
        }
        
        bool luaval = lua_toboolean(L, -1);
        lua_pop(L, 1);
        
        _innerEffectByLua[effid] = luaval;
        
        return luaval;
    }

    bool AIScriptBinder::isInnerEffectApplyHpchangeByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(INNEREFFECT_APPLYHPCHANGE_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;

        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_inneffapplyhpchangebylua");

        bool luaval = false;
        if (!lua_isnil(L, -1))
            luaval = lua_toboolean(L, -1);
        _luaconfig[INNEREFFECT_APPLYHPCHANGE_BYLUA] = luaval;

        lua_pop(L, 2);

        return luaval;
    }

    bool AIScriptBinder::isProjectileCasterThrowProjectileByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(PROJECTILECASTER_THROWPROJECTILE_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_projectilecaster_throwprojectile_bylua");

        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);

        _luaconfig[PROJECTILECASTER_THROWPROJECTILE_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isAttackActionUpdateByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(ATTACKACTION_UPDATE_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_attackaction_update_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[ATTACKACTION_UPDATE_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isAttackActionEnterByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(ATTACKACTION_ENTER_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_attackaction_enter_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[ATTACKACTION_ENTER_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isActionSelectorByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(ACTIONSELECTOR_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_actionselector_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[ACTIONSELECTOR_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isLockTargetByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(LOCKTARGET_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_locktarget_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[LOCKTARGET_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isCanCastUltraSkillByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(CANCASTULTRA_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_cancastultra_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[CANCASTULTRA_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isCheckTargetByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(CHECKTARGET_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_checktarget_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[CHECKTARGET_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isPickSkillByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(PICKSKILL_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_pickskill_bylua");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[PICKSKILL_BYLUA] = ret;
        return ret;
    }

    bool AIScriptBinder::isUseNewTargetHelper(lua_State *L)
    {
        auto iter = _luaconfig.find(USE_NEW_TARGETHELPER);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_use_new_targethelper");
        
        bool ret = false;
        if (!lua_isnil(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[USE_NEW_TARGETHELPER] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isIgnorePositiveHurtValueHitLabelAndStat()
    {
        return _isIgnorePositiveHurtValueHitLabelAndStat;
    }
    
    bool AIScriptBinder::isCaptainSelectorByLua(lua_State *L)
    {
        auto iter = _luaconfig.find(CAPTAIN_SELECTOR_BYLUA);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_captain_selector_bylua");
        
        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[CAPTAIN_SELECTOR_BYLUA] = ret;
        return ret;
    }
    
    bool AIScriptBinder::isReshowUIEffectsWhenRevived(lua_State *L)
    {
        auto iter = _luaconfig.find(RESHOW_UIEFFECTS_WHEN_REVIVED);
        if (iter != _luaconfig.end())
            return iter->second;
        
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_reshow_uieffects_when_revived");
        
        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);
        
        lua_pop(L, 2);
        
        _luaconfig[RESHOW_UIEFFECTS_WHEN_REVIVED] = ret;
        return ret;
    }

    bool AIScriptBinder::isReshowMarkIconsWhenRevived(lua_State *L)
    {
        auto iter = _luaconfig.find(RESHOW_MARKICONS_WHEN_REVIVED);
        if (iter != _luaconfig.end())
            return iter->second;

        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_reshow_markicons_when_revived");

        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);

        lua_pop(L, 2);

        _luaconfig[RESHOW_MARKICONS_WHEN_REVIVED] = ret;
        return ret;
    }

    bool AIScriptBinder::isCasterSyncAttackSpeed(lua_State *L, int caster_id)
    {
        auto iter = _casterSyncAttackSpeed.find(caster_id);
        if (iter != _casterSyncAttackSpeed.end())
            return iter->second;

        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_castersyncattackspeed");
        lua_remove(L, -2);
        lua_pushinteger(L, caster_id);

        if (lua_pcall(L, 1, 1, 0)) {
            luahelper_errorlog(L);
            return false;
        }

        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);
        lua_pop(L, 1);

        _casterSyncAttackSpeed[caster_id] = ret;

        return ret;
    }

    bool AIScriptBinder::isUseCannotBeAttackedBuffCnt(lua_State *L)
    {
        auto iter = _luaconfig.find(USE_CANNOT_BEATTACK_BUFFCNT);
        if (iter != _luaconfig.end())
            return iter->second;

        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_use_cannotbeattackbuffcnt");

        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);

        lua_pop(L, 2);

        _luaconfig[USE_CANNOT_BEATTACK_BUFFCNT] = ret;
        return ret;
    }

    bool AIScriptBinder::isEffect175DisableLimit(lua_State *L)
    {
        auto iter = _luaconfig.find(EFFECT_175_DISABLE_LIMIT);
        if (iter != _luaconfig.end())
            return iter->second;

        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, "is_effect_175_disable_limit");

        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);

        lua_pop(L, 2);

        _luaconfig[EFFECT_175_DISABLE_LIMIT] = ret;
        return ret;
    }

	int AIScriptBinder::getHitlabelDisplayType(lua_State *L, int labelType)
	{
		auto iter = _hitlabelDisplayType.find(labelType);
		if (iter != _hitlabelDisplayType.end())
			return iter->second;

		lua_getglobal(L, "g_hostconfig");
		lua_getfield(L, -1, "get_hitlabeldisplaytype");
		lua_remove(L, -2);
		lua_pushinteger(L, labelType);

		if (lua_pcall(L, 1, 1, 0)) {
			luahelper_errorlog(L);
			return 0;
		}

		int luaval = lua_tointeger(L, -1);
		lua_pop(L, 1);

		_hitlabelDisplayType[labelType] = luaval;

		return luaval;
	}
    
    bool AIScriptBinder::getCommonConfigBoolean(lua_State *L, const char *config_name)
    {
        auto iter = _luaconfigCommonBoolean.find(config_name);
        if (iter != _luaconfigCommonBoolean.end())
            return iter->second;

        bool ret = getCommonConfigBooleanNoCache(L, config_name);
        _luaconfigCommonBoolean[config_name] = ret;
        return ret;
    }

    bool AIScriptBinder::getCommonConfigBooleanNoCache(lua_State *L, const char *config_name)
    {
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, config_name);

        bool ret = false;
        if (lua_isboolean(L, -1))
            ret = lua_toboolean(L, -1);
        lua_pop(L, 2);
        return ret;
    }

    int AIScriptBinder::getCommonConfigInteger(lua_State *L, const char *config_name)
    {
        auto iter = _luaconfigCommonInteger.find(config_name);
        if (iter != _luaconfigCommonInteger.end())
            return iter->second;

        int ret = getCommonConfigIntegerNoCache(L, config_name);
        _luaconfigCommonInteger[config_name] = ret;
        return ret;
    }

    int AIScriptBinder::getCommonConfigIntegerNoCache(lua_State *L, const char *config_name)
    {
        lua_getglobal(L, "g_hostconfig");
        lua_getfield(L, -1, config_name);

        int ret = 0;
        if (lua_isinteger(L, -1))
            ret = lua_tointeger(L, -1);
        lua_pop(L, 2);
        return ret;
    }
    
    // MARK: ScriptBinder --
    
    void op_registertolua(lua_State *L)
    {
        luaL_requiref(L, "aicontext", luaopen_aicontext, 1);
        luaL_requiref(L, "aicharacterdata", luaopen_characterdata, 1);
        luaL_requiref(L, "aiprojectile", luaopen_projectile, 1);
        luaL_requiref(L, "aiprojectilecaster", luaopen_projectilecaster, 1);
        luaL_requiref(L, "aieffectcaster", luaopen_aieffectcaster, 1);
        luaL_requiref(L, "aisummoncaster", luaopen_summoncaster, 1);
        luaL_requiref(L, "aihitlabel", luaopen_hitlabel, 1);
        luaL_requiref(L, "aiactioncontroller", luaopen_aiactioncontroller, 1);
        luaL_requiref(L, "aiinnereffect", luaopen_aiinnereffect, 1);
        luaL_requiref(L, "aibattlestat", luaopen_battlestat, 1);
        luaL_requiref(L, "aispecialskillinfo", luaopen_specialskillinfo, 1);
        luaL_requiref(L, "aiskillcontroller", luaopen_aiskillcontroller, 1);
        luaL_requiref(L, "aimarkcaster", luaopen_markcaster, 1);
        luaL_requiref(L, "aicallback", luaopen_aicallback, 1);
        luaL_requiref(L, "aiexternaldelegate", luaopen_externaldelegate, 1);
    }
    
//    static int searcher_Lua (lua_State *L) {
//        const char *filename;
//        const char *name = luaL_checkstring(L, 1);
//        filename = findfile(L, name, "path", LUA_LSUBSEP);
//        if (filename == NULL) return 1;  /* module not found in this path */
//        return checkload(L, (luaL_loadfile(L, filename) == LUA_OK), filename);
//    }
    
    static int op_searcher_lua(lua_State *L)
    {
#ifndef SERVER_VERIFYING
        std::string name = luaL_checkstring(L, 1);
        std::string filename = "src/lua/ai/" + name + ".luac";
        if (!cocos2d::FileUtils::getInstance()->isFileExist(filename))
        {
            filename = "src/lua/ai/" + name + ".lua";
            if (!cocos2d::FileUtils::getInstance()->isFileExist(filename))
                return 1;
        }
        //std::string filestr = cocos2d::FileUtils::getInstance()->getStringFromFile(filename);
        //luaL_loadstring(L, filestr.c_str());
        auto filedata = cocos2d::FileUtils::getInstance()->getDataFromFile(filename);
        luaL_loadbuffer(L, (const char *)filedata.getBytes(), filedata.getSize(), filename.c_str());
        lua_pushstring(L, filename.c_str());  /* will be 2nd argument to module */
        return 2;  /* return open function and file name */
#else
        return 1;
#endif
    }
    
    lua_State *op_createluastate()
    {
        lua_State *L = luaL_newstate();
        luaL_openlibs(L);
        luaL_requiref(L, "cjson", luaopen_cjson, 1);
        
        
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        luaL_requiref(L, "socket", luaopen_socket_core, 1); // MARK: for debug purpose
#endif
        
        op_registertolua(L);
        
        bool isIOSorAndroid = false;

        // MARK: -- set custom lua searcher
#ifdef CC_TARGET_PLATFORM
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        lua_getglobal(L, "package");
        lua_getfield(L, -1, "searchers");
        
        lua_pushvalue(L, -2);  /* set 'package' as upvalue for all searchers */
//        auto lenofsearchers = lua_rawlen(L, -1);
        lua_pushcclosure(L, op_searcher_lua, 1);
        lua_rawseti(L, -2, 3);

        isIOSorAndroid = true;
#endif
#endif
        // MARK: set custom lua searcher --
        
        
        std::string platformname = AIPlatformHelper::getPlatformName();
        lua_pushstring(L, platformname.c_str());
        lua_setglobal(L, "g_platform");
        
#ifdef SERVER_VERIFYING
        //const std::string mainLuac = "./lua/ai/main.luac";
        //std::string mainLua = "./lua/ai/main.lua";
        std::string mainLua = AIPlatformHelper::getExecutablePath() + "/lua/ai/main.lua";
#ifdef __APPLE__
        mainLua = "../fight_server/lua/ai/main.lua";
#endif

#ifdef WIN32
        mainLua = "../fight_server/lua/ai/main.lua";
#endif
        std::string mainFile = mainLua;
        CCLOG("ai:try open main lua file:%s", mainFile.c_str());
        lua_pushstring(L, mainFile.c_str());
        lua_setglobal(L, "g_mainfilepath");
        luaL_loadfile(L, mainFile.c_str());
#else
        const std::string mainLuac = "src/lua/ai/main.luac";
        const std::string mainLua = "src/lua/ai/main.lua";
        std::string mainFile = "";
        if (cocos2d::FileUtils::getInstance()->isFileExist(mainLuac))
        {
            mainFile = mainLuac;
        }
        else if (cocos2d::FileUtils::getInstance()->isFileExist(mainLua))
        {
            mainFile = mainLua;
        }

        CCLOG("ai:try open main lua file:%s", mainFile.c_str());
        
        if (isIOSorAndroid)
        {
            auto mainData = cocos2d::FileUtils::getInstance()->getDataFromFile(mainFile);
            luaL_loadbuffer(L, (const char *)mainData.getBytes(), mainData.getSize(), "main");
        }
        else
        {
            mainFile = cocos2d::FileUtils::getInstance()->fullPathForFilename(mainFile);
            lua_pushstring(L, mainFile.c_str());
            lua_setglobal(L, "g_mainfilepath");
            luaL_loadfile(L, mainFile.c_str());
        }
#endif
        
        if (lua_pcall(L, 0, 0, 0))
        {
            //luahelper_errorlog(L);
            CCLOG("ai:lua load main fail:%s", lua_tostring(L, -1));
            luaL_error(L, lua_tostring(L, -1));
            lua_close(L);
            return nullptr;
        }
        
        lua_settop(L, 0);
        return L;
    }
    
    void op_closeluastate(lua_State *L)
    {
        lua_close(L);
    }
}
