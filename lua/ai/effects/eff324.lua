-- 324 判断目标的技能等级
local get_innereffdata = require "effects/getinnereffdata"

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- or
--     local effs = character:get_innereffects(__)
--     if not effs or #effs < 1 then return end
-- end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)

    local skid = effdata.targetattribute
    local sklv = effdata.changevalue / 100
    local comptype = effdata.valuebase

    local ismatch = false
    local lv = target:get_skilllevel(skid)
    if lv > 0 then
        -- 0等于 1大于 2小于 3大于等于 4小于等于
        if comptype == 0 then
            if lv == sklv then
                ismatch = true
            end
        elseif comptype == 1 then
            if lv > sklv then
                ismatch = true
            end
        elseif comptype == 2 then
            if lv < sklv then
                ismatch = true
            end
        elseif comptype == 3 then
            if lv >= sklv then
                ismatch = true
            end
        elseif comptype == 4 then
            if lv <= sklv then
                ismatch = true
            end
        end
    end

    if ismatch then
        local params = effdata.params
        local skillid = self:get_skill_id()
        local skilllevel = self:get_skill_level()
        local source = self:get_origin()
        for i=1,#params do
            local casterid = params[i]
            if casterid > 0 then
                g_exechelper.execute(casterid, skillid, skilllevel, source, nil, target, 0, false, 0)
            end
        end
    end

    return true
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

return {
    -- check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}