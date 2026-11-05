//
//  AIScriptBinder.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBinder_hpp
#define AIScriptBinder_hpp

//#include "AIScriptBindCharacterData.hpp"
#include <map>
#include <string>

struct lua_State;

namespace OnePiece {

    enum AIScriptBindConfigType {
        INNEREFFECT_APPLYHPCHANGE_BYLUA = 0,
        PROJECTILECASTER_THROWPROJECTILE_BYLUA,
        ATTACKACTION_UPDATE_BYLUA,
        ATTACKACTION_ENTER_BYLUA,
        HURTCALC_HURTTARGET_BYLUA,
        ACTIONSELECTOR_BYLUA,
        LOCKTARGET_BYLUA,
        CANCASTULTRA_BYLUA,
        CHECKTARGET_BYLUA,
        PICKSKILL_BYLUA,
        USE_NEW_TARGETHELPER,
        CAPTAIN_SELECTOR_BYLUA,
        RESHOW_UIEFFECTS_WHEN_REVIVED,
        RESHOW_MARKICONS_WHEN_REVIVED,
        USE_CANNOT_BEATTACK_BUFFCNT,
        EFFECT_175_DISABLE_LIMIT
    };
    
    class AIScriptBinder
    {
    public:
        static AIScriptBinder *getInstance();
        
        void initConfig(lua_State *L);
        void resetDefaultConfig();
        
        bool isSearchTargetByLua(lua_State *L, int searchType);
        inline bool isHurtCalcExertHurtByLua() const { return _isHurtCalcExertHurtByLua; }
        bool isHurtCalcHurtTargetByLua(lua_State *L);
        bool isInnerEffectByLua(lua_State *L, int effid);
        bool isInnerEffectApplyHpchangeByLua(lua_State *L);
        bool isProjectileCasterThrowProjectileByLua(lua_State *L);
        bool isAttackActionUpdateByLua(lua_State *L);
        bool isAttackActionEnterByLua(lua_State *L);
        bool isActionSelectorByLua(lua_State *L);
        bool isLockTargetByLua(lua_State *L);
        bool isCanCastUltraSkillByLua(lua_State *L);
        bool isCheckTargetByLua(lua_State *L);
        bool isPickSkillByLua(lua_State *L);
        bool isUseNewTargetHelper(lua_State *L);
        bool isIgnorePositiveHurtValueHitLabelAndStat();
        bool isCaptainSelectorByLua(lua_State *L);
        bool isReshowUIEffectsWhenRevived(lua_State *L);
        bool isReshowMarkIconsWhenRevived(lua_State *L);
        bool isCasterSyncAttackSpeed(lua_State *L, int caster_id);
        bool isUseCannotBeAttackedBuffCnt(lua_State *L);
        bool isEffect175DisableLimit(lua_State *L);
		int getHitlabelDisplayType(lua_State *L, int labelType);

        bool getCommonConfigBoolean(lua_State *L, const char *config_name);
        bool getCommonConfigBooleanNoCache(lua_State *L, const char *config_name);
        int getCommonConfigInteger(lua_State *L, const char *config_name);
        int getCommonConfigIntegerNoCache(lua_State *L, const char *config_name);
        
    private:
        //bool _isSearchTargetByLua = false;
        std::map<int, bool> _searchTargetByLua;
        bool _isHurtCalcExertHurtByLua = false;
        std::map<int, bool> _innerEffectByLua;
        std::map<int, bool> _luaconfig;
        bool _isIgnorePositiveHurtValueHitLabelAndStat = false;
        std::map<int, bool> _casterSyncAttackSpeed;
		std::map<int, int> _hitlabelDisplayType;
        std::map<std::string, bool> _luaconfigCommonBoolean;
        std::map<std::string, int> _luaconfigCommonInteger;
    };
    
    extern void op_registertolua(lua_State *L);
    extern lua_State *op_createluastate();
    extern void op_closeluastate(lua_State *L);
}

#endif /* AIScriptBinder_hpp */
