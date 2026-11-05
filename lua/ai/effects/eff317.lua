-- 317 目标血量降低指定数量/百分比后触发执行

local get_innereffdata = require "effects/getinnereffdata"
-- local calcchangevalue = require "effects/calcchangevalue"
local AttrType = require "datadefs/attrtype"


local function on_enter(self)
    if self:get_is_debuffed() then return false end
    
    local innerexec = get_innereffdata(self)
  
    local skilllevel = self:get_skill_level()
    local target = self:get_target()

    local changevalue = (innerexec.changevalue + innerexec.levelup * skilllevel) / 100

    self:set_changevalue(changevalue)
    self:set_innerdata("ct", innerexec.changetype) -- 数值变化类型 0-非百分比 1-百分比
    self:set_innerdata("vt", innerexec.type) -- 数值加减类型 1-增加 2-减少 现在只支持2
    self:set_innerdata("base", innerexec.valuebase)

    local params = innerexec.params
    if #params < 4 then
        return false
    end

    self:set_innerdata("tnum", params[1]) -- 触发次数
    self:set_innerdata("tcd", params[2]) -- 触发间隔
    self:set_innerdata("multi", params[3]) -- 是否可同时触发多次

    aieffectcaster.start_innereffect_cd(target, self)
    
    return true
end

local function triggerinner(character, attrdelta, eff)
    local dv = eff:get_innerdata("dv") + math.abs(attrdelta) -- deltavalue
    local compval = eff:get_changevalue()
    if eff:get_innerdata("ct") == 1 then
        local base = eff:get_innerdata("base")
        if base > 0 then
            compval = compval / 100 * character:get_attr(base)
        else
            compval = compval / 100 * character:get_attr(AttrType.HPTop)
        end
    end

    if dv >= compval then
        local count = 1 --可触发次数
        if eff:get_innerdata("multi") > 0 then
            count = dv // compval
            if eff:get_innerdata("tnum") > 0 then
                local left = eff:get_innerdata("tnum") - eff:get_innerdata("tcnt")
                count = math.min(count, left)
            end
            eff:set_innerdata("dv", dv - count * compval)
        else
            eff:set_innerdata("dv", 0)
        end

        local tcnt = eff:get_innerdata("tcnt") + count -- 已触发次数
        eff:set_innerdata("tcnt", tcnt)

        local caster = eff:get_parentcaster()
        local pt = caster:get_localtick()
        local cd = eff:get_innerdata("tcd")
        eff:set_innerdata("ntick", pt + cd)

        local effdata = get_innereffdata(eff)
        local params = effdata.params
        for i=1,count do
            -- 触发执行,从第5位开始
            for j=5,#params do
                local anexecid = params[j]
                if anexecid > 0 then
                    if params[4] > 0 then -- 原始施法者
                        g_exechelper.execute(anexecid, eff:get_skill_id(), eff:get_skill_level(), eff:get_originsafe(), eff:get_projectile(), eff:get_originsafe(), 0, false, 0)
                    else
                        g_exechelper.execute(anexecid, eff:get_skill_id(), eff:get_skill_level(), eff:get_originsafe(), eff:get_projectile(), character, 0, false, 0)
                    end
                end
            end
        end

        if params[1] > 0 and tcnt >= eff:get_innerdata("tnum") then
            eff:set_temp_invalid(true)
            eff:set_valid(false)
            -- eff:set_innerdata("isbroken", 1)
            eff:remove_ui_effect()
        end

    else
        eff:set_innerdata("dv", dv)
    end
end

local function check_do(character, attrdelta)
    local effs = character:get_innereffects(317)
    if not effs or #effs < 1 then return 0 end

    -- MARK: 现在只在减少的情况下调用,不用判断正负值

    for i=1, #effs do
        local eff = effs[i]
        if not eff:is_temp_invalid() then

            local ntick = eff:get_innerdata("ntick")
            local can_trigger = true

            if ntick > 0 then
                local caster = eff:get_parentcaster()
                if caster:get_localtick() > ntick then
                    eff:set_innerdata("ntick", 0)
                else
                    can_trigger = false
                end
            end

            if can_trigger then
                triggerinner(character, attrdelta, eff)
            end

        end
    end
end

-- local function on_exit(self)
-- end

-- local function on_scheduletick(self)
-- end

return {
    check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}