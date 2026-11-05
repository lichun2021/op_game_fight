-- summonhelper

local Attr = require("datadefs/attrtype")
local TeamListType = require("datadefs/teamlisttype")
local CasterType = require("datadefs/castertype")
local TeamSide = require("datadefs/teamside")

local theaisummoncaster = aisummoncaster
local thedatahelper

local function can_hurtsummon(source, hurtid, target, skillid)
    return theaisummoncaster.can_hurtsummon(source, hurtid, target, skillid)
end

local function init_summonpool(caster, summoner, summonid)
    local summondata = g_datahelper.get_summonpropdata(summonid)
    local pool_type = 0
    -- TODO: optimise
    if summondata then 
        local summonconf = summondata["conf"]
        if summonconf and #summonconf >= 28 and summonconf[28] == 1 then
            pool_type = 2
        end
    end
    theaisummoncaster.init_summonpool(caster, summoner, summonid, pool_type)
end

local default_summon_attr_inherit_list = {
    Attr.HPTop,
    Attr.PHY,
    Attr.AGILE,
    Attr.INT,
    Attr.ACCURATE,
    Attr.SPIRIT,
    Attr.BAPower,
    Attr.MAPower,
    Attr.PHY_CRIT,
    Attr.MAG_CRIT,
    Attr.PHY_CRIT_MUL,
    Attr.MAG_CRIT_MUL,
    Attr.PHY_HIT,
    Attr.MAG_HIT,
    Attr.PHY_DE,
    Attr.MAG_DE,
    Attr.PHY_PE,
    Attr.MAG_PE,
    Attr.DODGE_RATE,
    Attr.RESIST_RATE,
    Attr.BLOOD_SUCK,
    Attr.CURE
}

local function summon_inherit_attr(summon, summondata, target)
    local summon_lv = summon:get_attr(Attr.Lv)
    local inheritbase = summondata["inherit"]
    if inheritbase > 0 then
        local inheritgrow = summondata["inheritP"]
        local inheritratio = (inheritbase + inheritgrow * summon_lv) / 10000
        for i=1,#default_summon_attr_inherit_list do
            local attridx = default_summon_attr_inherit_list[i]
            local val = target:get_attr(attridx) * inheritratio
            val = math.floor(val)
            summon:set_attr(attridx, val)
        end
    end

    -- TODO: optimise
    local extraconf = summondata["arr"]
    if extraconf and #extraconf >= 4 then
        local special_inheritconfig = extraconf[4]
        if special_inheritconfig and #special_inheritconfig > 0 then
            for i=1,#special_inheritconfig do
                local a_special_inherit = special_inheritconfig[i]
                if a_special_inherit and #a_special_inherit >= 3 then
                    local a_special_attridx = a_special_inherit[1]
                    local a_special_inheritbase = a_special_inherit[2]
                    local a_special_inheritgrow = a_special_inherit[3]

                    local a_special_inheritratio = (a_special_inheritbase + a_special_inheritgrow * summon_lv) / 10000
                    local val = target:get_attr(a_special_attridx) * a_special_inheritratio
                    val = math.floor(val)
                    summon:set_attr(a_special_attridx, val)
                end
            end
        end
    end

    summon:set_attr(Attr.HP, summon:get_attr(Attr.HPTop))
end

local function check_summon_inherit_attr(character, summonid)
    local summondata = g_datahelper.get_summonpropdata(summonid)
    local extraconf = summondata["arr"]
    -- TODO: optimise
    if extraconf and #extraconf >= 5 then
        local inheritconf = extraconf[5]
        if inheritconf and #inheritconf > 0 then
                -- 幻象激活时继承属性规则
                -- 0-不重新继承属性
                -- 1-继承召唤者的当前属性
                -- 2-继承拥有指定标记的第一个友军的当前属性[绝对阵营]
                -- 3-继承拥有指定标记的第一个敌军的当前属性[绝对阵营]
                local inherit_type = inheritconf[1]
                if inherit_type == 1 then
                        local summoner = character:get_summonsource()
                        summon_inherit_attr(character, summondata, summoner)
                elseif inherit_type == 2 or inherit_type == 3 then
                        local the_markid = inheritconf[2]
                        local teamside = character:get_teamside()
                        local clusternum = character:get_val("clusternum")
                        if inherit_type == 3 then
                            if teamside == TeamSide.kOur then
                                teamside = TeamSide.kEnemy
                            else
                                teamside = TeamSide.kOur
                            end
                            clusternum = aicontext.get_current_team_clusternum(teamside)
                        end
                        local teamlist = aicontext.get_teamlist(TeamListType.AIContextTeamListType.AITEAM_ORIGIN, teamside, clusternum, true)
                        if teamlist then
                            for i=1,#teamlist do
                                local a_c = teamlist[i]
                                if a_c:is_summon() ~= true and g_actionhelper.is_deadaction(a_c:get_currentaction()) ~= true and a_c:has_caster(CasterType.eMarkExec, the_markid) then
                                    summon_inherit_attr(character, summondata, a_c)
                                    break
                                end
                            end
                        end
                end
        end
    end
