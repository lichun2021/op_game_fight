-- 285 判断觉醒等级触发执行

local get_innereffdata = require "effects/getinnereffdata"
local theaieffectcaster = aieffectcaster
local theexechelper

local eff = {}

eff.on_enter = function(self)

    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    local effdata = get_innereffdata(self)

    local target = self:get_target()
    local source = self:get_origin()
    local compare_target = target
    -- 0: compare target
    -- 15: compare origin
    local compare_type = math.floor(effdata.targetattribute)
    if compare_type == 15 then
        compare_target = source
    end

    local character_jxlv = compare_target:get_val("juexinglevel")
    local execid = math.floor(effdata.params[character_jxlv + 1])
    theexechelper = theexechelper or g_exechelper
    theexechelper.execute(execid, self:get_skill_id(), self:get_skill_level(), source, self:get_projectile(), target, 0, false, 0)

    self:set_is_ignorehitlabel(true)
    theaieffectcaster.start_innereffect_cd(target, self)

    return true
end

eff.on_exit = function(self)
end

return eff