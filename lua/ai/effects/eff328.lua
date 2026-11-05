-- 328 释放技能时触发执行
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

local function compareenterorexit(enterorexit, params)
	if enterorexit == 0 and params[1] == 1 then -- 释放技能时是否触发
		return true
	elseif enterorexit == 1 and params[3] == 1 then -- 技能结束时是否触发
		return true
	end
	-- print(string.format("enterorexit false:%d, param1:%d, param3:%d", enterorexit, params[1], params[3]))
	return false
end

local function compareskilltype(sktype, params)
	if sktype == 0 and params[8] == 1 then return true -- 普攻是否触发
	elseif sktype == 1 and params[10] == 1 then return true -- 大招是否触发
	elseif sktype == 2 and params[9] == 1 then return true -- 小技能是否触发
	end
	-- print("skilltype false:"..tostring(sktype))
	return false
end

local function comparereleasetype(isforce, params)
	if isforce and params[11] == 0 then return false end
	-- print("isforce false:"..tostring(isforce))
	return true
end

local function trigger(c, skid, sklv, sktype, isforce, enterorexit)
	-- 效果参数
	-- 1a-释放技能时是否触发 0-不触发 1-触发
	-- 2b-释放技能时触发延迟帧数 -1-立即触发 >=0 在所有角色计算完成后的延迟帧
	-- 3c-技能结束时是否触发 0-不触发 1-触发
	-- 4d-技能结束时触发延迟帧数 -1-立即触发 >=0 在所有角色计算完成后的延迟帧
	-- 5e-内置CD 触发后开始CD，期间不会再次触发
	-- 6f-最大触发次数 0-无限制 >0触发最大次数，达到最大次数后效果失效
	-- 7g-单次触发概率 0-100
	-- 8h-普攻是否触发 0-不触发 1-触发
	-- 9i-小技能是否触发 0-不触发 1-触发
	-- 10j-大招是否触发 0-不触发 1-触发
	-- 11k-强制释放技能是否触发 0-不触发 1-触发
	-- 12l-释放执行时施法者 0-效果拥有者 1-效果原始施法者
	-- 13m...-执行id，可填多个

	local effs = c:get_innereffects(328)
	if not effs or #effs < 1 then return 0 end

	local aicontext = aicontext

	for i=1,#effs do
		local eff = effs[i]
		local effdata = get_innereffdata(eff)

		local eff_origin = eff:get_origin()
		local eff_skid = eff:get_skill_id()
		local eff_sklv = eff:get_skill_level()

		local params = effdata.params
		local innercd = params[5]
		local cando
		local parentcaster

		-- print("[eff328]")

		if eff:is_valid() then
				if compareenterorexit(enterorexit, params) 
				and compareskilltype(sktype, params) 
				and comparereleasetype(isforce, params) then
					cando = true
				else
					cando = false
				end

				-- print("cando 1:" .. tostring(cando))
		
				if cando and innercd > 0 then
					parentcaster = eff:get_parentcaster()
					local ntick = eff:get_innerdata("ntick")
					if ntick > 0 then
						if parentcaster:get_localtick() <= ntick then
							cando = false
						end
					end
				end
		
				local randomlimit = params[7]
				if cando and randomlimit < 100 and aicontext.get_randomnumber() >= randomlimit then
					cando = false
				end
		end

		if cando then
				local waittick = enterorexit == 0 and params[2] or params[4]
				local exec_origin = params[12] == 0 and c or eff_origin -- 释放执行时施法者 0-效果拥有者 1-效果原始施法者

				for j=13,#params,1 do
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

				local maxlimit = params[6]
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