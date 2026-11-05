-- 191 受到来自面朝方向的法术攻击伤害大幅降低
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 191, param)
end

return eff