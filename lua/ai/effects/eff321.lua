-- 321 驱散指定的伤害执行

local get_innereffdata = require "effects/getinnereffdata"
local CasterType = require("datadefs/castertype")

local effecthelper

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)

    local target = self:get_target()

    effecthelper = effecthelper or g_effecthelper

    local params = effdata.params
    for i=1,#params do
        local casterid = params[i]
        if casterid > 0 and effecthelper.get_eff(322).check_do(target, casterid) == 0 then
            aiactioncontroller.invalidatecaster(target, CasterType.eHurtExec, casterid)
            aiactioncontroller.addtoremovecaster(target, CasterType.eHurtExec, casterid)
        end
    end

    return false
end

return {
    on_enter = on_enter
}