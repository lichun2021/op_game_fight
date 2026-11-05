-- 48 source能造成最多伤害的敌方

local simu_calc_hurt = require "hurtfuncs/simucalchurt" 
local function get_charactercanhurtmax(characterlist, params)
    if params.hurt_id == nil or params.hurt_id <= 0 then return nil end
    local themaxhurt = -1
    local thecharacter = nil
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local a_hurt = simu_calc_hurt(params, a_character)
            if a_hurt > themaxhurt then 
                themaxhurt = a_hurt
                thecharacter = a_character
            end
        end
    end
    if thecharacter ~= nil then return { thecharacter } else return nil end
end

return function(params)
    return get_charactercanhurtmax(g_targethelper.pick_filteredrivallist(params), params)
end