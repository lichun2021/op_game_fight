-- 192 受到来自面朝方向的所有攻击伤害大幅降低
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 192, param)
end

return eff