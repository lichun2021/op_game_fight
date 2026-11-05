-- 149 受伤害加成
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 149, param)
end

return eff