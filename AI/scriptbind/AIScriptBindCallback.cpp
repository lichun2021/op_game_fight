//
//  AIScriptBindCallback.cpp
//  op_game
//
//  Created by h7ing on 06/09/2017.
//
//

#include "AIScriptBindCallback.hpp"
#include "lua.hpp"
#if LUA_VERSION_NUM < 502
#include "compat-5.3.h"
#endif
#include "../AICallback.h"

namespace OnePiece {

    static inline const char *luatypename_aicallback() { return "aicallback"; }

    AICallback *luastack_getaicallback(lua_State *L, int idx)
    {
        if (lua_isnil(L, idx)) return nullptr;
        AICallback **lua_userdata = static_cast<AICallback **>(lua_touserdata(L, idx));
        return *lua_userdata;
    }

    void luastack_pushaicallback(lua_State *L, AICallback *a)
    {
        size_t sz = sizeof(AICallback *);
        AICallback **the_fulluserdata = static_cast<AICallback **>(lua_newuserdata(L, sz));
        *the_fulluserdata = a;
        
        luaL_getmetatable(L, luatypename_aicallback());
        lua_setmetatable(L, -2);
    }

    static int luabind_aicallback_start_schedule(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        a->startSchedule();
        return 0;
    }

    static int luabind_aicallback_stop_schedule(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int value = lua_tointeger(L, 2);
        int x = lua_tointeger(L, 3);
        int y = lua_tointeger(L, 4);
        int z = lua_tointeger(L, 5);
        a->stopSchedule(value, x, y, z);
        return 0;
    }

    static int luabind_aicallback_resume_schedule(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        a->resumeSchedule();
        return 0;
    }

    static int luabind_aicallback_monster_on_hit(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int monsterid = lua_tointeger(L, 2);
        int wavenum = lua_tointeger(L, 3);
        a->monsterOnHit(monsterid, wavenum);
        return 0;
    }

    static int luabind_aicallback_spawned_enemy_cluster(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int spawnedcount = lua_tointeger(L, 2);
        a->spawnedEnemyCluster(spawnedcount);
        return 0;
    }

    static int luabind_aicallback_on_create_projectile_for_warship(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        a->onCreateProjectileForWarship();
        return 0;
    }

    static int luabind_aicallback_fix_camera(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int x = lua_tointeger(L, 2);
        int y = lua_tointeger(L, 3);
        int z = lua_tointeger(L, 4);
        a->fixCamera(x, y, z);
        return 0;
    }

    static int luabind_aicallback_operation_track(lua_State *L)
    {
        //std::vector<TrackCannonDataPtr>& trackCannon, std::vector<std::pair<int, int>>& trackUltra, std::vector<int>& trackSetDistance, std::vector<std::pair<int, int>>& trackAutofight, TrackAccumulate& trackAccumulate, std::vector<int>& trackCenterX
        //auto a = luastack_getaicallback(L, 1);
        // TODO: operation_track
        return 0;
    }

    static int luabind_aicallback_character_enter_state(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int characteruid = lua_tointeger(L, 2);
        int state = lua_tointeger(L, 3);
        int skillid = lua_tointeger(L, 4);
        a->characterEnterState(characteruid, AICallbackCharacterState(state), skillid);
        return 0;
    }

    static int luabind_aicallback_character_exit_state(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int characteruid = lua_tointeger(L, 2);
        int state = lua_tointeger(L, 3);
        a->characterExitState(characteruid, AICallbackCharacterState(state));
        return 0;
    }

    static int luabind_aicallback_character_did_cast_ultra_skill(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int characteruid = lua_tointeger(L, 2);
        int staticdataid = lua_tointeger(L, 3);
        int teamside = lua_tointeger(L, 4);
        int ultratype = lua_tointeger(L, 5);
        int postertype = lua_tointeger(L, 6);
        int posternum = lua_tointeger(L, 7);
        a->characterDidCastUltraSkill(characteruid, staticdataid, teamside, ultratype, postertype, posternum);
        return 0;
    }

    static int luabind_aicallback_captain_cast_skill(lua_State *L)
    {
        auto a = luastack_getaicallback(L, 1);
        int captainuid = lua_tointeger(L, 2);
        int staticdataid = lua_tointeger(L, 3);
        int teamside = lua_tointeger(L, 4);
        int skillid = lua_tointeger(L, 5);
        a->captainCastSkill(captainuid, staticdataid, teamside, skillid);
        return 0;
    }
    
    int luaopen_aicallback(lua_State *L)
    {
        static const luaL_Reg reg[] = {
            { NULL, NULL }
        };
    
        static const luaL_Reg reg_m[] = {
            { "start_schedule", luabind_aicallback_start_schedule },
            { "stop_schedule", luabind_aicallback_stop_schedule },
            { "resume_schedule", luabind_aicallback_resume_schedule },
            // { "play_speed", },
            // { "fighting_complete", luabind_aicallback_fighting_complete },
            // { "monster_dead_reward", },
            { "monster_on_hit", luabind_aicallback_monster_on_hit }, 
            { "spawned_enemy_cluster", luabind_aicallback_spawned_enemy_cluster },
            { "on_create_projectile_for_warship", luabind_aicallback_on_create_projectile_for_warship }, 
            // { "remove_characters", },
            { "fix_camera", luabind_aicallback_fix_camera },
            // { "operation_track", luabind_aicallback_operation_track },
            { "character_enter_state", luabind_aicallback_character_enter_state },
            { "character_exit_state", luabind_aicallback_character_exit_state },
            { "character_did_cast_ultra_skill", luabind_aicallback_character_did_cast_ultra_skill },
            { "captain_cast_skill", luabind_aicallback_captain_cast_skill },
            { NULL, NULL }
        };
    
        luaL_newmetatable(L, luatypename_aicallback());  /* create metatable */
        lua_pushvalue(L, -1);  /* duplicate the metatable */
        lua_setfield(L, -2, "__index");  /* mt.__index = mt */
        luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
        luaL_newlib(L, reg);  /* create lib table */
        return 1;
    }
    
}
