-- 302 角色处于丢失状态时触发执行

local get_innereffdata = require "effects/getinnereffdata"

local theexechelper

local function check_do(character)
    local effs = character:get_innereffects(302)
    if not effs or #effs < 1 then 
        return 
    end

    for i=1,#effs do
        local aneff = effs[i]
        local effdata = get_innereffdata(aneff)
        for j=1,#effdata.params do
            local anexecid = effdata.params[j]
            if anexecid > 0 then
                theexechelper = theexechelper or g_exechelper
                theexechelper.execute(anexecid, aneff:get_skill_id(), aneff:get_skill_level(), aneff:get_originsafe(), aneff:get_projectile(), character, 0, false, 0)
            end
        end
    end
end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)

    return true
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

return {
    check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}