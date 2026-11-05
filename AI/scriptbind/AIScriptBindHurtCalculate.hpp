//
//  AIScriptBindHurtCalculate.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindHurtCalculate_hpp
#define AIScriptBindHurtCalculate_hpp

struct lua_State;

namespace OnePiece {
    class HurtCalcParam;
    class CharacterData;
    class AIProjectile;
    class HurtExec;
    class InnerEffectExec;
    class HitResult;
    
    extern void luastack_pushhurtcalcparam(lua_State *L, const HurtCalcParam &param);
    extern int luastack_gethurtcalcparam(lua_State *L, int idx, HurtCalcParam &outparam);
    extern int luahelper_exerthurt(lua_State *L, CharacterData* source, AIProjectile* projectile, int hurtID, int skillID, int skillLevel, CharacterData* target, int theMarkID);
    extern void luahelper_characterdeadbyhurt(lua_State *L, CharacterData *pTarget, CharacterData *pSource, int skillID, int theSkillLevel, HurtExec *pHurtExec, InnerEffectExec* pEffectExec);
    extern int luahelper_hurttarget(lua_State *L, CharacterData *source, CharacterData *target, HurtCalcParam &param, HitResult &outresult);
}

#endif /* AIScriptBindHurtCalculate_hpp */
