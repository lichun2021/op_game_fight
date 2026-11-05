-- 319 判断场上有几个带有指定标签的角色触发执行

local get_innereffdata = require "effects/getinnereffdata"
local TeamListType = require("datadefs/teamlisttype")
local TeamSide = require("datadefs/teamside")

local function append_label(labels, label)
    if label > 0 then
        labels[#labels + 1] = label
    end
end

local function match_cnt(characters, complabels, comptype, compcnt)
    local matchcnt = 0

    for i=1,#characters do
        local c = characters[i]
        local has = false
        local yescnt = 0
        local nocnt = 0
        for j=1,#complabels do
            local label = complabels[j]
            if c:has_label(label) then
                has = true
                break
            end
        end
        if has then
            yescnt = 1
        else
            nocnt = 1
        end

        -- 0-有 1-没有
        if comptype == 0 then
            matchcnt = matchcnt + yescnt
        else
            matchcnt = matchcnt + nocnt
        end

        if matchcnt >= compcnt then
            return compcnt
        end
    end

    return matchcnt
end

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    local origin = self:get_origin()
    local target = self:get_target()

    -- 关系类型
    -- 0-或:场上的有A或B的标签并满足个数时触发，若某个英雄同时满足两个条件则此英雄也算作1个
    -- 1-非:场上的没有A和B的标签并满足个数时触发
    local ct = effdata.changetype -- comparetype

    -- 数值大小:3表示有3个人时触发,999表示此效果触发时需要所有友方都满足时触发（排除幻象）
    local compcnt = effdata.changevalue

    -- 上下限开关  数值下限基础值  下限成长 数值上限基础值 上限成长 这5个地方填写标签的ID
    local complabels = {}
    append_label(complabels, effdata.limitonoff)
    append_label(complabels, effdata.limitvallowerbase)
    append_label(complabels, effdata.limitvallowergrowth)
    append_label(complabels, effdata.limitvalupperbase)
    append_label(complabels, effdata.limitvaluppergrowth)

    -- 目标属性:0-全体 1-己方 2-敌方 相对于施法者
    local rt = effdata.targetattribute -- relationtype
    local teamcnt = 0

    local mc = 0
    if rt == 0 or rt == 1 then
        local teamside = origin:get_teamside()
        local clusternum = origin:get_val("clusternum")
        local characters = aicontext.get_teamlist(TeamListType.AIContextTeamListType.AITEAM_ORIGIN, teamside, clusternum, true)
        mc = mc + match_cnt(characters, complabels, ct, compcnt)
        teamcnt = teamcnt + #characters
    end

    if mc < compcnt and ( rt == 0 or rt == 2 ) then
        local teamside = origin:get_teamside()
        if teamside == TeamSide.kOur then
            teamside = TeamSide.kEnemy
        else
            teamside = TeamSide.kOur
        end
        local clusternum = aicontext.get_current_team_clusternum(teamside)
        local characters = aicontext.get_teamlist(TeamListType.AIContextTeamListType.AITEAM_ORIGIN, teamside, clusternum, true)
        mc = mc + match_cnt(characters, complabels, ct, compcnt)
        teamcnt = teamcnt + #characters
    end

    if compcnt == 999 then
        compcnt = teamcnt
    end

    if mc >= compcnt then
        local skillid = self:get_skill_id()
        local skilllevel = self:get_skill_level()
        local proj = self:get_projectile()

        local params = effdata.params
        for i=1,#params do
            local anexecid = params[i]
            if anexecid > 0 then
                g_exechelper.execute(anexecid, skillid, skilllevel, origin, proj, target, 0, false, 0)
            end
        end
    end

    return false
end

return {
    -- check_do = check_do,
    on_enter = on_enter
    -- on_exit = on_exit,
    -- on_scheduletick = on_scheduletick
}