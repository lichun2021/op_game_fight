-- 177 法术伤害吸血
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 177, param)
end

return eff