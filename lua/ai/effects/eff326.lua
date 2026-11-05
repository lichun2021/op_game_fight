-- 326 完全抵挡一次任意类型伤害
local get_innereffdata = require "effects/getinnereffdata"

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- or
--     local effs = character:get_innereffects(__)
--     if not effs or #effs < 1 then return end
-- end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    -- self:set_is_ignorehitlabel(true)
    -- aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
	-- self:set_changevalue(changevalue)
    self:set_innerdata("t", effdata.targetattribute) -- trig exec

    return true
end

local function trigger(attacker, target, skid, sklv)
	local effs = target:get_innereffects(326)
	if not effs or #effs < 1 then return 0 end

	-- 326现在使用原始效果的技能信息和施法者,传入的skid和sklv为伤害的技能信息,attacker为伤害的攻击方,以防需要修改

	for i=1,#effs do
		local eff = effs[i]
		local execid = eff:get_innerdata("t")
		if execid > 0 then
			local eff_origin = eff:get_origin()
			local eff_skid = eff:get_skill_id()
			local eff_sklv = eff:get_skill_level()
			-- g_exechelper.execute(exec, eff_skid, eff_sklv, eff_origin, nil, target, 0, false, 0)
			aicontext.schedule_execute(execid, eff_skid, eff_sklv, eff_origin, nil, target, 0, false, 0)
		end
	end

end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

return {
    -- check_do = check_do,
	on_enter = on_enter,
	trigger = trigger
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}