-- 157 吸收伤害次数的物理攻击护盾
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 157, param)
end

eff.change = function(character, param)
    aieffectcaster.change_special_effect(character, 157, param)
end

return eff