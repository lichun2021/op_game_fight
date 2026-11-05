-- 180 伤害连结

local get_innereffdata = require "effects/getinnereffdata"

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- end

local theaicontext = aicontext

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    -- local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(changevalue)

    return true
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

local function is_sharehurtvalid(eff)
    local lastvalidaitick = eff:get_innerdata("ltick")
    return lastvalidaitick ~= theaicontext.get_currenttick()
end

local function get_firstvalideff(character)
    -- local theeffs = character:get_innereffects(180)
    -- if theeffs == nil or #theeffs < 1 then return end

    -- for i=1,#theeffs do
    --     local aneff = theeffs[i]
    --     if aneff and not aneff:is_temp_invalid() and is_sharehurtvalid(aneff) then
    --         return aneff
    --     end
    -- end

    -- return nil
    
    return aieffectcaster.eff180_get_firstvalid_eff(character)
end

local function trigger(character)
    -- local aneff = get_firstvalideff(character)
    -- if aneff then
    --     aneff:set_innerdata("ltick", theaicontext.get_currenttick())
    -- end
    aieffectcaster.eff180_trigger(character)
end

return {
    -- check_do = check_do,
    on_enter = on_enter,
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
    get_firstvalideff = get_firstvalideff,
    trigger = trigger
}