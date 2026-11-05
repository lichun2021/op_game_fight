//
//  AIScriptBindBattleStat.cpp
//  op_game
//
//  Created by h7ing on 16/08/2017.
//
//

#include "AIScriptBindBattleStat.hpp"
#include "lua.hpp"
#if LUA_VERSION_NUM < 502
#include "compat-5.3.h"
#endif
#include "AIScriptBindCharacterData.hpp"
#include "../AIBattleStats.h"
#include "../HeroHurtcalculate.h"

namespace OnePiece {
    
    static int luacallback_append_hurt(lua_State *L)
    {
        auto source = luastack_getcharacterdata(L, 1);
        int hurttype = lua_tointeger(L, 2);
        int hpdelta = lua_tointeger(L, 3);
        auto target = luastack_getcharacterdata(L, 4);
        StatsFunc::AppendHeroHurt(*source, HurtType(hurttype), hpdelta, *target);
        return 0;
    }
    
    static int luacallback_reset_stats(lua_State *L)
    {
        StatsFunc::ResetStats();
        return 0;
    }
    
    int luaopen_battlestat(lua_State *L)
    {
        static const luaL_Reg reg[] = {
            { "append_hurt", luacallback_append_hurt },
            { "reset_stats", luacallback_reset_stats },
            { NULL, NULL }
        };
        
        luaL_newlib(L, reg);  /* create lib table */
        return 1;
    }
    
}
