-- uiunithelper

local unithelper = {}

local unit_height = 75
local is_needadjust = true

function unithelper.adjust_shadowscale_by_deltay(oldscale, deltay)
    -- print(string.format("deltay:%f", deltay))
    local adjustscale = oldscale * (unit_height - deltay) / unit_height * 0.5
    if adjustscale < 1 then
        adjustscale = 1
    end

    local adjustopacity = 255 * unit_height / (unit_height - deltay)
    adjustopacity = math.floor( adjustopacity )
    -- print(string.format("adjustopacity:%d", adjustopacity))
    -- print(string.format( "old:%f,new:%f", oldscale, adjustscale ))
    return is_needadjust, adjustscale, adjustopacity
end

-- side: 1-our(左边入场) 2-enemy(右边入场)
function unithelper.get_armorbar_config(side)
    -- 护盾icon, 护盾条, icon位置, 护盾条位置
    local path = {"res/effect/hurtdis/Icon_shield.png", "res/effect/hurtdis/Naval-battle_accounts_progress-Shields.png"}
    local iconpos = {-50, 20}
    if side == 2 then
        iconpos[1] = 75
    end
    local barpos = {0, 20}
    return path[1], path[2], iconpos[1], iconpos[2], barpos[1], barpos[2]
end

return unithelper