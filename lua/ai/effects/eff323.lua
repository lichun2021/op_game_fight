-- 323 对盾的效果造成伤害，只会打盾不会对人造成伤害
local get_innereffdata = require "effects/getinnereffdata"
local calcchangevalue = require "effects/calcchangevalue"
--local HurtType = require "hurtfuncs/hurttype"
local HitType = require "hurtfuncs/hittype"
local HurtParam = require "hurtfuncs/hurtparam"

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
    local source = self:get_origin()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local skillid = self:get_skill_id()
    local skilllevel = self:get_skill_level()
    -- 百分比读取的属性是施法者的属性
    local changevalue = calcchangevalue(effdata, skilllevel, source)
    -- self:set_changevalue(changevalue)

    local hpdelta = changevalue
    local hurttype = effdata.hurttype - 1
    local hurtparam = HurtParam.new(nil, skillid, skilllevel, hpdelta, hurttype, 0, HitType.NORMAL_HIT, 0, false, 0)
    hurtparam.is_fromeffect = true
    if effdata.hurttype == 0 then
        hurtparam.is_rawhurt = true
    end

    local remainhurt, hurtnum = aicontext.hurt_shield(source, target, hurtparam, effdata.params)

    if effdata.label ~= 0 and hurtnum ~= 0 then
        -- local hitvalue = remainhurt - changevalue
        local hitvalue = -hurtnum
        g_hitlabel.execute(effdata.label, hitvalue, target)
    end

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
}