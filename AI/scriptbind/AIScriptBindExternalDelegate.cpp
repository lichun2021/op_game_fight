#include "AIScriptBindExternalDelegate.hpp"
#include "lua.hpp"
#if LUA_VERSION_NUM < 502
#include "compat-5.3.h"
#endif
#include "cocos2d.h"
#ifndef SERVER_VERIFYING
#include "SimpleAudioEngine.h"
#endif

#include "AIScriptBindCharacterData.hpp"
#include "AIScriptBindProjectile.hpp"
#include "../AIContext.h"
#include "../ExternalDelegateAnimation.h"
#include "AIScriptBindHelper.h"

namespace OnePiece {
    static inline const char *luatypename_externaldelegate() { return "aiexternaldelegate"; }
    
    // MARK: -- animation
    static int luabind_animation_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        int execid = lua_tointeger(L, 1);
        int sklv = lua_tointeger(L, 2);
        auto c = luastack_getcharacterdata(L, 3);
        auto pr = luastack_getprojectile(L, 4);
        auto t = luastack_getcharacterdata(L, 5);
        int tick = lua_tointeger(L, 6);
        ap->executeExec(*ai, execid, sklv, c, pr, t, tick);
        return 0;
    }

    static int luabind_animation_resync(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        ap->resyncCharacter(*ai, c);
        return 0;
    }

    static int luabind_animation_forceturn(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        ap->forceTurnCharacter(*ai, c);
        return 0;
    }

    static int luabind_animation_revive(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        ap->reviveCharacter(*ai, c);
        return 0;
    }

    static int luabind_animation_enter_copy(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        auto cs = luastack_getcharacterdata(L, 2);
        ap->enterCopyCharacter(*ai, c, cs);
        return 0;
    }

    static int luabind_animation_exit_copy(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        ap->exitCopyCharacter(*ai, c);
        return 0;
    }

    static int luabind_animation_enter_state(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int s = lua_tointeger(L, 2);
        int r = lua_tointeger(L, 3);
        int g = lua_tointeger(L, 4);
        int b = lua_tointeger(L, 5);
        int sp = lua_tointeger(L, 6);
        ap->enterStateAnimation(*ai, c, s, r, g, b, sp);
        return 0;
    }

    static int luabind_animation_exit_state(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->animationProxy;
        if (!ap) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int s = lua_tointeger(L, 2);
        ap->exitStateAnimation(*ai, c, s);
        return 0;
    }

    // MARK: animation --
    
    // MARK: -- audio
    static int luabind_audio_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto ap = ai->audioProxy;
        if (!ap) return 0;

        int execid = lua_tointeger(L, 1);
        int sklv = lua_tointeger(L, 2);
        auto c = luastack_getcharacterdata(L, 3);
        auto pr = luastack_getprojectile(L, 4);
        auto t = luastack_getcharacterdata(L, 5);
        int tick = lua_tointeger(L, 6);
        ap->executeExec(*ai, execid, sklv, c, pr, t, tick);
        return 0;
    }
    // MARK: audio --

    // MARK: -- bodyeffect
    static int luabind_bodyeffect_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto bep = ai->bodyEffectProxy;
        if (!bep) return 0;

        int parentexecid = lua_tointeger(L, 1);
        int bodyeffectid = lua_tointeger(L, 2);
        auto c = luastack_getcharacterdata(L, 3);
        bep->executeExec(*ai, parentexecid, bodyeffectid, c);
        return 0;
    }

    static int luabind_bodyeffect_notify(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto bep = ai->bodyEffectProxy;
        if (!bep) return 0;

        int parentexecid = lua_tointeger(L, 1);
        bep->notify(*ai, parentexecid);
        return 0;
    }

    static int luabind_bodyeffect_clear(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto bep = ai->bodyEffectProxy;
        if (!bep) return 0;

        bep->clear();
        return 0;
    }
    // MARK: bodyeffect --

    // MARK: -- combo
    static int luabind_combo_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto cp = ai->comboProxy;
        if (!cp) return 0;

        int comboid = lua_tointeger(L, 1);
        int internalid = lua_tointeger(L, 2);
        int targetid = lua_tointeger(L, 3);
        int x = lua_tointeger(L, 4);
        int y = lua_tointeger(L, 5);
        int totaltick = lua_tointeger(L, 6);
        cp->executeExec(comboid, internalid, targetid, x, y, totaltick);
        return 0;
    }
    // MARK: combo --

    // MARK: -- markeffect

    static int luabind_markeffect_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->markProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int mkid = lua_tointeger(L, 2);
        int mklv = lua_tointeger(L, 3);
        p->executeExec(*ai, c, mkid, mklv);
        return 0;
    }

    static int luabind_markeffect_notify(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->markProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int mkid = lua_tointeger(L, 2);
        p->notify(*ai, c, mkid);
        return 0;
    }
    // MARK: markeffect --

    // MARK: -- specialeffect

    static int luabind_specialeffect_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->seffProxy;
        if (!p) return 0;

        int parentexecid = lua_tointeger(L, 1);
        int sefid = lua_tointeger(L, 2);
        int sklv = lua_tointeger(L, 3);
        auto c = luastack_getcharacterdata(L, 4);
        auto pr = luastack_getprojectile(L, 5);
        auto t = luastack_getcharacterdata(L, 6);
        auto parenttick = lua_tointeger(L, 7);
        bool uselogicpos = lua_toboolean(L, 8);
        int alttick = lua_tointeger(L, 9);
        p->executeExec(*ai, parentexecid, sefid, sklv, c, pr, t, parenttick, uselogicpos, alttick);
        return 0;
    }

    static int luabind_specialeffect_notify(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->seffProxy;
        if (!p) return 0;

        int parentexecid = lua_tointeger(L, 1);
        p->notify(*ai, parentexecid);
        return 0;
    }

    // MARK: specialeffect --
    
    // MARK: -- specialexec
    static int luabind_specialexec_execute(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->sexecProxy;
        if (!p) return 0;

        int execid = lua_tointeger(L, 1);
        int sklv = lua_tointeger(L, 2);
        auto c = luastack_getcharacterdata(L, 3);
        auto pr = luastack_getprojectile(L, 4);
        auto t = luastack_getcharacterdata(L, 5);
        int parenttick = lua_tointeger(L, 6);
        p->executeExec(*ai, execid, sklv, c, pr, t, parenttick);
        return 0;
    }

    static int luabind_specialexec_stopscreen(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->sexecProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        p->stopScreenExec(*ai, c);
        return 0;
    }

    // MARK: specialexec --

    // MARK: -- summon

    static int luabind_summon_create(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->summonProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int sid = lua_tointeger(L, 2);
        p->createSummon(*ai, c, sid);
        return 0;
    }

    static int luabind_summon_awake(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->summonProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int sid = lua_tointeger(L, 2);
        p->awakeSummon(*ai, c, sid);
        return 0;
    }

    static int luabind_summon_sleep(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->summonProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int sid = lua_tointeger(L, 2);
        p->sleepSummon(*ai, c, sid);
        return 0;
    }

    static int luabind_summon_hideorshow(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->summonProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        bool ishide = lua_toboolean(L, 2);
        p->hideOrShowSource(*ai, c, ishide);
        return 0;
    }

    static int luabind_summon_notifyclear(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->summonProxy;
        if (!p) return 0;

        int sid = lua_tointeger(L, 1);
        p->notifyClearSummon(sid);
        return 0;
    }

    // MARK: summon --

    // MARK: -- transform
    static int luabind_transform_create(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->transformProxy;
        if (!p) return 0;
        
        auto c = luastack_getcharacterdata(L, 1);
        int charid = lua_tointeger(L, 2);
        p->createTransformCreature(*ai, c, charid);
        return 0;
    }

    static int luabind_transform_to(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->transformProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int charid = lua_tointeger(L, 2);
        p->transformToCreature(*ai, c, charid);
        return 0;
    }

    static int luabind_transform_clear_transformer(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->transformProxy;
        if (!p) return 0;

        int uid = lua_tointeger(L, 1);
        p->clearTransformer(uid);
        return 0;
    }

    static int luabind_transform_enter_to(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->transformProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        int charid = lua_tointeger(L, 2);
        p->enterTransformToCharacter(*ai, c, charid);
        return 0;
    }

    static int luabind_transform_exit(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->transformProxy;
        if (!p) return 0;

        auto c = luastack_getcharacterdata(L, 1);
        p->exitTransform(c);
        return 0;
    }

    static int luabind_transform_clear(lua_State *L)
    {
        auto ai = AIContext::getInstance();
        auto p = ai->transformProxy;
        if (!p) return 0;

        p->clear();
        return 0;
    }

    // MARK: transform --

    // MARK: -- audioengine

    static int luabind_audioengine_get_effects_volume(lua_State *L)
    {
        float luaret = 0;
#ifndef SERVER_VERIFYING
        luaret = CocosDenshion::SimpleAudioEngine::getInstance()->getEffectsVolume();
#endif
        lua_pushnumber(L, luaret);
        return 1;
    }

    static int luabind_audioengine_get_background_volume(lua_State *L)
    {
        float luaret = 0;
#ifndef SERVER_VERIFYING
        luaret = CocosDenshion::SimpleAudioEngine::getInstance()->getBackgroundMusicVolume();
#endif
        lua_pushnumber(L, luaret);
        return 1;
    }

    static int luabind_audioengine_set_effects_volume(lua_State *L)
    {
#ifndef SERVER_VERIFYING
        float vol = lua_tonumber(L, 1);
        CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(vol);
#endif
        return 0;
    }

    static int luabind_audioengine_set_background_volume(lua_State *L)
    {
#ifndef SERVER_VERIFYING
        float vol = lua_tonumber(L, 1);
        CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(vol);
#endif
        return 0;
    }

