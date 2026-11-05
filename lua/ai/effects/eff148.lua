-- 148 受治疗加成
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 148, param)
end

return eff