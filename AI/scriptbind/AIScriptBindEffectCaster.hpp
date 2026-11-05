//
//  AIScriptBindEffectCaster.hpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#ifndef AIScriptBindEffectCaster_hpp
#define AIScriptBindEffectCaster_hpp

struct lua_State;

namespace OnePiece {

    class AIContinuousEffectCaster;
    class AIInnerEffect;
    class CharacterData;
    class AIProjectile;

    extern void luastack_pusheffectcaster(lua_State *L, AIContinuousEffectCaster *acaster);
    extern AIContinuousEffectCaster *luastack_geteffectcaster(lua_State *L, int idx);

    extern void luastack_pushinnereffect(lua_State *L, AIInnerEffect *aneff);
    extern AIInnerEffect *luastack_getinnereffect(lua_State *L, int idx);

    extern bool luahelper_innereffect_onenter(lua_State *L, AIInnerEffect *aneff);
    extern void luahelper_innereffect_onexit(lua_State *L, AIInnerEffect *aneff);
    extern void luahelper_innereffect_onscheduletick(lua_State *L, AIInnerEffect *aneff);
    extern bool luahelper_innereffect_check_delayed(lua_State *L, AIContinuousEffectCaster *e, CharacterData *c, int i);
    extern bool luahelper_innereffect_is_delayed(lua_State *L, AIContinuousEffectCaster *e, CharacterData *c, int i);
    extern int luahelper_innereffect_apply_hpchange(lua_State *L, CharacterData *source, AIProjectile *proj, CharacterData *target, int skillid, int skilllevel, int attridx, int changeval, int markid, int casterid, AIInnerEffect *eff);
    extern int luahelper_innereffect_273_trigger(lua_State *L, CharacterData *c, int param, bool is_simu);
    extern int luahelper_innereffect_282_check_do(lua_State *L, CharacterData *s, CharacterData *t);
    extern int luahelper_innereffect_283_check_do(lua_State *L, CharacterData *c, int hurttype, int hpdelta);
    extern void luahelper_innereffect_287_on_onetimehurt(lua_State *L, CharacterData *c);
    extern CharacterData *luahelper_innereffect_288_get_first_mosthpdelta_attacker(lua_State *L, CharacterData *c);
    extern int luahelper_innereffect_288_check_do_simu(lua_State *L, CharacterData *s, CharacterData *t, int param);
    extern int luahelper_innereffect_288_check_do(lua_State *L, CharacterData *s, CharacterData *t, int param);
    extern bool luahelper_innereffect_290_check_do(lua_State *L, CharacterData *s, CharacterData *t);
    extern int luahelper_innereffect_291_check_do(lua_State *L, CharacterData *s, CharacterData *t, int param);
    extern bool luahelper_innereffect_294_is_skillidblocked(lua_State *L, CharacterData *c, int skill_id);
    extern bool luahelper_innereffect_297_is_findable(lua_State *L, CharacterData *t, CharacterData *searcher);
    extern void luahelper_innereffect_302_check_do(lua_State *L, CharacterData *c);
	extern int luahelper_innereffect_295_checkdo(lua_State *L, CharacterData *c, int curaction);
    extern int luahelper_innereffect_312_trigger(lua_State *L, CharacterData *c, int hurttype, int hpdelta, bool is_simu);
    extern int luahelper_innereffect_315_trigger(lua_State *L, CharacterData *c, int endelta);
    extern int luahelper_innereffect_316_trigger(lua_State *L, CharacterData *c, int healdelta);
    extern void luahelper_innereffect_317_checkdo(lua_State *L, CharacterData *c, int dt);
    extern void luahelper_innereffect_318_checkdo(lua_State *L, CharacterData *c, int dt);
    extern int luahelper_innereffect_320_trigger(lua_State *L, CharacterData *c, int hurttype, int hurtprop, int hpdelta, bool is_simu);
	extern int luahelper_innereffect_325_checkdo(lua_State *L, CharacterData *c, int idx);
    extern void luahelper_innereffect_326_trigger(lua_State *L, CharacterData *s, CharacterData *t, int skid, int sklv);
	extern void luahelper_innereffect_327_trigger(lua_State *L, CharacterData *c, CharacterData *attacker, int hurttype, int hurtprop);
	extern void luahelper_innereffect_328_trigger(lua_State *L, CharacterData *c, int skillid, int skilllv, int sktype, bool isforce, int enterorexit); // enterorexit:0-enter 1-exit
    extern int luahelper_innereffect_10001_trigger(lua_State *L, CharacterData *c, int hurttype, int hpdelta, bool is_simu, int hittype);

    extern int luaopen_aieffectcaster(lua_State *L);
    extern int luaopen_aiinnereffect(lua_State *L);

    extern bool luahelper_effectcaster_is_group_covered(lua_State *L, CharacterData *source, CharacterData *target, int execid);
    extern void luahelper_effectcaster_check_cover_group_casters(lua_State *L, CharacterData *target, int execid);
}

#endif /* AIScriptBindEffectCaster_hpp */
