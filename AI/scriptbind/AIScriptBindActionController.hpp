//
//  AIScriptBindActionController.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindActionController_hpp
#define AIScriptBindActionController_hpp

#include <list>
#include <vector>

struct lua_State;

namespace OnePiece {
    class CharacterData;
    class AIProjectile;
    class AIContext;
    class AITargetContext;
    
    extern int luaopen_aiactioncontroller(lua_State *L);
    
    extern void luahelper_searchtarget(lua_State *L, CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, std::list<CharacterData *> *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, std::list<CharacterData *> *outTargets);
    extern void luahelper_searchtargetwith(lua_State *L, CharacterData& source, CharacterData* target, AIProjectile* projectile, AIContext& aiContext, int searchType, int x1, int x2, int y1, int y2, int z1, int z2, std::vector<int> *searchParams, CharacterData& skillCaster, bool isUsingSkill, int theSkillId, int hurtID, int skillLevel, std::list<CharacterData *> *filterList, CharacterData *excludeCharacter, bool shouldHpRatioHigherThanExcludeCharacter, std::list<CharacterData *> *outTargets);
    extern void luahelper_searchtargetwith(lua_State *L, AITargetContext *ctx, std::list<CharacterData *> *out_targets); // 12.0

    extern bool luahelper_targethelper_is_target297findabletosearcher(lua_State *L, CharacterData *c, CharacterData *searcher, int realsearchtype);
    
    extern bool luahelper_actioncontroller_execute_exec(lua_State *L, int execID, int skillID, int skillLevel, CharacterData* source, AIProjectile* projectile, CharacterData* target, int parentTicklen, bool useLogicPos, int markID);

    extern void luahelper_actioncontroller_exit_action(lua_State *L, CharacterData *c);
    extern void luahelper_actioncontroller_did_enter_action(lua_State *L, CharacterData *c);
    extern void luahelper_actioncontroller_on_actionselector_turntotarget(lua_State *L, CharacterData *character);
    extern void luahelper_actioncontroller_on_moveaction_turntotarget(lua_State *L, CharacterData *character);
    extern void luahelper_actioncontroller_enter_deadaction(lua_State *L, CharacterData *character, int deadType, bool isFatal);
    extern void luahelper_actioncontroller_force_enter_deadaction(lua_State *L, CharacterData *character);
    extern void luahelper_actioncontroller_action_selector(lua_State *L, CharacterData *character);
    extern void luahelper_actioncontroller_captain_action_selector(lua_State *L, CharacterData *character);
    extern bool luahelper_actioncontroller_is_escape_effect_stateanim(lua_State *L, CharacterData *c);
    extern bool luahelper_actioncontroller_is_escape_hurt_result(lua_State *L, CharacterData *c);
    extern void luahelper_attackaction_update(lua_State *L, CharacterData *character);
    extern void luahelper_attackaction_enter(lua_State *L, CharacterData *c, int seqindex, int skid, int sklv, bool is_resetcd, int tickoffset, bool is_ultra); // 12.0
    extern int luahelper_attackaction_cast_ultra(lua_State *L, CharacterData *character, bool is_external);
    extern int luahelper_attackaction_check_target(lua_State *L, CharacterData *character, int castultraskill);
    extern int luahelper_attackaction_can_cast_ultra(lua_State *L, CharacterData *character, bool is_external);
    extern void luahelper_attackaction_on_exit(lua_State *L, CharacterData *c);
    extern bool luahelper_attackaction_force_end_accu(lua_State *L, CharacterData *c);
    extern void luahelper_outofcontrolaction_on_exit(lua_State *L, CharacterData *c);
	extern int luahelper_actioncontroller_is_move_action_effect(lua_State *L, CharacterData *c, int movetype);
    extern int luahelper_attackaction_force_cast_ultra(lua_State *L, CharacterData *c, int jx);
}

#endif /* AIScriptBindActionController_hpp */
