//
//  AIScriptBindSummonCaster.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindSummonCaster_hpp
#define AIScriptBindSummonCaster_hpp

struct lua_State;

namespace OnePiece {
    class CharacterData;
    class AIContinuousSummonCaster;

    extern int luaopen_summoncaster(lua_State *L);

    extern void luastack_pushsummoncaster(lua_State *L, AIContinuousSummonCaster *caster);
    extern AIContinuousSummonCaster *luastack_getsummoncaster(lua_State *L, int idx);

    extern void luahelper_summoncaster_init_summonpool(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summoner, int summon_id);
    extern void luahelper_summoncaster_on_summon_awake(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summon, int summon_id);
    extern int luahelper_summoncaster_get_group_summon_available_count(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summoner);
    extern void luahelper_summoncaster_check_sleep_group_summon(lua_State *L, AIContinuousSummonCaster *caster, CharacterData *summoner, int exec_id, int group_id, int group_weight, int group_max_num, int current_group_summon_num);
    extern void luahelper_summoncaster_on_finish_awake_summon(lua_State *L, int caster_myid, CharacterData *summoner);
}

#endif /* AIScriptBindSummonCaster_hpp */
