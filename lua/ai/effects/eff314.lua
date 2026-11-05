-- 314 必杀技替换立绘

local get_innereffdata = require "effects/getinnereffdata"

local function pick_tagkey(pt)
    if pt == 1 then return "posterall"
    elseif pt == 2 then return "posternone"
    end
    return ""
end

local function increase_tagval(target, tagkey)
    local oldval = target:get_tag(tagkey)
    target:set_tag(tagkey, oldval + 1)
end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)

    -- 是否百分比 0-释放觉醒技时显示 1-普通/觉醒技都显示 2-始终不显示
    self:set_innerdata("pt", effdata.changetype) -- postertype

    -- 数值大小 对应替换立绘的索引，从1开始，表示第1个替换立绘；直接使用，不再除100
    self:set_changevalue(effdata.changevalue)

    -- if effdata.changetype == 1 then
    --     local oldval = target:get_tag("posterall")
    --     target:set_tag("posterall", oldval + 1)
    -- end

    increase_tagval(target, pick_tagkey(effdata.changetype))

    target:set_tag("posternum", effdata.changevalue)

    return true
end

local function on_exit(self)
    local target = self:get_targetsafe()
    if not target then return end
    -- if self:get_innerdata("pt") == 1 then
    --     local val = target:get_tag("posterall")
    --     val = val - 1
    --     if val < 0 then
    --         val = 0
    --     end
    --     target:set_tag("posterall", val)
    -- end

    local pt = self:get_innerdata("pt")
    if pt > 0 then
        local tagkey = pick_tagkey(pt)
        local val = target:get_tag(tagkey)
        val = val - 1
        if val < 0 then
            val = 0
        end
        target:set_tag(tagkey, val)
    end

    -- MARK: 简单处理效果消失的情况
    -- 现在一个该效果消失后，目标不会再使用替换立绘（即使还有未消失的同类效果），直到下一次再触发该效果
    target:set_tag("posternum", 0)
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}