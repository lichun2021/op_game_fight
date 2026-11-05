-- aicontexthelper

-- local CDOPCODE = require "datadefs/characterdataopcode"
local calccharactercombatpower = require "targetfuncs/calccharactercombatpower"
local TeamSide = require "datadefs/teamside"
local TeamListType = require "datadefs/teamlisttype"

local function schedule_execute(execid, skillid, skilllevel, source, projectile, target, parenttick, is_uselogicpos, waittick)
    aicontext.schedule_execute(execid, skillid, skilllevel, source, projectile, target, parenttick, is_uselogicpos, waittick)
end

local function on_tonextcluster(adjustdist, clustercenterx)
    -- print("ai:on_tonextcluster:"..tostring(adjustdist)..","..tostring(clustercenterx))
    return false
end

local function on_update(delta)
    -- print("ai:on_update:"..tostring(delta))
    return false
end

local scheduletasks
local to_delete_caster_cnt = 0

local function on_update_end()
    if scheduletasks and #scheduletasks > 0 then
        for i=1,#scheduletasks do
            scheduletasks[i]()
        end
        scheduletasks = {}
    end

    --[[
    local the_to_delete_cache = g_cachehelper.cache.to_delete_casters
    if the_to_delete_cache and #the_to_delete_cache > 0 then
        for i=1,#the_to_delete_cache do
            local a_record = the_to_delete_cache[i]
            aicontext.delete_caster(a_record[1], a_record[2], a_record[3])
        end
        g_cachehelper.cache.to_delete_casters = {}
    end
    --]]

    if to_delete_caster_cnt > 0 then
        aicontext.check_delete_todeletecasters()
        to_delete_caster_cnt = 0
    end
end

local function on_battle_finished()
end

-- false: host should not close lua state
local function on_clear()
    -- print("ai:on_clear")
    g_cachehelper:clear_cache()
    -- collectgarbage()
    
    -- if g_platform == "SERVER" then return false end
    return true
end

local function on_clear_client()
    g_cachehelper:clear_cache()
    return true
end

local function on_clear_server()
    g_cachehelper:clear_cache()
    return true
end

local function init_cluster(cache_summon_different_side, summoner_side, summoner_clusternum, to_side, to_clusternum)
        local teamside = summoner_side
        local clusternum = summoner_clusternum
        local toteamside = to_side
        local toclusternum = to_clusternum
        local cache_team = cache_summon_different_side[teamside]
        if not cache_team then return end
    
        local cache_cluster = cache_team[clusternum]
        if not cache_cluster then return end
    
        for i=1,#cache_cluster do
            local summon_myid = cache_cluster[i]
            local summon = aicontext.get_character_by_myid(summon_myid)
            -- aicontext.remove_character_from_cluster(summon_myid)
            local summon_side = summon:get_teamside()
            local summon_clusternum = summon:get_val("clusternum")
            if teamside ~= summon_side or clusternum ~= summon_clusternum then
                aicontext.remove_character_from_cluster(summon, TeamListType.AIContextTeamListType.AITEAM_ORIGIN, summon_side, summon_clusternum, false)
                aicontext.remove_character_from_cluster(summon, TeamListType.AIContextTeamListType.AITEAM_FRIENDS_FOR_CLUSTER, summon_side, summon_clusternum, false)
            end
            aicontext.remove_character_from_cluster(summon, TeamListType.AIContextTeamListType.AITEAM_ORIGIN, teamside, clusternum, false)
            aicontext.remove_character_from_cluster(summon, TeamListType.AIContextTeamListType.AITEAM_FRIENDS_FOR_CLUSTER, teamside, clusternum, false)
    
            aicontext.add_character_to_cluster(summon, TeamListType.AIContextTeamListType.AITEAM_ORIGIN, toteamside, toclusternum, false)
            aicontext.add_character_to_cluster(summon, TeamListType.AIContextTeamListType.AITEAM_FRIENDS_FOR_CLUSTER, toteamside, toclusternum, false)
            -- summon:set_val("teamside", toteamside)
            summon:set_val("clusternum", toclusternum)
    
            summon:set_team_list_info(TeamListType.CharacterTeamListType.CTEAM_FRIENDS, TeamListType.AIContextTeamListType.AITEAM_ORIGIN, toteamside, toclusternum, false)
            summon:set_team_list_info(TeamListType.CharacterTeamListType.CTEAM_TMP_FRIENDS, TeamListType.AIContextTeamListType.AITEAM_FRIENDS_FOR_CLUSTER, toteamside, toclusternum, false)
        end
