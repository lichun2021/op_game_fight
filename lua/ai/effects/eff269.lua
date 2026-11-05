-- 269 根据目标当前血量百分比造成伤害
local eff = {}

eff.get_extrahurt = function(character, hpdelta)
    return aieffectcaster.get_eff_269_extrahurt(character, hpdelta)
end

eff.get_hitlabel = function(character)
    return aieffectcaster.get_eff_269_hitlabel(character)
end

return eff