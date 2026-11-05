//
//  AIScriptBindProjectile.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIScriptBindProjectile.hpp"
#include "lua.hpp"
#include "AIScriptBindHelper.h"
#include "AIScriptBindCharacterData.hpp"
#include "../AIProjectile.h"
#include "../projectile/ProjectileForBouncing.h"
#include "../projectile/ProjectileForCharacter.h"
#include "../projectile/ProjectileForLanding.h"
#include "../projectile/ProjectileForWarship.h"
#include "../AIContinuousProjectileCaster.h"

namespace OnePiece {

// MARK: -- Lua binding projectile
static inline const char *luatypename_projectile() { return "aiprojectile"; }

void luastack_pushprojectile(lua_State *L, AIProjectile *the_projectile)
{
    size_t sz = sizeof(AIProjectile *);
    AIProjectile **the_fulluserdata = static_cast<AIProjectile **>(lua_newuserdata(L, sz));
    *the_fulluserdata = the_projectile;
    
    luaL_getmetatable(L, luatypename_projectile());
    lua_setmetatable(L, -2);
}

AIProjectile *luastack_getprojectile(lua_State *L, int idx)
{
    if (lua_isnil(L, idx)) return nullptr;
    AIProjectile **lua_userdata = static_cast<AIProjectile **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luabind_new_projectile_for_bouncing(lua_State *L)
{
    int projid = lua_tointeger(L, 1);
    auto o = luastack_getcharacterdata(L, 2);
    auto t = luastack_getcharacterdata(L, 3);
    int sid = lua_tointeger(L, 4);
    int slv = lua_tointeger(L, 5);
    new ProjectileForBouncing(*AIContext::getInstance(), projid, o, t, sid, slv);
    return 0;
}

static int luabind_new_projectile_for_character(lua_State *L)
{
    int projid = lua_tointeger(L, 1);
    auto o = luastack_getcharacterdata(L, 2);
    auto t = luastack_getcharacterdata(L, 3);
    int sid = lua_tointeger(L, 4);
    int slv = lua_tointeger(L, 5);
    new ProjectileForCharacter(*AIContext::getInstance(), projid, o, t, sid, slv);
    return 0;
}

static int luabind_new_projectile_for_landing(lua_State *L)
{
    int projid = lua_tointeger(L, 1);
    auto o = luastack_getcharacterdata(L, 2);
    auto t = luastack_getcharacterdata(L, 3);
    int tx = lua_tointeger(L, 4);
    int ty = lua_tointeger(L, 5);
    int tz = lua_tointeger(L, 6);
    int sid = lua_tointeger(L, 7);
    int slv = lua_tointeger(L, 8);
    new ProjectileForLanding(*AIContext::getInstance(), projid, o, t, tx, ty, tz, sid, slv);
    return 0;
}

static int luabind_new_projectile_for_warship(lua_State *L)
{
    int projid = lua_tointeger(L, 1);
    auto o = luastack_getcharacterdata(L, 2);
    int sx = lua_tointeger(L, 3);
    int sy = lua_tointeger(L, 4);
    int sz = lua_tointeger(L, 5);
    int tx = lua_tointeger(L, 5);
    int ty = lua_tointeger(L, 6);
    int tz = lua_tointeger(L, 7);
    int sid = lua_tointeger(L, 8);
    int slv = lua_tointeger(L, 9);
    new ProjectileForWarship(*AIContext::getInstance(), projid, o, sx, sy, sz, tx, ty, tz, sid, slv);
    return 0;
}

static int luabind_projectile_get_myid(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->myID);
    return 1;
}

static int luabind_projectile_get_projectileid(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->projectileID);
    return 1;
}

static int luabind_projectile_get_origin(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    luastack_pushcharacterdata(L, the_projectile->origin);
    return 1;
}

static int luabind_projectile_get_skillid(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->skillID);
    return 1;
}

static int luabind_projectile_get_skilllevel(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->skillLevel);
    return 1;
}

static int luabind_projectile_get_currentx(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->currentX);
    return 1;
}

static int luabind_projectile_get_currenty(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->currentY);
    return 1;
}

static int luabind_projectile_get_distancefromlasthit(lua_State *L)
{
    auto the_projectile = luastack_getprojectile(L, 1);
    lua_pushinteger(L, the_projectile->getDistanceFromLastHit());
    return 1;
}

