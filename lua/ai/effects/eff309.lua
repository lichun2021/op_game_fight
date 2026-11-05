-- 309 释放指定类型的技能时触发执行

local get_innereffdata = require "effects/getinnereffdata"
local ActionAttType = require "actions/actionatttype"
local theexechelper
local theskillhelper

local function check_do(character)
    local effs = character:get_innereffects(309)
    if not effs or #effs < 1 then return end

    local skillid = character:get_actionatt(ActionAttType.SKILL_ID)
    theskillhelper = theskillhelper or g_skillhelper
    local sktype = theskillhelper.get_skilltype(character, skillid)

    -- if character:has_tag("ultraskill") then sktype = 1 end

    theexechelper = theexechelper or g_exechelper

    for i=1,#effs do
        local aneff = effs[i]
        local changeval = aneff:get_changevalue()
        if changeval == sktype then
            local effdata = get_innereffdata(aneff)
            local casttype = math.floor(effdata.valuebase)
            local caster = character
            if casttype == 1 then caster = aneff:get_origin() end
            for j=1,#effdata.params do 
                local anexecid = math.floor(effdata.params[j])
                theexechelper.execute(anexecid, aneff:get_skill_id(), aneff:get_skill_level(), caster, nil, character, 0, false, 0)
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

    -- 0-普通技能 1-大招
    self:set_changevalue(effdata.changevalue)

    return true
end

return {
    check_do = check_do,
    on_enter = on_enter
}