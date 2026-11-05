-- 271 承受伤害小于一定阈值伤害不生效，且触发执行id
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 271, param)
end

eff.trigger = function(target)
    aieffectcaster.trigger_271(target)
end

return eff