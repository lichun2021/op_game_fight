//
//  AIPlatformHelper.cpp
//  op_game
//
//  Created by h7ing on 07/08/2017.
//
//

#include "AIPlatformHelper.hpp"
#include <stdexcept>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#include <limits.h>
#include <cstdlib>
#endif
#ifdef __unix__
#include <unistd.h>
#endif

#include "cocos2d.h"

namespace OnePiece {
    
    std::string AIPlatformHelper::getPlatformName()
    {
#ifdef SERVER_VERIFYING
// Headless verification uses server-only Lua branches on every host.
        return "SERVER";

        
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
#ifdef __APPLE__
        char buffer[PATH_MAX], resolved[PATH_MAX]; uint32_t size=sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) != 0 || !realpath(buffer, resolved))
            throw std::runtime_error("executable path unavailable");
        std::string path(resolved);
        return path.substr(0, path.find_last_of("/"));
#elif defined __unix__
        char buf[4096];
        ssize_t n = readlink("/proc/self/exe", buf, sizeof(buf)-1);
        if (n <= 0 || n >= ssize_t(sizeof(buf)-1)) throw std::runtime_error("executable path unavailable");
        std::string execPath(buf, size_t(n));
        auto lastSlashPos = execPath.find_last_of("/");
        if (lastSlashPos == std::string::npos)
            return "./";
        
        return execPath.substr(0, lastSlashPos);
#endif
        return "./";
    }
    
} // namespace OnePiece

