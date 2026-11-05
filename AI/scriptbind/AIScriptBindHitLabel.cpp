//
//  AIScriptBindHitLabel.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindHitLabel.hpp"
#include "lua.hpp"
#if LUA_VERSION_NUM < 502
#include "compat-5.3.h"
#endif
#include "AIScriptBindCharacterData.hpp"
#include "../AISpecialEffectLabel.h"
#include "../AIContext.h"
#include "../AICharacterData.h"

namespace OnePiece {

// MARK: -- Lua binding hitlabel
static int luacallback_hitlabel_execute(lua_State *L)
{
    int labelid = lua_tointeger(L, 1);
    int num = lua_tointeger(L, 2);
    auto target = luastack_getcharacterdata(L, 3);
    AISpecialEffectLabel::excute(labelid, num, *AIContext::getInstance(), target);
    return 0;
}

int luaopen_hitlabel(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "execute", luacallback_hitlabel_execute },
        { NULL, NULL }
    };
    
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}
// MARK: Lua binding hitlabel --

}