end

local function on_summon_awake(caster, character, summonid)
    check_summon_inherit_attr(character, summonid)
end

local function get_group_summon_available_count(caster_myid, character, execid)
    --[[
    thedatahelper = thedatahelper or g_datahelper
    local summonexec = thedatahelper.get_summonexecdata(execid)
    if not summonexec then return -1 end
    if not summonexec.group then return -1 end

    -- group:[id,weight,maxnum]
    local groupid = summonexec.group[1]
    if groupid <= 0 then return -1 end

    local groupweight = summonexec.group[2]
    
    local available_count = summonexec.group[3]

    local summon_casters = character:get_summon_casters()
    if not summon_casters then 
        return available_count 
    end

    for i=1,#summon_casters do
        local a_caster = summon_casters[i]
        if a_caster:get_myid() ~= caster_myid then
            local a_caster_execid = a_caster:get_execid()
            local a_caster_exec = thedatahelper.get_summonexecdata(a_caster_execid)
            if a_caster_exec and a_caster_exec.group and a_caster_exec.group[1] == groupid then
                if a_caster_exec.group[2] > groupweight then
                    local a_caster_active_count = a_caster:get_active_count()
                    available_count = available_count - a_caster_active_count
                    if available_count <= 0 then
                        return 0
                    end
                end
            end
        end
    end

    return available_count
    --]]

    return theaisummoncaster.get_group_summon_available_count(caster_myid, character, execid)
end

local function sleep_group_summon(group_id, group_weight, character, exclude_caster_myid, sleep_group_num)
    local sleeped_num = 0
    local to_sleep_num = sleep_group_num
    local casters = character:get_summon_casters()
    thedatahelper = thedatahelper or g_datahelper
    for i=1,#casters do
        local a_caster = casters[i]
        if a_caster and a_caster:get_myid() ~= exclude_caster_myid then
            local a_caster_execid = a_caster:get_execid()
            local a_caster_exec = thedatahelper.get_summonexecdata(a_caster_execid)
            if a_caster_exec and a_caster_exec.group then
                local a_caster_group = a_caster_exec.group
                if a_caster_group[1] == group_id and a_caster_group[2] <= group_weight then
                    local a_summon_id = a_caster:get_summonid()
                    local a_sleep_num = a_caster:remove_summon(a_summon_id, to_sleep_num)

                    sleeped_num = sleeped_num + a_sleep_num
                    if sleeped_num >= sleep_group_num then
                        return
                    end

                    to_sleep_num = sleep_group_num - sleeped_num
                end
            end
        end
    end
end

local function check_sleep_group_summon(caster_myid, character, execid, group_id, group_weight, group_max_num, current_group_summon_count)
    if group_id <= 0 or group_max_num <= 0 then return end

    if current_group_summon_count <= group_max_num then return end

    local to_sleep_group_num = current_group_summon_count - group_max_num

    -- sleep_group_summon(group_id, group_weight, character, caster_myid, to_sleep_group_num)
    theaisummoncaster.sleep_group_summon(group_id, group_weight, character, caster_myid, to_sleep_group_num)
end

local function on_finish_awake_summon(caster_myid, character)
end

return {
    can_hurtsummon = can_hurtsummon,
    init_summonpool = init_summonpool,
    on_summon_awake = on_summon_awake,
    get_group_summon_available_count = get_group_summon_available_count,
    check_sleep_group_summon = check_sleep_group_summon,
    on_finish_awake_summon = on_finish_awake_summon
}