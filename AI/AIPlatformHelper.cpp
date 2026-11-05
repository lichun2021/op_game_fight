//
//  AIPlatformHelper.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIPlatformHelper.hpp"
#ifdef __unix__
#include <unistd.h>
#endif

#include "cocos2d.h"

namespace OnePiece {
    
    std::string AIPlatformHelper::getPlatformName()
    {
#ifdef SERVER_VERIFYING
#ifdef WIN32
        return "WIN32_SERVER";
#elif defined __APPLE__
        return "MACOS_SERVER";
#else
        return "SERVER";
#endif

        
#elif defined CC_TARGET_PLATFORM
        if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
        {
            return "MACOS";
        }
        else if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
        {
            return "IOS";
        }
        else if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
        {
            return "ANDROID";
        }
        else if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)
        {
            return "WIN32";
        }
#endif
        
        return "GENERIC";
    }
    
    std::string AIPlatformHelper::getExecutablePath()
    {
#ifdef __unix__
        char buf[256];
        readlink("/proc/self/exe", buf, 256);
        std::string execPath = buf;
        auto lastSlashPos = execPath.find_last_of("/");
        if (lastSlashPos == std::string::npos)
            return "./";
        
        return execPath.substr(0, lastSlashPos);
#endif
        return "./";
    }
    
} // namespace OnePiece

