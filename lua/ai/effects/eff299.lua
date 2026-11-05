-- 299 提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）

local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local skillid = self:get_skill_id()
    local skilllevel = self:get_skill_level()
    local changevalue = (effdata.changevalue + effdata.levelup * skilllevel) / 100
    changevalue = math.floor(changevalue)
    self:set_changevalue(changevalue)

    local theskillid = effdata.targetattribute
    if target:has_skill(theskillid) then
        local oldskilllevel = target:get_skilllevel(theskillid)
        if not target:has_skillinfobackup(theskillid) then target:backup_skillinfo(theskillid, oldskilllevel) end
        local theskilllevel = oldskilllevel + changevalue
        if theskilllevel < 0 then theskilllevel = 0 end
        target:update_skilllevel(theskillid, theskilllevel)
        self:set_innerdata("rc", theskilllevel - oldskilllevel)

        local eff299cnt = 0
        if target:has_tag("eff299") then eff299cnt = target:get_tag("eff299") end
        target:set_tag("eff299", eff299cnt + 1)

        if changevalue >= effdata.limitvallowerbase / 100 then
            local execid = math.floor(effdata.limitvallowergrowth)
            g_exechelper.execute(execid, skillid, skilllevel, target, nil, target, 0, false, 0)
        end
    else
        return false
    end

    return true
end

local function on_exit(self)
    local effdata = get_innereffdata(self)
    -- if effdata.exitclear ~= 1 then return end
    local target = self:get_targetsafe()
    if not target then return end
    local realchangevalue = self:get_innerdata("rc")
    if realchangevalue == 0 then return end

    local theskillid = effdata.targetattribute
    if not target:has_skill(theskillid) then return end

    if target:has_tag("eff299") and target:get_tag("eff299") <= 1 and target:has_skillinfobackup(theskillid) then
        local oldskilllevel = target:get_skillinfobackup(theskillid)
        target:update_skilllevel(theskillid, oldskilllevel)
    else
        local theskilllevel = target:get_skilllevel(theskillid) - realchangevalue
        if theskilllevel < 0 then theskilllevel = 0 end
        target:update_skilllevel(theskillid, theskilllevel)
    end
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}