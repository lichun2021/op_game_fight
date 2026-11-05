-- 247 抵挡一次致命伤害并触发一个执行
local eff = {}

eff.can_trigger = function(character, param)
    return aieffectcaster.can_trigger_247(character, param)
end

eff.trigger = function(character)
    aieffectcaster.trigger_247(character)
end

return eff