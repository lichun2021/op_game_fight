#include "AIScriptBindHelper.h"
#include "lua.hpp"
#include "cocos2d.h"
#ifndef SERVER_VERIFYING
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
#include "../../../SDK/Bugly/IOS/CrashReport.h"
#elif (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "bugly/CrashReport.h"
#endif
#endif

namespace OnePiece {

    void luahelper_errorlog(lua_State *L)
    {
        CCLOG("ai:lua error:%s", lua_tostring(L, -1));
#ifndef SERVER_VERIFYING
        
#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS) || (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        //CrashReport::log(CrashReport::CRLogLevel::Error, "ai_lua", lua_tostring(L, -1));
      ///  CrashReport::reportException(5, "LuaError", lua_tostring(L, -1), NULL);
#else
        luaL_error(L, lua_tostring(L, -1));
#endif
        
#else
        luaL_error(L, lua_tostring(L, -1));
#endif
        lua_pop(L, 1);
    }

} // namespace OnePiece
