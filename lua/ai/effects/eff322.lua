-- 322 保护指定的伤害执行不被驱散

local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(effdata.changevalue)

    local params = effdata.params
    for i=1,#params do
        local casterid = params[i]
        if casterid > 0 then
            self:set_innerdata(tostring(casterid), 1)
        end
    end

    return true
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

-- 判断目标的指定伤害执行是否被保护 0-没有保护 1-有保护
local function check_do(character, casterid)
    local effs = character:get_innereffects(322)
    if not effs or #effs < 1 then return 0 end

    local idstr = tostring(casterid)
    for i=1,#effs do
        local eff = effs[i]
        if not eff:is_temp_invalid() and eff:get_innerdata(idstr) == 1 then
            return 1
        end
    end

    return 0
end

return {
    check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}