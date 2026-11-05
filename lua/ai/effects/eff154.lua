-- 154 伤害上限阈值,单次受到伤害不会超过自身最大生命值的X%
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 154, param)
end

return eff