//
//  AIScriptBindSkillController.hpp
//  op_game
//
//  Created by h7ing on 29/08/2017.
//
//

#ifndef AIScriptBindSkillController_hpp
#define AIScriptBindSkillController_hpp

#include <memory>

struct lua_State;

namespace OnePiece {
    class CharacterData;
    class SkillSeqInfo;
    class SpecialSkillInfo;

    extern void luastack_pushspecialskillinfo(lua_State *L, SpecialSkillInfo *info);
    extern SpecialSkillInfo *luastack_getspecialskillinfo(lua_State *L, int idx);
    extern int luaopen_specialskillinfo(lua_State *L);

    extern int luaopen_aiskillcontroller(lua_State *L);

    extern void luahelper_skillcontroller_pickskill(lua_State *L, CharacterData *character, SkillSeqInfo *skillseqinfo);
}

#endif /* AIScriptBindSkillController_hpp */