#ifndef SERVER_VERIFYING
    class AudioEngineFadeTo : public cocos2d::ActionInterval
    {
    public:
        static AudioEngineFadeTo* create(float duration, float tovol, int voltype) {
            AudioEngineFadeTo* action = new AudioEngineFadeTo();
            action->initWithDuration(duration, tovol, voltype);
            action->autorelease();
            return action;
        }

        virtual AudioEngineFadeTo* clone() const override {
            auto a = new AudioEngineFadeTo();
            a->initWithDuration(_duration, _tovol, _voltype);
            a->autorelease();
            return a;
        }
        virtual AudioEngineFadeTo* reverse(void) const override {
            return nullptr;
        }
        virtual void startWithTarget(cocos2d::Node *target) override {
            cocos2d::ActionInterval::startWithTarget(target);
            if (_voltype == 0)
                _fromvol = CocosDenshion::SimpleAudioEngine::getInstance()->getEffectsVolume();
            else
                _fromvol = CocosDenshion::SimpleAudioEngine::getInstance()->getBackgroundMusicVolume();
        }
        virtual void update(float time) override {
            float vol = _fromvol + (_tovol - _fromvol) * time;
            if (_voltype == 0)
                CocosDenshion::SimpleAudioEngine::getInstance()->setEffectsVolume(vol);
            else
                CocosDenshion::SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(vol);
        }

    public:
        AudioEngineFadeTo() {}
        virtual ~AudioEngineFadeTo() {}

        bool initWithDuration(float duration, float tovol, int voltype) {
            if (cocos2d::ActionInterval::initWithDuration(duration)) {
                _tovol = tovol;
                _voltype = voltype;
                return true;
            }
            return false;
        }

    private:
        int _voltype = 0; // 0 - effects, 1 - background
        float _tovol = 0;
        float _fromvol = 0;
    };
