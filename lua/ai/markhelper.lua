-- markhelper

local theaicontext = aicontext
local theaimarkcaster = aimarkcaster

local function triggerby_hurt(target, skillid, skill_level, hurttype, hurtprop, hpdelta, hittype, source, hurtid)
    return theaicontext.hurtcalculate_triggermark(target, skillid, skill_level, hurttype, hurtprop, hpdelta, hittype, source, hurtid)
end

local function execute_effect(character, skillid, skilllevel, hurttype, hurtproperty, calltype, hpdelta, hittype, trigger)
    theaimarkcaster.check_excute_effect(character, skillid, skilllevel, hurttype, hurtproperty, calltype, hpdelta, hittype, trigger)
end

return {
    triggerby_hurt = triggerby_hurt,
    execute_effect = execute_effect
}