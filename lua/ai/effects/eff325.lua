-- 325 指定属性阈值
--[[
    新增一个效果，使目标的指定属性无法高于或低于指定值，指定值可以填固定值或百分比
    添加后保持，当指定属性改变时判断指定属性是否高于或低于指定值。
    如果判定成功，则使所有改变该属性的效果暂时失效并将指定属性改为指定值，本效果结束后数值返还。
    在本效果判断成功后所有影响指定值的效果在生效和失效时都会再次触发本效果。

    该效果多个叠加时以指定属性为单位后添加的优先。
--]]

--[[
上下限数值配置使用原有配置
上下限规则配置：上下限开关
0- <=上限
1- >=下限
2- >=下限 且 <=上限

百分比：
0-非百分比
1-开场技能释放后百分比
2-开场技能释放前百分比
--]]

--[[
    记录还原数值的规则：
    超过上限时，记录超出量；
    低于下限时，记录减少量；
]]

--[[
# 注意点
* 排除血量：最大血量和当前血量
* 效果本身不会对特殊属性做特殊处理，策划配置时规避
--]]

--[[
# 问题
* 百分比要用开场时角色初始属性作为基准
--]]

local get_innereffdata = require "effects/getinnereffdata"

local function check_do(character, idx, attrval, exiteff)
    local effs = character:get_innereffects(325)
    if not effs or #effs < 1 then return 0 end

    local oldval = attrval
    local diff = 0

    for i=#effs,1,-1 do
        local eff = effs[i]
        if exiteff and exiteff:get_uid() == eff:get_uid() then
        	-- continue
        elseif eff:is_temp_invalid() then
        	-- continue
        elseif eff:get_innerdata("idx") == idx then
            local limittype = eff:get_innerdata("lt")
            local uplimit = eff:get_innerdata("up")
            local dnlimit = eff:get_innerdata("dn")

            if (limittype == 0 or limittype == 2) and uplimit > 0 and oldval > uplimit then
                local upval = eff:get_innerdata("upval")
                upval = oldval - uplimit + upval
                eff:set_innerdata("upval", upval)
                character:set_attr(idx, uplimit)
                diff = uplimit - oldval
            end

            if (limittype == 1 or limittype == 2) and dnlimit > 0 and oldval < dnlimit then
                local dnval = eff:get_innerdata("dnval")
                dnval = dnlimit - oldval + dnval
                eff:set_innerdata("dnval", dnval)
                character:set_attr(idx, dnlimit)
                diff = dnlimit - oldval
            end

            break
        end
    end

    return diff
end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local character = self:get_target()
    aieffectcaster.start_innereffect_cd(character, self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)

    -- limit type: 0- <=up  1- >=down 2- >=down & <=up
    local limittype = effdata.limitonoff
    local idx = effdata.targetattribute
    local uplimit = (effdata.limitvalupperbase + effdata.limitvaluppergrowth * skilllevel) / 100
    local dnlimit = (effdata.limitvallowerbase + effdata.limitvallowergrowth * skilllevel) / 100
    -- 0-非百分比 1-开场技能释放后百分比 2-开场技能释放前百分比
    if effdata.changetype == 1 then 
        local attrval = character:get_attr_afterbegin(idx)
        uplimit = uplimit / 100 * attrval
        dnlimit = dnlimit / 100 * attrval
    elseif effdata.changetype == 2 then
        local attrval = character:get_attr_origin(idx)
        uplimit = uplimit / 100 * attrval
        dnlimit = dnlimit / 100 * attrval
    end

    self:set_innerdata("idx", idx)
    self:set_innerdata("lt", limittype)
    self:set_innerdata("up", math.floor(uplimit)) -- uplimit
    self:set_innerdata("dn", math.floor(dnlimit)) -- downlimit
    self:set_innerdata("hl", effdata.label) -- hitlabel

    local oldval = character:get_attr(idx)
    if (limittype == 0 or limittype == 2) and uplimit > 0 and oldval > uplimit then
        local upval = oldval - uplimit
        self:set_innerdata("upval", upval)
        character:set_attr(idx, uplimit)
    end

    if (limittype == 1 or limittype == 2) and dnlimit > 0 and oldval < dnlimit then
        local dnval = dnlimit - oldval
        self:set_innerdata("dnval", dnval)
        character:set_attr(idx, dnlimit)
    end
    local diff = character:get_attr(idx) - oldval
    if diff ~= 0 then
        g_hitlabel.execute(effdata.label, diff, character)
    end

    return true
end

local function on_exit(self)
    local upval = self:get_innerdata("upval")
    local dnval = self:get_innerdata("dnval")
    local limittype = self:get_innerdata("lt")

    local c = self:get_target()
    local idx = self:get_innerdata("idx")
    local oldval = c:get_attr(idx)
    local diff = 0

    if upval ~= 0 and (limittype == 0 or limittype == 2) then
        diff = upval
    end

    if dnval ~= 0 and (limittype == 1 or limittype == 2) then
        diff = -dnval
    end

    if diff ~= 0 then
        local attrval = oldval + diff
        c:set_attr(idx, attrval)
        local anotherdiff = check_do(c, idx, attrval, self)
        if anotherdiff ~= 0 then
            diff = c:get_attr(idx) - oldval
        end

        if diff ~= 0 then
            g_hitlabel.execute(self:get_innerdata("hl"), diff, c)
        end
    end

end

-- local function on_scheduletick(self)
-- end

return {
    check_do = check_do,
    on_enter = on_enter,
    on_exit = on_exit
    -- on_scheduletick = on_scheduletick
}