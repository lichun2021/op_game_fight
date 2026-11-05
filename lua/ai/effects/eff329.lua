-- 329 强制转向(不打断当前动作)
local get_innereffdata = require "effects/getinnereffdata"
local TeamSide = require("datadefs/teamside")

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
	local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)

	local direction = target:get_val("direction")

	local type = effdata.params[1]
	if type == 1 then -- 1:固定向右转
		direction = 1 
	elseif type == 2 then -- 2:固定向左转
		direction = -1 
	elseif type == 3 then -- 3:向出场方向转
		if target:get_teamside() == TeamSide.kOur then
			direction = 1
		else
			direction = -1
		end
	elseif type == 4 then -- 4:向出场方向的反向转
		if target:get_teamside() == TeamSide.kOur then
			direction = -1 
		else
			direction = 1
		end
	elseif type == 5 then -- 5:当前方向的反向转
		direction = -direction
	end

	target:set_val("direction", direction)

    return true
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

return {
    -- check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
	-- trigger = trigger
}