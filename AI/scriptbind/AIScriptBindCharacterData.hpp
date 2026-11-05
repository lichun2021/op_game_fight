//
//  AIScriptBindCharacterData.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindCharacterData_hpp
#define AIScriptBindCharacterData_hpp

struct lua_State;

namespace OnePiece {
    class CharacterData;
    class AIProjectile;
    
    extern void luastack_pushcharacterdata(lua_State *L, CharacterData *source);
    extern CharacterData *luastack_getcharacterdata(lua_State *L, int idx);
    
    extern void luahelper_characterdata_reset_register(lua_State *L);
    
    extern int luaopen_characterdata(lua_State *L);

    extern CharacterData *luahelper_characterdata_locktarget(lua_State *L, CharacterData *character, bool isJudgeBy297);

    extern bool luahelper_characterdata_canbeattackbyskill(lua_State *L, CharacterData *c, CharacterData *attacker, int skillid, AIProjectile *p);

    extern bool luahelper_characterdata_is_rival_team_all_dead_for_check_ultra(lua_State *L, CharacterData *c);
}

#endif /* AIScriptBindCharacterData_hpp */
