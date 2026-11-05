-- 138 物理伤害吸收护盾
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 138, param)
end

return eff