-- 330 控制效果
local get_innereffdata = require "effects/getinnereffdata"
local outofcontrolaction = require "actions/outofcontrolaction"
local ActionType = require "actions/actiontype"

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- or
--     local effs = character:get_innereffects(__)
--     if not effs or #effs < 1 then return end
-- end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

	local parentcaster = self:get_parentcaster()
    local casterdata = g_datahelper.get_effectdata(parentcaster:get_effectid())
	if casterdata.conf[9] ~= 0 then return false end -- 优先使用原有眩晕配置

	local target = self:get_target()
	if target:has_effect(281) then return false end -- 不可覆盖281冰冻效果

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
	
    local efftick = casterdata.conf[6]

    -- 已有同类控制效果，剩余时间以更长的为准
    if target:get_currentaction() == ActionType.CONTROLLED_EFFECT_ACTION then
    	local remaintick = target:get_totaltick() + target:get_starttick() - target:get_localtick()
    	-- 同类效果的持续时间更久，则只增加tag，不重置目标动作时间了
    	if remaintick >= efftick then
			local effcnt = target:get_tag("eff330") + 1
			target:set_tag("eff330", effcnt)

			-- 仅第一个控制效果更新动画
			if g_platform ~= "SERVER" and effcnt == 1 then
				--ui动作同步
				local stateanimation = effdata.changevalue
				aiactioncontroller.enter_uieffectaction(target, stateanimation, effdata.params[1], effdata.params[2], effdata.params[3], effdata.params[4])
			end
    		return true
    	end
    end

    -- 其他情况，直接进入受控状态（已经是受控的，则覆盖动作时间，以后上的且时间更久的为准）
	local issuccess = outofcontrolaction.enter(target, efftick, ActionType.CONTROLLED_EFFECT_ACTION, self:get_origin())
	if issuccess == true then
			local effcnt = target:get_tag("eff330") + 1
			target:set_tag("eff330", effcnt)

			-- 仅第一个控制效果更新动画
			if g_platform ~= "SERVER" and effcnt == 1 then
				--ui动作同步
				local stateanimation = effdata.changevalue
				aiactioncontroller.enter_uieffectaction(target, stateanimation, effdata.params[1], effdata.params[2], effdata.params[3], effdata.params[4])
			end
	end

    return issuccess and true or false
end

local function on_exit(self)
	local target = self:get_target()

	local effcnt = target:get_tag("eff330") - 1
	if effcnt < 0 then
		effcnt = 0
	end
	target:set_tag("eff330", effcnt)

	local effcount = target:get_tag("eff331")
	if effcnt <= 0 then
		target:erase_tag("eff330")

		--最后一个控制效果时ui动作退出
		if g_platform ~= "SERVER" then
			local effdata = get_innereffdata(self)
			local stateanimation = effdata.changevalue
			aiactioncontroller.exit_uieffectaction(target, stateanimation)
		end

		local curact = target:get_currentaction()
		if curact == ActionType.CONTROLLED_EFFECT_ACTION and effcount == 0 then
			-- 最后一个控制效果消失时退出状态
			outofcontrolaction.enter(target, 1, ActionType.IDLE_ACTION, target)
		end		
	end

end

-- local function on_scheduletick(self)
-- end

return {
    -- check_do = check_do,
    on_enter = on_enter,
    on_exit = on_exit
    -- on_scheduletick = on_scheduletick
}