-- 147 雷光火水暗毒系伤害免疫 - 毒
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 147, param)
end

return eff