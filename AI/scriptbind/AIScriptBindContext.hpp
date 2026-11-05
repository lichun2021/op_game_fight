//
//  AIScriptBindContext.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindContext_hpp
#define AIScriptBindContext_hpp

struct lua_State;

namespace OnePiece {

    class CharacterData;
    
    extern int luaopen_aicontext(lua_State *L);
    
    extern bool luahelper_aicontext_clear(lua_State *L);
    extern bool luahelper_aicontext_clear_client(lua_State *L);
    extern bool luahelper_aicontext_clear_server(lua_State *L);
    extern void luahelper_aicontext_addcharacter(lua_State *L, CharacterData *character);
    extern void luahelper_aicontext_addcaptain(lua_State *L, CharacterData *captain);
    extern void luahelper_aicontext_to_nextcluster_end(lua_State *L, int adjust, int centerx);
    extern void luahelper_aicontext_on_battle_finished(lua_State *L);
    extern void luahelper_aicontext_on_update_end(lua_State *L);
    extern void luahelper_aicontext_on_to_nextcluster_init_cluster(lua_State *L, int clusternum_our, int clusternum_enemy);
}

#endif /* AIScriptBindContext_hpp */
