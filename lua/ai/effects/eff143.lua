-- 143 雷光火水暗毒系伤害免疫 - 光
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 143, param)
end

return eff