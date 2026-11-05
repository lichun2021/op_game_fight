-- 295 强制抓取，目标无法行动，但可受到移动执行影响

local get_innereffdata = require "effects/getinnereffdata"

local theactionhelper
local CD_MYID = g_cddata.CD_MYID
local AttrType = require "datadefs/attrtype"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    local target = self:get_target()
    if target:has_effect(298) then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)

    theactionhelper = theactionhelper or g_actionhelper
    local origin = self:get_origin()
    local tick = math.floor(changevalue)
    local actiontype = math.floor(effdata.targetattribute)
    theactionhelper.outofcontrolaction.enter(target, tick, actiontype, origin)
    if target:get_currentaction() == actiontype then
      if effdata.valuebase == 0 then
        target:add_tag("unactionable")
      end
    end

    local charid = target:get_attr(AttrType.ID)
    local myid = target:get_data(CD_MYID)
    local cuaction = target:get_currentaction()
    
    self:set_innerdata("actype", actiontype)
    self:set_innerdata("totaltick", tick)

  
    return true
end

local function on_exit(self)
  local character = self:get_target()
  local charid = character:get_attr(AttrType.ID)
  local myid = character:get_data(CD_MYID)
  local cuaction = character:get_currentaction()
  local origin = self:get_origin()
  
  theactionhelper.outofcontrolaction.enter(character, 0, cuaction, origin)
end

local function can_change(target, ca)
  
  local effs = target:get_innereffects(295)

  if not effs or #effs < 1 then return false end
  
  for i=1, #effs do
    local theeff = effs[i]
    local effdata = get_innereffdata(theeff)
    local params = effdata.params
    
    for j = 1, #params do
      local actype = params[j]
      
      if ca == actype then
        return true
      end
    end
    
  end
  
  return false
  
end



local function check_do(character, currentaction)
  
  local charid = character:get_attr(AttrType.ID)
  local myid = character:get_data(CD_MYID)
  
  local isefffound = 0 
  
  if character:has_effect(295) then
    
    local effs = character:get_innereffects(295)

    if not effs or #effs < 1 then return 0 end
    
    for i=1, #effs do
      local theeff = effs[i]
      local effdata = get_innereffdata(theeff)
      local params = effdata.params
      local isfind = false
      
      for j = 1, #params do
        local actype = params[j]
        
        if currentaction == actype then
          isfind = true
          break
        end
      end
      
      if isfind == false then -- shouldn't change action, then enter 295 managed action
        local intoaction = theeff:get_innerdata("actype")
        local tick = theeff:get_innerdata("totaltick")
        local origin = theeff:get_origin()
        
        theactionhelper.outofcontrolaction.enter(character, tick, intoaction, origin)
        
        isefffound = 1
      else
        isefffound = 0
      end        
      
    end
  end

  return isefffound
  
end


return {
    on_enter = on_enter,
    on_exit = on_exit,
    can_change = can_change,
    check_do = check_do
}