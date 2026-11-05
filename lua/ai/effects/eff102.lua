-- 102 回光返照
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 102, param)
end

return eff