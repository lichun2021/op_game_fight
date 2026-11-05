-- 139 法术伤害吸收护盾
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 139, param)
end

return eff