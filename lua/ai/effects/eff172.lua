-- 172 致命伤害转移到效果施法者

local theaieffectcaster = aieffectcaster

local function has_activated_eff(character, worldtick, source)
    return theaieffectcaster.eff172_has_activated_eff(character, worldtick, source)
end

local function check_do(character, param)
    return theaieffectcaster.check_exert_special_effect(character, 172, param)
end

local function get_valideff(character, tick, source)
    return theaieffectcaster.eff172_get_valid_eff(character, tick, source)
end

local function get_activatedeff(character, tick ,source)
    return theaieffectcaster.eff172_get_activated_eff(character, tick, source)
end

local function trigger(character, tick, source, valideff)
    theaieffectcaster.eff172_trigger(character, tick, source, valideff)
end

return {
    check_do = check_do,
    has_activated_eff = has_activated_eff,
    get_valideff = get_valideff,
    get_activatedeff = get_activatedeff,
    trigger = trigger
}