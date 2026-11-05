-- 273 吸收所有伤害类型的护盾，护盾的数值根据自身的某项属性

--[[
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 273, param)
end

eff.trigger = function(character, param)
    return aieffectcaster.trigger_273(character, param)
end

return eff
]]

local get_innereffdata = require "effects/getinnereffdata"
local attr = require "datadefs/attrtype"

local theaicontext = aicontext
local theexechelper

local function on_enter(self)
    if self:get_is_debuffed() then return false end
    
    local innerexec = get_innereffdata(self)
  
    local skilllevel = self:get_skill_level()
    local target = self:get_target()
    
    local ma = innerexec.changetype  
    local mb = innerexec.targetattribute
    local mblvup = innerexec.valuebase
  
    local mc = innerexec.changevalue
    local mclevelup = innerexec.levelup
    
    local mratioa = 0
    local mratiob = (mb + mblvup * skilllevel) / 100
    local mratioc = (mc + mclevelup * skilllevel) / 100
    
    if ma > 0 and ma < attr.ATTR_CNT then
        mratioa = target:get_attr(ma)
    end
    
    local changevalue = mratioa * mratiob + mratioc
    -- local changevalue = innerexec.changevalue + innerexec.levelup * skilllevel
    
    if innerexec.limitonoff == 0 then
        self:set_changevalue(changevalue)
    else
        local limitupper = (innerexec.limitvalupperbase + innerexec.limitvaluppergrowth * skilllevel) / 100
        local limitlower = (innerexec.limitvallowerbase + innerexec.limitvallowergrowth * skilllevel) / 100
        
        changevalue = math.min(changevalue, limitupper)
        changevalue = math.max(changevalue, limitlower)
        
        changevalue = math.floor(changevalue)
        
        self:set_changevalue(changevalue)
    end
    
    aieffectcaster.start_innereffect_cd(target, self)
      
    return true
end


local function on_exit(theeff)
    if theeff:get_innerdata("isbroken") == 1 then
        return
    end
    if aicontext.has_winlose() then
        -- 临时判断：过节点时清除护盾不会触发执行
        return
    end
    local target = theeff:get_targetsafe()
    if not target then
        return
    end
    local effdata = get_innereffdata(theeff)
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
            theexechelper.execute(anexecid, theeff:get_skill_id(), theeff:get_skill_level(), theeff:get_originsafe(), theeff:get_projectile(), target, 0, false, 0)
        end
    end
end

local function trigger_withparam(target, hpdelta, is_simu)
  local effs = target:get_innereffects(273)
  if not effs or #effs < 1 then return 0 end
  
  local repelnum = 0
  
  for i=1, #effs do
    local theeff = effs[i]
    if not theeff:is_temp_invalid() then
        local thechangevalue = theeff:get_changevalue()
        repelnum = repelnum + thechangevalue

        if repelnum <= hpdelta then
            if not is_simu then
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
                        theexechelper.execute(anexecid, theeff:get_skill_id(), theeff:get_skill_level(), theeff:get_originsafe(), theeff:get_projectile(), target, 0, false, 0)
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

local function trigger(target, hpdelta)
    return trigger_withparam(target, hpdelta, false)
end

return {
    on_enter = on_enter,
    on_exit = on_exit,
    trigger = trigger,
    trigger_withparam = trigger_withparam
}

