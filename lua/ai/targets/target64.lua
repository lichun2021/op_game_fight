-- 64 敌军之中战力最高的目标

local get_highestcombatpowercharacter = require "targetfuncs/gethighestcombatpowercharacter"

return function(params)
    local characterlist = g_targethelper.pick_filteredrivallist(params)
    return get_highestcombatpowercharacter(characterlist, params)
end
