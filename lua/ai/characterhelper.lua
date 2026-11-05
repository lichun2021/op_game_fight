-- characterhelper

local Attr = require "datadefs/attrtype"
local TeamSide = require "datadefs/teamside"

local thedatahelper
local theeffecthelper

local SexType = 
{
    MALE = 0,
    FEMALE = 1,
    TRANSSEXUAL = 2
}

local function gethpratio(a_character)
    if a_character:get_attr(Attr.HPTop) <= 0 then 
        return 0 
    else
        return a_character:get_attr(Attr.HP) / a_character:get_attr(Attr.HPTop)
    end
end

local function getcurrentfriendlist(a_character)
    return nil
end

local CD_K = g_cddata.CD_K
local function is_cannon(character)
    return character:get_data(CD_K) == 100
end

local locktarget = require "targetfuncs/locktarget"

local function is_enemyteam(teamside)
    return teamside == TeamSide.kEnemy
end

local function is_ourteam(teamside)
    return teamside == TeamSide.kOur
end

local function canbeattackbyskill(character, attacker, skillid, projectile)
    if skillid and projectile then
        thedatahelper = thedatahelper or g_datahelper
        local skilldata = thedatahelper.get_skilldata(skillid)
        if character:has_effect(297) and attacker and skilldata and skilldata.t and skilldata.t == 1 then
            theeffecthelper = theeffecthelper or g_effecthelper
            return theeffecthelper.get_eff(297).is_findable(character, attacker)
        end
    end
    return true
end

local function is_rivalteam_alldead_for_checkultra(character)
    local rival_teamside = TeamSide.kEnemy
    local teamside = character:get_teamside()
    if teamside == TeamSide.kEnemy then 
        rival_teamside = TeamSide.kOur 
    end
    local rival_clusternum = aicontext.get_current_team_clusternum(rival_teamside)
    local rival_alive_cnt,_ = aicontext.get_team_alive_info(rival_teamside, rival_clusternum)
    if rival_alive_cnt <= 0 then
        return true
    end
    return false
end

return {
    sextype = SexType,
    onhostadd = onhostadd,
    gethpratio = gethpratio,
    getcurrentfriendlist = getcurrentfriendlist,
    is_cannon = is_cannon,
    locktarget = locktarget,
    is_enemyteam = is_enemyteam,
    is_ourteam = is_ourteam,
    canbeattackbyskill = canbeattackbyskill,
    is_rivalteam_alldead_for_checkultra = is_rivalteam_alldead_for_checkultra
}
