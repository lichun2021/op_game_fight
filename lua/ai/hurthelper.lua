--
local hurthelper = {}

hurthelper.calcinithurt = require "hurtfuncs/calcinithurt"  --ok
hurthelper.getbasechp = require "hurtfuncs/getbasechp"  --ok
hurthelper.checkandexecutetalent = require "hurtfuncs/checkandexecutetalent"  --ok
hurthelper.characterdeadbyhurt = require "hurtfuncs/characterdeadbyhurt"  --ok
hurthelper.reboundandabsorb = require "hurtfuncs/reboundandabsorb"  --ok
hurthelper.getoriginalavoidancehurt = require "hurtfuncs/getoriginalavoidancehurt"  --ok
-- hurthelper.simu_calc_hurt = require "simucalchurt"
hurthelper.hurttarget = require "hurtfuncs/hurttarget"  --ok
hurthelper.healtarget = require "hurtfuncs/healtarget"  --ok
hurthelper.roundtable = require "hurtfuncs/roundtable"  --ok
hurthelper.executehurt = require "hurtfuncs/executehurt"  --ok

return hurthelper
