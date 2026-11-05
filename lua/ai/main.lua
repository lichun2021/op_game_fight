-- main

local theplatform = g_platform
local theconfigpath
if theplatform == "IOS" or theplatform == "ANDROID" then
    g_mainfilepath = nil
    theconfigpath = "src/config_data/"
elseif theplatform == "SERVER" then
    theconfigpath = "/data/op_game_server/bin/export/client_data_cn/"
    local themainfilepath = g_mainfilepath
    local srcpos = string.find( themainfilepath, "/lua/ai/main.lua" )
    local srcpath = string.sub( themainfilepath, 1, srcpos )
    -- print(srcpath)
    local theluapath = srcpath .. "lua/"
    package.path = theluapath.."ai/?.lua"
elseif theplatform == "MACOS_SERVER" then
    theconfigpath = "data/config_data/"
    local theluapath = "../fight_server/lua/"
    local libs = {
        theluapath.."ai/",
        theluapath.."luasocket/",
        theluapath.."mobdebug/?.lua"
    }
    package.path = table.concat(libs, "?.lua;")
    require("mobdebug").start()
elseif theplatform == "WIN32_SERVER" then
    theconfigpath = "data/config_data/"
    local theluapath = "../fight_server/lua/"
    local libs = {
        theluapath.."ai/",
        theluapath.."luasocket/",
        theluapath.."mobdebug/?.lua"
    }
    package.path = table.concat(libs, "?.lua;")
    require("mobdebug").start()
else
    local themainfilepath = g_mainfilepath
    print(themainfilepath)
    local srcpos = string.find( themainfilepath, "/lua/ai/main.luac" )
    if not srcpos then
        srcpos = string.find( themainfilepath, "/lua/ai/main.lua" )
    end
    -- print(srcpos)
    local srcpath = string.sub( themainfilepath, 1, srcpos )
    print(srcpath)
    theconfigpath = srcpath .. "config_data/"
    local theluapath = srcpath .. "lua/"
    local libs = {
        theluapath.."ai/",
        theluapath.."luasocket/",
        theluapath.."mobdebug/?.lua"
    }
    package.path = table.concat(libs, "?.lua;")
    require("mobdebug").start()
end
-- print(package.path)

g_hostconfig = require "hostscriptconfig"
g_aihelper = require "aicontexthelper"
g_datahelper = require "configdatahelper"
g_datahelper.preload_allconfigdata(theconfigpath)

g_cddata = require "datadefs/characterdataopcode"
g_actionhelper = require "actionhelper"
g_hurthelper = require "hurthelper"
g_targethelper = require "targethelper"
g_effecthelper = require "effecthelper"
g_characterhelper = require "characterhelper"
g_summonhelper = require "summonhelper"
g_hitlabel = require "hitlabelhelper"
g_markhelper = require "markhelper"
g_exechelper = require "exechelper"
g_skillhelper = require "skillhelper"
g_stathelper = require "stathelper"
g_cachehelper = require "datacachehelper"
g_projectilehelper = require "projectilehelper"
g_uiunithelper = require "uiunithelper"
