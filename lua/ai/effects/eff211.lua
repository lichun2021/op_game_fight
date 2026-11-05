-- 211 完全抵挡下一次攻击并反伤（反伤数值/百分比需要可配置，若填0则不反伤），该效果触发后移除

local check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 211, param)
end

local trigger = function(target, reboundtarget, skillid, skill_level)
    aieffectcaster.trigger_211(target, reboundtarget, skillid, skill_level)
end

local clean = function(target)
    aieffectcaster.zero_special_effect(target, 211)
end

return {
    check_do = check_do,
    trigger = trigger,
    clean = clean
}