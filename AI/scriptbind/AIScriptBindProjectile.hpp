//
//  AIScriptBindProjectile.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindProjectile_hpp
#define AIScriptBindProjectile_hpp

struct lua_State;

namespace OnePiece {
    
    class AIProjectile;
    class CharacterData;
    class AIContinuousProjectileCaster;
    
    extern void luastack_pushprojectile(lua_State *L, AIProjectile *the_projectile);
    extern AIProjectile *luastack_getprojectile(lua_State *L, int idx);
    
    extern int luaopen_projectile(lua_State *L);
    
    extern void luastack_pushprojectilecaster(lua_State *L, AIContinuousProjectileCaster *the_projectile);
    extern AIContinuousProjectileCaster *luastack_getprojectilecaster(lua_State *L, int idx);
    extern int luaopen_projectilecaster(lua_State *L);
    
    extern void luahelper_projectilecaster_throw_projectile(lua_State *L, AIContinuousProjectileCaster *c, bool shouldturn, bool shouldthrow);
}

#endif /* AIScriptBindProjectile_hpp */
