-- 304 被认为是幻象或英雄

local get_innereffdata = require "effects/getinnereffdata"

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = math.floor(effdata.changevalue)
    self:set_changevalue(changevalue)

    -- 0-不被认为是幻象
    -- 1-被认为是幻象
    if changevalue == 0 then
        if target:is_summon() then
            local tag = "fakesummoneff_notsummon"
            local tagval = target:get_tag(tag) + 1
            target:set_tag(tag, tagval)
        end
    elseif changevalue == 1 then
        if not target:is_summon() then
            local tag = "fakesummoneff_summon"
            local tagval = target:get_tag(tag) + 1
            target:set_tag(tag, tagval)
        end
    end

    return true
end

local function on_exit(self)
    local target = self:get_targetsafe()
    if not target then
        return
    end

    local changevalue = self:get_changevalue()
    -- 0-不被认为是幻象
    -- 1-被认为是幻象
    if changevalue == 0 then
        if target:is_summon() then
            local tag = "fakesummoneff_notsummon"
            local tagval = target:get_tag(tag) - 1
            if tagval <= 0 then
                target:erase_tag(tag)
            else
                target:set_tag(tag, tagval)
            end
        end
    elseif changevalue == 1 then
        if not target:is_summon() then
            local tag = "fakesummoneff_summon"
            local tagval = target:get_tag(tag) - 1
            if tagval <= 0 then
                target:erase_tag(tag)
            else
                target:set_tag(tag, tagval)
            end
        end
    end
end

-- local function on_scheduletick(self)
-- end

return {
    -- check_do = check_do,
    on_enter = on_enter,
    on_exit = on_exit
    -- on_scheduletick = on_scheduletick
}