#endif

    static int luabind_audioengine_fade_to_volume(lua_State *L)
    {
#ifndef SERVER_VERIFYING
        float d = lua_tonumber(L, 1);
        float v = lua_tonumber(L, 2);
        int t = lua_tointeger(L, 3);
        int tag = lua_tointeger(L, 4);
        auto act = AudioEngineFadeTo::create(d, v, t);
        act->setTag(tag);
        auto s = cocos2d::Director::getInstance()->getRunningScene();
        if (s)
        {
            if (tag != cocos2d::Action::INVALID_TAG)
                s->stopActionByTag(tag);
            s->runAction(act);
        }
#endif
        return 0;
    }

    // MARK: audioengine --

    int luaopen_externaldelegate(lua_State *L)
    {
        static const luaL_Reg reg[] = {
            { "animation_execute", luabind_animation_execute },
            { "animation_resync", luabind_animation_resync },
            { "animation_forceturn", luabind_animation_forceturn },
            { "animation_revive", luabind_animation_revive },
            { "animation_enter_copy", luabind_animation_enter_copy },
            { "animation_exit_copy", luabind_animation_exit_copy },
            { "animation_enter_state", luabind_animation_enter_state },
            { "animation_exit_state", luabind_animation_exit_state },

            { "audio_execute", luabind_audio_execute },

            { "bodyeffect_execute", luabind_bodyeffect_execute },
            { "bodyeffect_notify", luabind_bodyeffect_notify },
            { "bodyeffect_clear", luabind_bodyeffect_clear },

            { "combo_execute", luabind_combo_execute},

            { "markeffect_execute", luabind_markeffect_execute },
            { "markeffect_notify", luabind_markeffect_notify },

            { "specialeffect_execute", luabind_specialeffect_execute },
            { "specialeffect_notify", luabind_specialeffect_notify },

            { "specialexec_execute", luabind_specialexec_execute },
            { "specialexec_stopscreen", luabind_specialexec_stopscreen },

            { "summon_create", luabind_summon_create },
            { "summon_awake", luabind_summon_awake },
            { "summon_sleep", luabind_summon_sleep },
            { "summon_hideorshow", luabind_summon_hideorshow },
            { "summon_notifyclear", luabind_summon_notifyclear },

            { "transform_create", luabind_transform_create },
            { "transform_to", luabind_transform_to },
            { "transform_clear_transformer", luabind_transform_clear_transformer },
            { "transform_enter_to", luabind_transform_enter_to },
            { "transform_exit", luabind_transform_exit },
            { "transform_clear", luabind_transform_clear },

            { "audioengine_get_effects_volume", luabind_audioengine_get_effects_volume },
            { "audioengine_get_background_volume", luabind_audioengine_get_background_volume },
            { "audioengine_set_effects_volume", luabind_audioengine_set_effects_volume },
            { "audioengine_set_background_volume", luabind_audioengine_set_background_volume },
            { "audioengine_fade_to_volume", luabind_audioengine_fade_to_volume },

            { NULL, NULL }
        };
    
        static const luaL_Reg reg_m[] = {
            { NULL, NULL }
        };
        
        luaL_newmetatable(L, luatypename_externaldelegate());  /* create metatable */
        lua_pushvalue(L, -1);  /* duplicate the metatable */
        lua_setfield(L, -2, "__index");  /* mt.__index = mt */
        luaL_setfuncs(L, reg_m, 0);  /* register metamethods */
        luaL_newlib(L, reg);  /* create lib table */
        return 1;
    }

    bool luahelper_uiunit_get_adjust_shadowscale_by_deltay(lua_State *L, float oldscale, int deltay, float *out_adjust_scale, unsigned char *out_adjust_opacity)
    {
        lua_getglobal(L, "g_uiunithelper");
        lua_getfield(L, -1, "adjust_shadowscale_by_deltay");
        lua_remove(L, -2);
        lua_pushnumber(L, oldscale);
        lua_pushinteger(L, deltay);
        if (lua_pcall(L, 2, 3, 0)) {
            luahelper_errorlog(L);
            return false;
        }

        bool is_needadjust = false;
        if (lua_isboolean(L, -3)) {
            is_needadjust = lua_toboolean(L, -3);
        }
        
        if (out_adjust_scale && lua_isnumber(L, -2))
            *out_adjust_scale = lua_tonumber(L, -2);

        if (out_adjust_opacity && lua_isinteger(L, -1))
            *out_adjust_opacity = lua_tointeger(L, -1);

        lua_pop(L, 3);

        lua_settop(L, 0);
        return is_needadjust;
    }

    void luahelper_uiunit_get_armorbar_config(lua_State *L, std::string *out_icon_path, std::string *out_bar_path, float *out_icon_pos_x, float *out_icon_pos_y, float *out_bar_pos_x, float *out_bar_pos_y)
    {
        lua_getglobal(L, "g_uiunithelper");
        lua_getfield(L, -1, "get_armorbar_config");
        lua_remove(L, -2);

        if (lua_pcall(L, 0, 6, 0)) {
            luahelper_errorlog(L);
            return;
        }

        if (lua_isstring(L, -6) && out_icon_path) {
            *out_icon_path = lua_tostring(L, -6);
        }

        if (lua_isstring(L, -5) && out_bar_path) {
            *out_bar_path = lua_tostring(L, -5);
        }

        if (lua_isnumber(L, -4) && out_icon_pos_x) {
            *out_icon_pos_x = lua_tonumber(L, -4);
        }

        if (lua_isnumber(L, -3) && out_icon_pos_y) {
            *out_icon_pos_y = lua_tonumber(L, -3);
        }

        if (lua_isnumber(L, -2) && out_bar_pos_x) {
            *out_bar_pos_x = lua_tonumber(L, -2);
        }

        if (lua_isnumber(L, -1) && out_bar_pos_y) {
            *out_bar_pos_y = lua_tonumber(L, -1);
        }

        lua_pop(L, 6);
    }
}
