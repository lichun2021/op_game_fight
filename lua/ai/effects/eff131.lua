-- 131 降低治疗效果
local eff = {}

eff.check_do = function(character, originvalue)
    return aieffectcaster.check_exert_special_effect(character, 131, originvalue)
end

return eff