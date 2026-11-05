-- 113 反弹攻击方血量
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 113, param)
end

return eff