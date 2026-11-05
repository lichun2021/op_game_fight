-- 283 反伤 以未计算减伤前的伤害值作为反伤初始值
local get_innereffdata = require "effects/getinnereffdata"

local eff = {}

eff.on_enter = function(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)
    self:set_innerdata("ct", effdata.changetype)
    self:set_innerdata("ht", effdata.valuebase) -- -1:all 0~2:phy,mag,heal
    if effdata.limitonoff == 1 then
        local limitupper = (effdata.limitvalupperbase + effdata.limitvaluppergrowth * skilllevel) / 100
        local limitlower = (effdata.limitvallowerbase + effdata.limitvallowergrowth * skilllevel) / 100
        self:set_innerdata("lon", 1) -- limiton
        self:set_innerdata("lhigh", limitupper) -- limithigh
        self:set_innerdata("llow", limitlower) -- limitlow
    end

    return true
end

eff.on_exit = function(self)
end

eff.check_do = function(character, hurttype, hpdelta)
    local effs = character:get_innereffects(283)
    if character:get_remaineffectcd(283) > 0 or effs == nil or #effs < 1 then return 0 end

    local reboundval = 0

    for i=1,#effs do
        local aneff = effs[i]
        local changetype = aneff:get_innerdata("ct")
        -- local aneffdata = get_innereffdata(aneff)
        assert(changetype ~= nil, "changetype is nil!")
        local allowedhurttype = aneff:get_innerdata("ht")
        -- -1-所有 0-物理 1-法术 2-治疗 3-真实伤害（效果执行里的类型）
        if allowedhurttype == -1 or allowedhurttype == hurttype or (allowedhurttype == 3 and hurttype == -1) then
            local changevalue = aneff:get_changevalue()
            local a_reboundval = 0
            if changetype == 1 then
                a_reboundval = changevalue / 100 * hpdelta / 100
            else
                a_reboundval = changevalue / 100
            end

            if aneff:get_innerdata("lon") == 1 then
                local limit_high = aneff:get_innerdata("lhigh")
                if a_reboundval > limit_high then
                    a_reboundval = limit_high
                end

                local limit_low = aneff:get_innerdata("llow")
                if a_reboundval < limit_low then
                    a_reboundval = limit_low
                end
            end

            reboundval = reboundval + a_reboundval
        end
    end

    return math.floor(reboundval)
end

return eff