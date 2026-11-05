-- 118 法术伤害减免
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 118, param)
end

return eff