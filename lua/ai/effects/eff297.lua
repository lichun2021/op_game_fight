-- 297 不能被指定阵营的单体目标类型选中
local get_innereffdata = require "effects/getinnereffdata"

local function is_findable(character, searcher)
    local effs = character:get_innereffects(297)
    if not effs or #effs < 1 then return true end

    local t1 = character:get_teamside()
    local t2 = searcher:get_teamside()

    for i=1,#effs do
        local aneff = effs[i]
        local changevalue = aneff:get_changevalue()
        -- 0-己方 1-敌方 2-全体
        if changevalue == 0 or changevalue == 2 then
            if t1 == t2 then return false end
        elseif changevalue == 1 or changevalue == 2 then
            if t1 ~= t2 then return false end
        end
    end
    return true
end

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    local changevalue = effdata.targetattribute
    self:set_changevalue(changevalue)

    return true
end

local function on_exit(self)
end

return {
    is_findable = is_findable,
    on_enter = on_enter,
    on_exit = on_exit
}