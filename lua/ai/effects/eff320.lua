-- 320 吸收指定属性类型伤害的护盾

local get_innereffdata = require "effects/getinnereffdata"
local calcchangevalue = require "effects/calcchangevalue"

local HurtTypeKeys = {}
HurtTypeKeys[-1] = "t3" -- 纯粹伤害:效果执行的伤害属性填0,计算伤害时会将类型-1,使其与伤害执行一致
HurtTypeKeys[0] = "t0" -- 物理
HurtTypeKeys[1] = "t1" -- 法术

local HurtPropKeys = {}
for i=0,6 do
    HurtPropKeys[i] = string.format( "p%d",i )
end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    -- 百分比读取的属性是施法者的属性
    local changevalue = calcchangevalue(effdata, skilllevel, self:get_origin())
    if changevalue <= 0 then return false end
    self:set_changevalue(changevalue)

    local params = effdata.params
    -- 参数列表前3个字段:支持的伤害类型 0-无效 1-物理 2-法术 3-纯粹 (治疗不支持)
    for i=1,3 do
        local t = params[i]
        if t == 1 then 
            t = 0
        elseif t == 2 then
            t = 1
        elseif t == 3 then
            t = -1
        else
            t = -2
        end
        local key = HurtTypeKeys[t]
        if key then
            self:set_innerdata(key, 1)
        end
    end

    -- 参数列表4-10,共7个字段:支持的伤害属性 0-无效 1-无属性 ~ 7-毒
    for i=4,10 do
        local p = params[i] - 1
        local key = HurtPropKeys[p]
        if key then
            self:set_innerdata(key, 1)
        end
    end

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

    -- 参数列表16-20个字段:时间到触发执行
    for j=16,#params do
        local anexecid = params[j]
        if anexecid > 0 then
            g_exechelper.execute(anexecid, self:get_skill_id(), self:get_skill_level(), self:get_originsafe(), self:get_projectile(), target, 0, false, 0)
        end
    end
end

local function trigger(target, hurttype, hurtprop, hpdelta, is_simu)
    local effs = target:get_innereffects(320)
    if not effs or #effs < 1 then return 0 end
    
    local repelnum = 0
    local tkey = HurtTypeKeys[hurttype]
    local pkey = HurtPropKeys[hurtprop]

    if not tkey then return 0 end
    if not pkey then return 0 end
    
    for i=1, #effs do
        local theeff = effs[i]
        if not theeff:is_temp_invalid() and theeff:get_innerdata(tkey) == 1 and theeff:get_innerdata(pkey) == 1 then
            local thechangevalue = theeff:get_changevalue()
            repelnum = repelnum + thechangevalue

            if repelnum <= hpdelta then
                if not is_simu then
                    theeff:set_changevalue(0)
                    theeff:set_temp_invalid(true)
                    theeff:set_valid(false)
                    theeff:set_innerdata("isbroken", 1)
                    theeff:remove_ui_effect()
                    
                    local effdata = get_innereffdata(theeff)
                    local params = effdata.params
                    local exec_max_cnt = #params
                    -- 参数列表11-15,共5个字段:破盾执行
                    if exec_max_cnt > 15 then
                        exec_max_cnt = 15
                    end
                    for j=11,exec_max_cnt do
                        local anexecid = params[j]
                        if anexecid > 0 then
                            g_exechelper.execute(anexecid, theeff:get_skill_id(), theeff:get_skill_level(), theeff:get_originsafe(), theeff:get_projectile(), target, 0, false, 0)
                        end
                    end
                end
                
            else
                if not is_simu then
                    local changevalue = repelnum - hpdelta
                    theeff:set_changevalue(changevalue)
                end
                repelnum = hpdelta
                break
            end
        end

    end
    
    return math.floor(repelnum)
end

return {
    -- check_do = check_do,
    on_enter = on_enter,
    on_exit = on_exit,
    trigger = trigger
}