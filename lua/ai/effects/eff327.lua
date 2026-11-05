-- 327 拥有效果的目标，持续时间内受到伤害，对伤害来源的目标触发执行
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

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    -- local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(changevalue)

    return true
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

local function trigger(c, attacker, hurttype, hurtprop)

	-- //	1a 是否百分比:可否对不可选中目标生效 0 - 不生效 1 - 生效
	-- //	2b 目标属性:可否对幻象生效 0 - 不生效 1 - 生效
	-- //	3c 数值计算基准:内置CD
	-- //	4d 数值大小:最大触发次数 0 - 不限次数 > 0触发次数达到限制后效果失效
	-- //	5e 等级成长:触发执行延迟 -1立即触发 >=0 延迟
	-- //	6f 上下限开关:触发执行的施法者 0-效果拥有者 1-效果施法者

	-- //效果参数：[]
	-- //	伤害类型数量 后面配相应数量的伤害类型
	-- //	... 伤害类型 : 0 - 物理 1 - 法术 3 - 纯粹

	-- //	伤害属性数量 跟在伤害类型后，之后配相应数量的伤害属性

	-- //	触发的执行id 跟在伤害属性后，后续均为执行id

	local effs = c:get_innereffects(327)
	if not effs or #effs < 1 then return 0 end

	local aicontext = aicontext

	for i=1,#effs do
		local eff = effs[i]
		local effdata = get_innereffdata(eff)

		local params = effdata.params
		local innercd = effdata.valuebase -- p3
		local cando
		local parentcaster
		local execid_startidx

		-- print("[eff327]")

		repeat
			if not eff:is_valid() then break end

			-- 可否对不可选中目标生效 0 - 不生效 1 - 生效
			if not attacker:is_findabletoattacker(c) and effdata.changetype == 0 then break end

			-- 可否对幻象生效 0 - 不生效 1 - 生效
			if attacker:is_summon() and effdata.targetattribute == 0 then break end

			-- 内置CD
			if innercd > 0 then
				parentcaster = eff:get_parentcaster()
				local ntick = eff:get_innerdata("ntick")
				if ntick > 0 then
					if parentcaster:get_localtick() <= ntick then
						break
					end
				end
			end

			-- 1:伤害类型数量 后面配相应数量的伤害类型
			local typecntidx = 1
			local typecnt = params[typecntidx]
			local istype
			-- print("hurttype:"..tostring(hurttype))
			for ti=1,typecnt do
				-- print("vs "..params[typecntidx + ti])
				if hurttype == params[typecntidx + ti] then
					istype = true
					break
				end
			end

			-- print("istype:"..tostring(istype))

			-- 伤害属性数量
			if istype then
				local propcntidx = typecntidx + typecnt + 1
				local propcnt = params[propcntidx]

				-- print("hurtprop:"..tostring(hurtprop))
				
				for pidx=1,propcnt do
					-- print("vs "..tostring(params[propcntidx + pidx]))
					if hurtprop == params[propcntidx + pidx] then
						cando = true
						-- print("cando:"..tostring(cando))
						break
					end
				end

				execid_startidx = propcntidx + propcnt + 1
			end

		until true

		-- print("cando is "..tostring(cando))

		if cando then
				local waittick = effdata.levelup --触发执行延迟 -1立即触发 >=0 延迟
				local exec_origin = effdata.limitonoff == 0 and c or eff:get_origin() -- 释放执行时施法者 0-效果拥有者 1-效果施法者

				local eff_skid = eff:get_skill_id()
				local eff_sklv = eff:get_skill_level()

				for j=execid_startidx,#params,1 do
					local execid = params[j]

					if waittick < 0 then
						aicontext.execute_exec(execid, eff_skid, eff_sklv, exec_origin, nil, c, 0, false, 0)
					else
						aicontext.schedule_execute(execid, eff_skid, eff_sklv, exec_origin, nil, c, 0, false, waittick)
					end
				end

				if innercd > 0 then
					eff:set_innerdata("ntick", parentcaster:get_localtick() + innercd)
				end

				-- 最大触发次数 0 - 不限次数 > 0触发次数达到限制后效果失效
				local maxlimit = effdata.changevalue
				if maxlimit > 0 then
					local tcnt = eff:get_innerdata("tcnt") + 1
					eff:set_innerdata("tcnt", tcnt)
					if tcnt >= maxlimit then
						eff:set_temp_invalid(true)
						eff:set_valid(false)
						eff:set_innerdata("isbroken", 1)
						eff:remove_ui_effect()
					end
				end
		end
	end
end

return {
    -- check_do = check_do,
    on_enter = on_enter,
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
	trigger = trigger
}