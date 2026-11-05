-- 286 恢复上一次直接攻击造成的一定百分比的伤害(不包括效果执行的持续伤害)
local get_innereffdata = require "effects/getinnereffdata"
local heal_target = require "hurtfuncs/healtarget"

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()

    -- self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    local lastonetimehurt = target:get_tag("lastonetimehurt")
    if lastonetimehurt > 0 then
        local effdata = get_innereffdata(self)
        local skilllevel = self:get_skill_level()
        local changevalue = effdata.changevalue + effdata.levelup * skilllevel
        changevalue = changevalue / 100
        -- self:set_changevalue(changevalue)
        local healval = lastonetimehurt * changevalue / 100
        if healval > 0 then
            local source = self:get_origin()
            local skillid = self:get_skill_id()
            heal_target(source, target, healval, 100002, skillid, false, 0)
            self:set_changevalue(healval)
        end
    end

    return true
end

local function on_exit(self)
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}