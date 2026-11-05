//
//  AIScriptBindCallback.hpp
//  op_game
//
//  Created by h7ing on 06/09/2017.
//
//

#ifndef AIScriptBindCallback_hpp
#define AIScriptBindCallback_hpp

struct lua_State;

namespace OnePiece {
    class AICallback;

    extern AICallback *luastack_getaicallback(lua_State *L, int idx);
    extern void luastack_pushaicallback(lua_State *L, AICallback *a);

    extern int luaopen_aicallback(lua_State *L);
}


#endif /* AIScriptBindCallback_hpp */
