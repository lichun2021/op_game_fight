-- 316 吸收治疗量的护盾

local get_innereffdata = require "effects/getinnereffdata"
local theexechelper

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()
    self:set_is_ignorehitlabel(true)

    local target = self:get_target()
    local source = self:get_origin()

    local innerexec = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = (innerexec.changevalue + innerexec.levelup * skilllevel) / 100
    if innerexec.changetype == 1 and innerexec.valuebase > 0 and innerexec.valuebase < 38 then
        -- 吸收量的基准数值为施法者自己
        local attrval = source:get_attr(innerexec.valuebase)
        changevalue = changevalue / 100 * attrval;
    end
    changevalue = math.floor(changevalue)
    
    if innerexec.limitonoff == 0 then
        self:set_changevalue(changevalue)
    else
        local limitupper = (innerexec.limitvalupperbase + innerexec.limitvaluppergrowth * skilllevel) / 100
        local limitlower = (innerexec.limitvallowerbase + innerexec.limitvallowergrowth * skilllevel) / 100
        
        changevalue = math.min(changevalue, limitupper)
        changevalue = math.max(changevalue, limitlower)
        
        self:set_changevalue(changevalue)
    end

    if changevalue <= 0 then
        return false
    end

    self:set_innerdata("rt", innerexec.type) -- repltype
    
    aieffectcaster.start_innereffect_cd(target, self)

    return true
end

local function on_exit(self)
    if self:get_innerdata("isbroken") == 1 then
        return
    end
    if aicontext.has_winlose() then
        -- 临时判断：过节点时清除护盾不会触发执行
        return
    end
    local target = self:get_targetsafe()
    if not target then
        return
    end
    local effdata = get_innereffdata(self)
    local params = effdata.params
    local exec_max_cnt = #params

    -- 参数后5位：效果时间到触发执行
    if exec_max_cnt < 6 then
        return
    end
    for j=6,exec_max_cnt do
        local anexecid = params[j]
        if anexecid > 0 then
            theexechelper = theexechelper or g_exechelper
            theexechelper.execute(anexecid, self:get_skill_id(), self:get_skill_level(), self:get_originsafe(), self:get_projectile(), target, 0, false, 0)
        end
    end
end

local function trigger(character, healdelta)
    local effs = character:get_innereffects(316)
    if not effs or #effs < 1 then return 0 end

    local delta = healdelta

    -- 1-加血时吸收 2-扣血时吸收
    local rt = 1
    if healdelta < 0 then
        rt = 2
        delta = -healdelta
    end

    local repelnum = 0
    for i=1, #effs do
        local theeff = effs[i]
        if not theeff:is_temp_invalid() and rt == theeff:get_innerdata("rt") then
            local thechangevalue = theeff:get_changevalue()
            repelnum = repelnum + thechangevalue

            if repelnum <= delta then
                    theeff:set_changevalue(0)
                    theeff:set_temp_invalid(true)
                    theeff:set_valid(false)
                    theeff:set_innerdata("isbroken", 1)
                    theeff:remove_ui_effect()
                    
                    -- fresh added
                    local effdata = get_innereffdata(theeff)
                    local params = effdata.params
                    local exec_max_cnt = #params
                    -- 参数前5位：破盾执行
                    if exec_max_cnt > 5 then
                        exec_max_cnt = 5
                    end
                    for j=1,exec_max_cnt do
                        local anexecid = params[j]
                        if anexecid > 0 then
                            theexechelper = theexechelper or g_exechelper
                            theexechelper.execute(anexecid, theeff:get_skill_id(), theeff:get_skill_level(), theeff:get_originsafe(), theeff:get_projectile(), character, 0, false, 0)
                        end
                    end
            else
                    local changevalue = repelnum - delta
                    theeff:set_changevalue(changevalue)
                    repelnum = delta
                    break
            end
        end

    end

    if healdelta < 0 then
        repelnum = -repelnum
    end
    
    return math.floor(repelnum)
end

return {
    on_enter = on_enter,
    on_exit = on_exit,
    trigger = trigger
}