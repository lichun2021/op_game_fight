-- 303 当角色产生位移[移动执行（主动释放或被动触发或被敌方强制位移）、击飞、击倒]时触发执行

local get_innereffdata = require "effects/getinnereffdata"

local theexechelper
local CD_MYID = g_cddata.CD_MYID
local CD_LOCALTICK = g_cddata.CD_LOCALTICK
local AttrType = require "datadefs/attrtype"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    local target = self:get_target()
     
    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    
    aieffectcaster.start_innereffect_cd(target, self)
    
    return true
end
  
local function on_exit(self)
end

local function check_do_triggered_eff(target, movetype)
  
  local effs = target:get_innereffects(303)
  if target:get_remaineffectcd(303) > 0 or effs == nil or #effs < 1 then return true end

  local charid = target:get_attr(AttrType.ID)
  local myid = target:get_data(CD_MYID)
    
  for i=1,#effs do
      local aneff = effs[i]
    
      if not aneff:is_temp_invalid() then
        local innerexec = get_innereffdata(aneff)
        
        if innerexec ~= nil then
          local params = innerexec.params
          local execid = math.floor(innerexec.targetattribute)
          local trigger_cd = math.floor(innerexec.valuebase)
         
          for j = 1, #params do
            local acty = params[j]
            
            if acty == movetype then
              -- trigger effect
              if execid > 0 then
                
                local iftriggerd = aneff:has_innerdata("trigger") and aneff:get_innerdata("trigger") or 0
                local triggertick = aneff:has_innerdata("tick") and aneff:get_innerdata("tick") or 0
                local localtick = target:get_data(CD_LOCALTICK)
                
                if iftriggerd == 0 or (iftriggerd == 1 and localtick > triggertick + trigger_cd) then
                  theexechelper = theexechelper or g_exechelper
                  theexechelper.execute(execid, aneff:get_skill_id(), aneff:get_skill_level(), aneff:get_originsafe(), aneff:get_projectile(), target, 0, false, 0)
                  aneff:set_innerdata("trigger", 1)
                  aneff:set_innerdata("tick", target:get_data(CD_LOCALTICK))
                end
              end
              break
            end
          end
        end
        
      end
  end
      
end


return {
    on_enter = on_enter,
    on_exit = on_exit,
    check_do_triggered_eff = check_do_triggered_eff
}