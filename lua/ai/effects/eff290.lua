-- 290 攻击背后必定暴击
local AttrType = require "datadefs/attrtype"
local get_innereffdata = require "effects/getinnereffdata"

local function check_do(attacker, target)
    if not attacker:has_effect(290) then return false end
    local direction_target = target:get_direction()

    local is_attacker_face_target_back = false

    local x_attacker = attacker:get_attr(AttrType.X)
    local x_target = target:get_attr(AttrType.X)

    if direction_target == 1 and x_attacker <= x_target then
        is_attacker_face_target_back = true
    elseif direction_target == -1 and x_attacker >= x_target then
        is_attacker_face_target_back = true
    end

    if not is_attacker_face_target_back then return false end

    -- local effs = attacker:get_innereffects(290)
    -- if effs == nil or #effs < 1 then return false end

    -- local allchangevalue = 0
    -- for i=1,#effs do
    --     local aneff = effs[i]
    --     allchangevalue = allchangevalue + aneff:get_changevalue() / 100
    -- end
    -- return allchangevalue

    return true
end

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    -- local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(changevalue)

    return true
end

local function on_exit(self)
end

return {
    check_do = check_do,
    on_enter = on_enter,
    on_exit = on_exit
}