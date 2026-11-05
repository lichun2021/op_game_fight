-- 66 敌我之中战力最高的目标
local get_highestcombatpowercharacter = require "targetfuncs/gethighestcombatpowercharacter"

return function(params)
    local characterlist = g_targethelper.pick_filteredalllist(params)
    return get_highestcombatpowercharacter(characterlist, params)
end
