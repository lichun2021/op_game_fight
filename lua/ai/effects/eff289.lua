-- 289 释放统计的伤害
-- 查找本效果施法者身上的效果288统计的伤害值，将匹配的288的伤害，作为物理伤害，作用在统计伤害最高的单位身上

local get_innereffdata = require "effects/getinnereffdata"

local theeff288helper

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local origin = self:get_origin()

    local skilllevel = self:get_skill_level()
    local effdata = get_innereffdata(self)
    local mid = math.floor(effdata.targetattribute)
    local hurttype = effdata.hurttype - 1

    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    local ratio = changevalue / 10000
    local usetype = effdata.valuebase
    local low = effdata.limitvallowerbase + effdata.limitvallowergrowth * skilllevel
    -- low = math.floor(low / 100)
    local high = effdata.limitvalupperbase + effdata.limitvaluppergrowth * skilllevel
    -- high = math.floor(high / 100)

    theeff288helper = theeff288helper or g_effecthelper.get_eff(288)

    local skillid = self:get_skill_id()
    local parentcaster = self:get_parentcaster()
    local triggercasterid = parentcaster:get_effectid()
    theeff288helper.on_trigger(origin, mid, hurttype, ratio, usetype, low, high, skillid, skilllevel, triggercasterid, self, effdata.label, target)

    return true
end

local function on_exit(self)
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}