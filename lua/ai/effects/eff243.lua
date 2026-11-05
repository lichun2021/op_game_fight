-- 243 增加目标造成的所有伤害(不含加血)
local eff = {}
eff.check_do = function (character, param)
    return aieffectcaster.check_exert_special_effect(character, 243, param)
end
return eff