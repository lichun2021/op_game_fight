-- 301 清空288统计的伤害

local get_innereffdata = require "effects/getinnereffdata"

local theeffecthelper

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local mid = math.floor(effdata.targetattribute)

    theeffecthelper = theeffecthelper or g_effecthelper
    theeffecthelper.get_eff(288).on_clear_data(target, mid)

    return true
end


return {
    on_enter = on_enter
}