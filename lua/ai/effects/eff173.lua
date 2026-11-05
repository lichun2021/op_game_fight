-- 173 受到幻象的伤害降低/提高
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 173, param)
end

return eff