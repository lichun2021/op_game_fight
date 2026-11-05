-- 181 持续时间内若连续X次攻击不是暴击，则下一次攻击必定命中且暴击
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 181, param)
end

rethelper.try_trigger = function(character)
    return aieffectcaster.try_trigger_181(character)
end

rethelper.trigger = function(character, param)
    aieffectcaster.trigger_181(character, param)
end

return rethelper