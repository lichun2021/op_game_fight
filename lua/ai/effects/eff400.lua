-- 400 改变目标状态的效果执行，只有在成功改变目标状态（即眩晕石化）后，才会显示绑定的UI特效

local get_innereffdata = require "effects/getinnereffdata"

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- or
--     local effs = character:get_innereffects(__)
--     if not effs or #effs < 1 then return end
-- end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()
    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    -- local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(changevalue)

    local parentcaster = self:get_parentcaster()
    if parentcaster and parentcaster:get_stateanimation_result(target) > 0 then
        return true
    end

    return false
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

return {
    -- check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}