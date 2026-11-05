-- 291 受到指定ID的角色伤害时，附加额外伤害到初始伤害
local AttrType = require "datadefs/attrtype"
local get_innereffdata = require "effects/getinnereffdata"

local function check_do(attacker, target, hpdelta)
    local effs = target:get_innereffects(291)
    if not effs or #effs < 1 then return 0 end

    local attacker_id = attacker:get_attr(AttrType.ID)
    local attacker_teamside = attacker:get_teamside()
    local target_teamside = target:get_teamside()

    local allchangevalue = 0
    for i=1,#effs do
        local aneff = effs[i]
        local cid = aneff:get_innerdata("cid")
        local ts = aneff:get_innerdata("ts")
        if cid < 0 or attacker_id == cid then
            if ts == 2 or (ts == 0 and attacker_teamside == target_teamside) or (ts == 1 and attacker_teamside ~= target_teamside) then
                local ct = aneff:get_innerdata("ct")
                if ct == 1 then
                    allchangevalue = allchangevalue + aneff:get_changevalue() / 100 * hpdelta / 100
                else
                    allchangevalue = allchangevalue + aneff:get_changevalue() / 100
                end
            end
        end
    end
    return math.floor(allchangevalue)
end

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)
    self:set_innerdata("ct", effdata.changetype)
    self:set_innerdata("cid", effdata.targetattribute)
    self:set_innerdata("ts", effdata.valuebase)

    return true
end

local function on_exit(self)
end

return {
    check_do = check_do,
    on_enter = on_enter,
    on_exit = on_exit
}