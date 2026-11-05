//
//  AIScriptBindMarkCaster.hpp
//  op_game
//
//  Created by h7ing on 04/09/2017.
//
//

#ifndef AIScriptBindMarkCaster_hpp
#define AIScriptBindMarkCaster_hpp

struct lua_State;

namespace OnePiece {
    class AIContinuousMarkCaster;

    extern AIContinuousMarkCaster *luastack_getmarkcaster(lua_State *L, int idx);
    extern void luastack_pushmarkcaster(lua_State *L, AIContinuousMarkCaster *m);

    extern int luaopen_markcaster(lua_State *L);
}

#endif /* AIScriptBindMarkCaster_hpp */
