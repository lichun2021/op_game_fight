-- 141 只能被高于阈值的伤害所伤害
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 141, param)
end

return eff