end

local function on_to_nextcluster_init_cluster(clusternum_our, clusternum_enemy)
    local cache = g_cachehelper.cache.summon_different_side
    if not cache then return end

    init_cluster(cache, TeamSide.kOur, clusternum_our, TeamSide.kEnemy, clusternum_enemy)
    init_cluster(cache, TeamSide.kEnemy, clusternum_enemy, TeamSide.kOur, clusternum_our)
end

local function on_to_nextcluster_end(adjust, centerx)
    if g_platform ~= "SERVER" and g_platform ~= "MACOS_SERVER" then
        collectgarbage()
    end

    --[[
    if g_hostconfig.beginskill_lazytick > 0 then
        g_cachehelper.cache.aibeginskill_releasetick = g_hostconfig.beginskill_lazytick + aicontext.get_currenttick()
    end
    --]]
end

local function on_addcharacter(character)
    --all_characters[character:get_data(CDOPCODE.CD_MYID)] = character
    if character:is_summon() then
        local fp = calccharactercombatpower(character)
        character:set_val("combatpower", fp)

        -- TODO: optimise compare method
        local summonpropid = character:get_val("summonpropid")
        local summondata = g_datahelper.get_summonpropdata(summonpropid)
        local summonconf = summondata["conf"]
        if summonconf and #summonconf >= 28 and summonconf[28] == 1 then
                -- cache: {summoner_teamside : { summoner_clusternum : [ summon_myid ] }}
                if not g_cachehelper.cache.summon_different_side then
                    g_cachehelper.cache.summon_different_side = {}
                end
                local summoner = character:get_summonsource()
                if summoner then
                    local summoner_teamside = summoner:get_teamside()
                    local cache_team = g_cachehelper.cache.summon_different_side[summoner_teamside]
                    if not cache_team then
                        cache_team = {}
                        g_cachehelper.cache.summon_different_side[summoner_teamside] = cache_team
                    end

                    local summon_clusternum = summoner:get_val("clusternum")
                    local cache_cluster = cache_team[summon_clusternum]
                    if not cache_cluster then
                        cache_cluster = {}
                        cache_team[summon_clusternum] = cache_cluster
                    end

                    cache_cluster[#cache_cluster + 1] = character:get_myid()

                    local summon_teamside = TeamSide.kEnemy
                    if summoner_teamside == summon_teamside then
                        summon_teamside = TeamSide.kOur
                    end
                    character:set_val("teamside", summon_teamside)
                end
        else
                local teamside = character:get_teamside()
                local clusternum = character:get_val("clusternum")

                aicontext.add_character_to_cluster(character, TeamListType.AIContextTeamListType.AITEAM_ORIGIN, teamside, clusternum, false)
                aicontext.add_character_to_cluster(character, TeamListType.AIContextTeamListType.AITEAM_FRIENDS_FOR_CLUSTER, teamside, clusternum, false)

                character:set_team_list_info(TeamListType.CharacterTeamListType.CTEAM_FRIENDS, TeamListType.AIContextTeamListType.AITEAM_ORIGIN, teamside, clusternum, false)
                character:set_team_list_info(TeamListType.CharacterTeamListType.CTEAM_TMP_FRIENDS, TeamListType.AIContextTeamListType.AITEAM_FRIENDS_FOR_CLUSTER, teamside, clusternum, false)
        end
    end
end

local function on_addcaptain(captain)
end

local function add_schedule_task(task)
    if not scheduletasks then
        scheduletasks = {}
    end
    scheduletasks[#scheduletasks + 1] = task
end

local function add_to_delete_covered_caster_cnt(cnt)
    to_delete_caster_cnt = to_delete_caster_cnt + cnt
end

return {
    schedule_execute = schedule_execute,
    on_tonextcluster = on_tonextcluster,
    on_update = on_update,
    on_update_end = on_update_end,
    on_battle_finished = on_battle_finished,
    on_clear = on_clear,
    on_clear_client = on_clear_client,
    on_clear_server = on_clear_server,
    on_to_nextcluster_end = on_to_nextcluster_end,
    on_addcharacter = on_addcharacter,
    on_addcaptain = on_addcaptain,
    add_schedule_task = add_schedule_task,
    on_to_nextcluster_init_cluster = on_to_nextcluster_init_cluster,
    add_to_delete_covered_caster_cnt = add_to_delete_covered_caster_cnt
}