-- 310 将指定属性转换为另一项属性

local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()
    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    self:set_innerdata("exitclear", effdata.exitclear)

    local skilllevel = self:get_skill_level()
    local ratio = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_innerdata("ratio", ratio)
    ratio = ratio / 10000

    local changevalue = target:get_attr(effdata.valuebase) * ratio
    changevalue = math.floor(changevalue)
    self:set_changevalue(changevalue)

    local attridx = effdata.targetattribute
    self:set_innerdata("attridx", attridx)

    local realchangevalue = aieffectcaster.innereffect_apply_nonehpchange(target, attridx, changevalue)
    if realchangevalue == 0 then return false end

    self:set_innerdata("realchangeval", realchangevalue)

    if effdata.label ~= 0 then
        g_hitlabel.execute(effdata.label, realchangevalue, target)
    end

    return true
end

local function on_exit(self)
    if self:get_innerdata("exitclear") ~= 1 then
        return
    end

    local target = self:get_targetsafe()
    if not target then return end

    local realchangeval = self:get_innerdata("realchangeval")
    local attridx = self:get_innerdata("attridx")

    local attrval = target:get_attr(attridx) - realchangeval
    if attrval < 0 then attrval = 0 end
    target:set_attr(attridx, attrval)

end

return {
    on_enter = on_enter,
    on_exit = on_exit
}