-- 146 雷光火水暗毒系伤害免疫 - 暗
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 146, param)
end

return eff