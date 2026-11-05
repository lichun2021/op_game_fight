-- 308 根据指定技能的等级及效果数值区间，决定是否触发效果执行

local get_innereffdata = require "effects/getinnereffdata"
local theexechelper

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()
    local origin = self:get_origin()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)

    local paramskillid = math.floor(effdata.targetattribute)
    local lowerthreshold = math.floor(effdata.limitvallowerbase / 100)
    local upperthreshold = math.floor(effdata.limitvalupperbase / 100)
    
    
    local skilllist = origin:get_skill_list()
    if #skilllist < 1 then return false end
    for i=1, #skilllist do
      local skill = skilllist[i]
      if paramskillid == skill[1] then
        local flag = false
        local skilllevel = skill[2]
        local changevalue = (effdata.changevalue + effdata.levelup * skilllevel) / 100
        changevalue = math.floor(changevalue)
    
        if upperthreshold > 0 then
         if upperthreshold > lowerthreshold and (changevalue >= lowerthreshold and changevalue < upperthreshold) then
          flag = true 
         end
        else
          if changevalue >= lowerthreshold then
            flag = true
          end
        end
        
        if flag == true then
          local anexecid = math.floor(effdata.limitvallowergrowth)
          if anexecid > 0 then
              theexechelper = theexechelper or g_exechelper
              theexechelper.execute(anexecid, self:get_skill_id(), self:get_skill_level(), origin, self:get_projectile(), target, 0, false, 0)
          end  
        end
        
        break
      end
    end
    
    return true
end

local function on_exit(self)
    
end


return {
    on_enter = on_enter,
    on_exit = on_exit
}