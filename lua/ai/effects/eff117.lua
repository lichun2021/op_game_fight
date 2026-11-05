-- 117 物理伤害减免
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 117, param)
end

return eff