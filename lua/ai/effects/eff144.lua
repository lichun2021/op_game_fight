-- 144 雷光火水暗毒系伤害免疫 - 火
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 144, param)
end

return eff