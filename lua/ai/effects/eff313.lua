-- 313 搜索最近目标时优先寻找非幻象

local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    return true
end

return {
    on_enter = on_enter
}