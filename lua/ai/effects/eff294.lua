-- 294 不能使用含有移动执行的技能
local AttrType = require "datadefs/attrtype"
local get_innereffdata = require "effects/getinnereffdata"
local exechelper = g_exechelper

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    -- local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(changevalue)

    return true
end

local function on_exit(self)

end

local function is_exec_has_movecaster(execid)
    if exechelper.is_moveexec(execid) then return true end

    if exechelper.is_hurtexec(execid) then
        local hurtdata = g_datahelper.get_hurtdata(math.floor(execid))
        local hitexecs = hurtdata.hitexecs
        for i=1,#hitexecs do
            local anexecid = hitexecs[i]
            if anexecid > 0 and exechelper.is_moveexec(anexecid) then return true end
        end
    end

    return false
end

local function is_execs_has_movecaster(execs)
    for i=1,#execs,4 do
        local execid = execs[i]
        if execid > 0 and is_exec_has_movecaster(execid) then
            return true
        end
    end

    return false
end

local function is_skillcastconditon_match_params(condition, params)
    for i=1,#params do
        if condition == params[i] then return true end
    end
    return false
end

local skill_info_cache

local function is_skill_has_movecaster(character, skillid)
    if skillid <= 0 then return false end
    if not skill_info_cache then
        skill_info_cache = {}
    end
    if skill_info_cache[skillid] then
        return skill_info_cache[skillid]
    end
    local skilldata = g_datahelper.get_skilldata(skillid)
    if not skilldata then return false end
    local skillcastcond = skilldata.conf[10]
    if skillcastcond ~= 0 then
        local effs = character:get_innereffects(294)
        for i=1, #effs do
            local aneff = effs[i]
            local effdata = get_innereffdata(aneff)
            if is_skillcastconditon_match_params(skillcastcond, effdata.params) then
                local ret = is_execs_has_movecaster(skilldata.exe)
                skill_info_cache[skillid] = ret
                return ret
            end
        end
    else
        local ret = is_execs_has_movecaster(skilldata.exe)
        skill_info_cache[skillid] = ret
        return ret
    end
    
    skill_info_cache[skillid] = false
    return false
end

local function is_skillidblocked(character, skillid)
    if character:has_effect(294) ~= true then return false end

    if is_skill_has_movecaster(character, skillid) then return true end
    return false
end

local function is_skillblocked(character, skillidindex)
    if character:has_effect(294) ~= true then return false end
    local characterdata = g_datahelper.get_characterdata(character:get_attr(AttrType.ID))
    local skillid = characterdata.sk[skillidindex + 1]
    assert(skillid ~= nil, "skillid is nil!")
    return is_skillidblocked(character, skillid)
end

return {
    on_enter = on_enter,
    on_exit = on_exit,
    is_skillblocked = is_skillblocked,
    is_skillidblocked = is_skillidblocked
}