int luaopen_projectile(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { "new_projectile_for_bouncing", luabind_new_projectile_for_bouncing },
        { "new_projectile_for_character", luabind_new_projectile_for_character },
        { "new_projectile_for_landing", luabind_new_projectile_for_landing },
        { "new_projectile_for_warship", luabind_new_projectile_for_warship },
        { NULL, NULL }
    };
    
    static const luaL_Reg reg_m[] = {
        { "get_myid", luabind_projectile_get_myid},
        { "get_projectileid", luabind_projectile_get_projectileid },
        { "get_origin", luabind_projectile_get_origin },
        { "get_skillid", luabind_projectile_get_skillid },
        { "get_skilllevel", luabind_projectile_get_skilllevel },
        { "get_currentx", luabind_projectile_get_currentx},
        { "get_currenty", luabind_projectile_get_currenty},
        { "get_distancefromlasthit", luabind_projectile_get_distancefromlasthit },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, luatypename_projectile());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}
// MARK: Lua binding projectile --


// MARK: -- lua binding projectilecaster

static inline const char *luatypename_projectilecaster() { return "aiprojectilecaster"; }

void luastack_pushprojectilecaster(lua_State *L, AIContinuousProjectileCaster *the_projectile)
{
    size_t sz = sizeof(AIContinuousProjectileCaster *);
    AIContinuousProjectileCaster **the_fulluserdata = static_cast<AIContinuousProjectileCaster **>(lua_newuserdata(L, sz));
    *the_fulluserdata = the_projectile;
    
    luaL_getmetatable(L, luatypename_projectilecaster());
    lua_setmetatable(L, -2);
}

AIContinuousProjectileCaster *luastack_getprojectilecaster(lua_State *L, int idx)
{
    if (lua_isnil(L, idx)) return nullptr;
    AIContinuousProjectileCaster **lua_userdata = static_cast<AIContinuousProjectileCaster **>(lua_touserdata(L, idx));
    return *lua_userdata;
}

static int luabind_projectilecaster_get_myid(lua_State *L)
{
    auto c = luastack_getprojectilecaster(L, 1);
    lua_pushinteger(L, c->myID);
    return 1;
}

static int luabind_projectilecaster_get_projectileid(lua_State *L)
{
    auto c = luastack_getprojectilecaster(L, 1);
    lua_pushinteger(L, c->projectileID);
    return 1;
}

static int luabind_projectilecaster_get_origin(lua_State *L)
{
    auto c = luastack_getprojectilecaster(L, 1);
    luastack_pushcharacterdata(L, c->origin);
    return 1;
}

static int luabind_projectilecaster_get_skillid(lua_State *L)
{
    auto c = luastack_getprojectilecaster(L, 1);
    lua_pushinteger(L, c->skillID);
    return 1;
}

static int luabind_projectilecaster_get_skilllevel(lua_State *L)
{
    auto c = luastack_getprojectilecaster(L, 1);
    lua_pushinteger(L, c->skillLevel);
    return 1;
}

static int luabind_projectilecaster_impl_throw_projectile(lua_State *L)
{
    auto c = luastack_getprojectilecaster(L, 1);
    bool shouldturn = lua_toboolean(L, 2);
    bool shouldthrow = lua_toboolean(L, 3);
    c->implThrowProjectileNative(*AIContext::getInstance(), shouldturn, shouldthrow);
    return 0;
}

int luaopen_projectilecaster(lua_State *L)
{
    static const luaL_Reg reg[] = {
        { NULL, NULL }
    };

    static const luaL_Reg reg_m[] = {
        { "get_myid", luabind_projectilecaster_get_myid},
        { "get_projectileid", luabind_projectilecaster_get_projectileid },
        { "get_origin", luabind_projectilecaster_get_origin },
        { "get_skillid", luabind_projectilecaster_get_skillid },
        { "get_skilllevel", luabind_projectilecaster_get_skilllevel },
        { "impl_throw_projectile", luabind_projectilecaster_impl_throw_projectile },
        { NULL, NULL }
    };
    
    luaL_newmetatable(L, luatypename_projectilecaster());  /* create metatable */
    lua_pushvalue(L, -1);  /* duplicate the metatable */
    lua_setfield(L, -2, "__index");  /* mt.__index = mt */
    luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
    luaL_newlib(L, reg);  /* create lib table */
    return 1;
}

// MARK: lua binding projectilecaster -- 

void luahelper_projectilecaster_throw_projectile(lua_State *L, AIContinuousProjectileCaster *c, bool shouldturn, bool shouldthrow)
{
    lua_getglobal(L, "g_projectilehelper");
    lua_getfield(L, -1, "projectilecaster_throw_projectile");

    luastack_pushprojectilecaster(L, c);
    lua_pushboolean(L, shouldturn);
    lua_pushboolean(L, shouldthrow);

    if (lua_pcall(L, 3, 0, 0))
    {
        luahelper_errorlog(L);
    }
    
    lua_settop(L, 0);
}

}
