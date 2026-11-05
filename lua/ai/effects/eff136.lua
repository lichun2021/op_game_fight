-- 136 反弹攻击方魔法伤害
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 136, param)
end

return eff