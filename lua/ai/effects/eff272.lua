-- 272 受到致命伤害（本次伤害会造成死亡）时，保留1滴血量不死，触发一个执行
local eff = {}
eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 272, param)
end

eff.can_trigger = function(character, param)
    return aieffectcaster.can_trigger_272(character, param)
end

eff.trigger = function(character)
    aieffectcaster.trigger_272(character)
end

eff.can_clean = function(character)
    return aieffectcaster.can_clean_272(character)
end

return eff