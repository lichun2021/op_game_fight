-- 226 吸收伤害次数的所有类型攻击护盾 物理 法术 纯粹
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 226, param)
end

eff.change = function(character, param)
    aieffectcaster.change_special_effect(character, 226, param)
end

return eff