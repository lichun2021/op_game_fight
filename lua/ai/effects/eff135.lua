-- 135 反弹攻击方物理伤害
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 135, param)
end

return eff