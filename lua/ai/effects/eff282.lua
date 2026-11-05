-- 282 受到某种类型标签的角色攻击时必定受到暴击，暴击伤害取攻击者的暴击伤害。（可能受试炼，队友被动技能等效果影响）
local AttrType = require "datadefs/attrtype"
local get_innereffdata = require "effects/getinnereffdata"

local eff = {}

eff.on_enter = function(self)

    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    -- TODO: 
    local effdata = get_innereffdata(self)
    assert(effdata.changevalue ~= nil , "get_innereffdata not implemented!")
    self:set_changevalue(effdata.changevalue)

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    return true
end

eff.on_exit = function(self)
end


eff.check_do = function(character, param)
    local effs = character:get_innereffects(282)

    if not effs or #effs < 1 or character:get_remaineffectcd(282) > 0 then return 0 end

    local attacker = param

    for i=1,#effs do
        local aneff = effs[i]
        --local aneffdata = get_innereffdata(aneff)
        local a_label = aneff:get_changevalue() -- TODO: maybe use params
        if attacker:has_label(a_label) then return 1 end
    end

    return 0
end

return eff