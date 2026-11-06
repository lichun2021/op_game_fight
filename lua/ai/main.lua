-- main

assert(g_platform == "SERVER", "local adapter requires Linux SERVER")
assert(type(g_op38_config_path) == "string" and string.sub(g_op38_config_path, 1, 1) == "/", "missing verified config path")
local theconfigpath = g_op38_config_path
local srcpath = assert(string.match(g_mainfilepath, "^(.*)/lua/ai/main%.lua$"), "unexpected main path")
package.path = srcpath .. "/lua/ai/?.lua"
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
