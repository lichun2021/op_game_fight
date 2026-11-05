-- attackaction
local ActionType = require "actions/actiontype"
local ActionAttType = require "actions/actionatttype"
local AttrType = require "datadefs/attrtype"

local locktarget = require "targetfuncs/locktarget"
local is_targettyperandom = require "targetfuncs/istargettyperandom"

local math_floor = math.floor

local theactioncontroller = aiactioncontroller
local theaicontext = aicontext
local theaieffectcaster = aieffectcaster
local theactionhelper
local theskillhelper
local thedatahelper
local thetargethelper
local theeffecthelper
local thecachehelper

local function is_accumulate_skill(castcondition)
    return castcondition == 7 or castcondition == 15 or castcondition == 18
end

local function is_repeat_skill(castcondition)
    return castcondition == 20
end

local function is_energy_skill(castcondition)
    return castcondition == 19
end

local function is_repeat_skill_has_ending(skillid)
    thecachehelper = thecachehelper or g_cachehelper
    local repeatskillendingcache = thecachehelper.cache.repeatskillending or {}
    thecachehelper.cache.repeatskillending = repeatskillendingcache

    local currentcache = repeatskillendingcache[skillid]
    if currentcache == nil then
        currentcache = {}
        currentcache.has_ending = false
        thedatahelper = thedatahelper or g_datahelper
        theskillhelper = theskillhelper or g_skillhelper
        local skilldata = thedatahelper.get_skilldata(skillid)
        local execs = skilldata.exe
        for i=1,#execs,4 do
            local execid = execs[i]
            local repeattype = execs[i + 3]
            if execid > 0 and repeattype == 2 then
                currentcache.has_ending = true
                --theskillhelper.calculate_exec_length(execid)
                break
            end
        end
        repeatskillendingcache[skillid] = currentcache
    end

    return currentcache.has_ending
end

local function get_repeat_skill_ending_tick(skillid)
    thecachehelper = thecachehelper or g_cachehelper
    local repeatskillendingcache = thecachehelper.cache.repeatskillending or {}
    thecachehelper.cache.repeatskillending = repeatskillendingcache

    local currentcache = repeatskillendingcache[skillid]
    if currentcache == nil then
        currentcache = {}
        repeatskillendingcache[skillid] = currentcache
    end
    local endingtick = currentcache.endingtick
    if not endingtick then
        endingtick = -1
        thedatahelper = thedatahelper or g_datahelper
        theskillhelper = theskillhelper or g_skillhelper
        local skilldata = thedatahelper.get_skilldata(skillid)
        local execs = skilldata.exe
        for i=1,#execs,4 do
            local execid = execs[i]
            local lazytick = execs[i + 2]
            local repeattype = execs[i + 3]
            if execid > 0 and repeattype == 2 then
                local anexectick = lazytick + theskillhelper.calculate_exec_length(execid)
                if anexectick > endingtick then
                    endingtick = anexectick
                end
            end
        end

        currentcache.endingtick = endingtick
    end

    return endingtick
end

local function get_repeatskill_repeattype(skilldata)
    local castparams = skilldata.conf[11]
    if #castparams >= 4 then
        return math_floor(castparams[4])
    end
    return 0
end

local function get_repeatskill_readytick(skilldata)
    local castparams = skilldata.conf[11]
    if #castparams >= 3 then
        return math_floor(castparams[3])
    end
    return 0
end

local function get_repeatskill_basictick(skilldata)
    local castparams = skilldata.conf[11]
    if #castparams >= 2 then
        return math_floor(castparams[2])
    end
    return 0
end

local function force_end_accu(character)
    if character:has_tag("repeatskill") then
        local skillid = character:get_val("currentrepeatskillid")
        if is_repeat_skill_has_ending(skillid) then
            character:erase_tag("repeatskill")
            character:add_tag("repeatskillending")
            local starttick = character:get_localtick() + 1
            local totaltick = get_repeat_skill_ending_tick(skillid)
            character:set_starttick(starttick)
            character:set_totaltick(totaltick)
            return true
        end
    end
    return theactioncontroller.attackaction_forceendaccu(character)
end

local function impl_enter(character, seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra, is_force_enter)
    local currentaction = character:get_currentaction()
    if currentaction == ActionType.ATTACK_ACTION then
        if character:has_tag("attacknobrokenwhenenter") then return end

        if not is_force_enter then
            if is_ultra and character:has_tag("nointerruptbyultra") then return end
            
            local skillactioncd = character:get_attr(AttrType.AInterval)
            if skillactioncd <= 0 then skillactioncd = 2 end
            character:set_actionatt(ActionAttType.SKILL_ACTION_CD, skillactioncd)

            if is_ultra ~= true and character:has_tag("nocdattack") ~= true then return end
        else
            character:add_tag("exitattackbyforceattack")
        end
    end

    if theaicontext:get_is_fighting() ~= true then
        theaicontext:set_is_fighting(true)
    end

    theactionhelper = theactionhelper or g_actionhelper

    theactionhelper.exit_action(character)
    theactionhelper.cleanup(character, false)

    if is_force_enter then
        character:set_actionatt(ActionAttType.OLD_ACTION, currentaction)
        character:set_actionatt(ActionAttType.OLD_TOTAL_TICK, character:get_totaltick())
        character:set_actionatt(ActionAttType.OLD_TICK, character:get_localtick())
    end

    character:set_currentaction(ActionType.ATTACK_ACTION)
    character:set_actioncauser_myid(character:get_myid())

    if seqindex > -1 and is_force_enter ~= true then
        if character:has_effect(126) then
            character:set_skillseqindex2(seqindex)
        else
            character:set_skillseqindex(seqindex)
        end
    end

    character:set_actionatt(ActionAttType.SKILL_ID, skillid)
    character:set_actionatt(ActionAttType.SKILL_LV, skilllevel)
    character:set_actionatt(ActionAttType.YINGZHI, 0)

    thedatahelper = thedatahelper or g_datahelper
    local skilldata = thedatahelper.get_skilldata(skillid)
    if skilldata == nil then
        g_actionhelper.idleaction.enter(character, 1, character)
        return
    end

    local starttick = character:get_localtick() + 1 - tickoffset
    character:set_starttick(starttick)

    theskillhelper = theskillhelper or g_skillhelper
    local totaltick = theskillhelper.calculate_skill_length(skillid)
    character:set_totaltick(totaltick)

    local skillcastcondition = math_floor(skilldata.conf[10])
    if is_accumulate_skill(skillcastcondition) then
        local nextaltaccutick = character:get_nextalternateaccumaxtick(skillid)
        if nextaltaccutick >= 0 then
            character:set_currentalternateaccumaxtick(skillid, nextaltaccutick)
            local accuskilllen = theskillhelper.calculate_accu_skill_length(skillid, nextaltaccutick)
            character:set_totaltick(accuskilllen)
        else
            if character:get_currentalternateaccumaxtick(skillid) >= 0 then
                character:set_currentalternateaccumaxtick(skillid, -1)
            end
        end
    elseif is_repeat_skill(skillcastcondition) then
        local repeatskillfirstroundtick = theskillhelper.calculate_repeat_skill_first_round_tick_length(skillid)
        character:set_totaltick(repeatskillfirstroundtick)

        local repeatskillmaxrepeattick = theskillhelper.get_repeatskillmaxrepeattick(character, skillid)
        character:set_val("currentrepeatskillmaxrepeattick", repeatskillmaxrepeattick)

        if get_repeatskill_repeattype(skilldata) == 1 then -- 使用基础重复执行时间，当施法者暴击时延长
            character:add_tag("extendrepeatoncrit")
            character:add_tag("repeatskillnointerrupt")
        end
    elseif is_energy_skill(skillcastcondition) then
        local repeatskillfirstroundtick = theskillhelper.calculate_energy_skill_first_round_tick_length(skillid)
        character:set_totaltick(repeatskillfirstroundtick)
    end

    local speedup = g_effecthelper.get_eff(251).check_do(character, 100)
    if speedup ~= nil and speedup ~= 0 then
        local attackspeed = character:get_attackspeed()
        attackspeed = attackspeed + speedup
        character:set_attackspeed(attackspeed)
        if attackspeed > 0 then
            totaltick = character:get_totaltick()
            local extendedtick = totaltick * attackspeed / 100
            extendedtick = math_floor(extendedtick)
            if totaltick > 0 and extendedtick <= 0 then
                totaltick = 1
            else
                totaltick = extendedtick
            end
            character:set_totaltick(totaltick)
        end
    end

    if is_ultra then character:add_tag("ultraskill") end
    if skilldata.conf[16] == 0 then character:add_tag("nointerruptbyultra") end
    if skilldata.conf[18] == 0 then character:add_tag("attacknobrokenwhenenter") end

    if is_energy_skill(skillcastcondition) then
        character:set_val("currentenergyskillid", skillid)
        character:set_val("currentenergyskilltick", 0)
        character:add_tag("energyskill")
        character:add_tag("energyskillfirstround")
    elseif is_repeat_skill(skillcastcondition) then
        character:set_val("currentrepeatskillid", skillid)
        character:set_val("currentrepeatskilltick", 0)
        character:add_tag("repeatskill")
        character:add_tag("repeatskillfirstround")
        character:add_tag("repeatskillfirstbasicround")

        aicontext.add_event_listener("castultra", character)
    end

    g_markhelper.execute_effect(character, skillid, skilllevel, -1, -1, 0, 0, 0, nil)

    character:on_startcastskill(skillid)

    if g_targethelper.get_realsearchtype(skilldata.conf[12]) == 26 and is_force_enter ~= true then
        character:set_val("flagfirstfound26target", true)
        character:set_val("firstroundconfirm26target", nil)
    end

    theactionhelper.did_enter_action(character)
end

local function force_enter(character, seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra)
    aiactioncontroller.attackaction_force_enter(character, skillseq, skillid, skilllevel, is_resetcd, tickoffset, is_ultra)
    -- impl_enter(character, seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra, true)
end

local function enter(character, seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra)
    -- if theactioncontroller.attackaction_trigger_combo(character, skillid, skilllevel) then return end
    aiactioncontroller.attackaction_enter(character, seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra)
    -- impl_enter(character, seqindex, skillid, skilllevel, is_resetcd, tickoffset, is_ultra, false)
end

local function update_repeatskillending(character, localtick, starttick, totaltick, skillid, skilllevel)
    if localtick < starttick then return end
    theskillhelper = theskillhelper or g_skillhelper
    theskillhelper.execute_repeatending_execidintick(skillid, skilllevel, localtick - starttick, character, character:get_currenttarget())

    if localtick > starttick + totaltick then
        if character:has_tag("repeatskillnotinterruptbeforeending") then
            character:add_tag("attacknotinterrupt")
        end
        theactionhelper = theactionhelper or g_actionhelper
        theactionhelper.idleaction.enter(character, 1, character)
    end
end

local function update(character)
    local localtick = character:get_localtick()
    local starttick = character:get_starttick()
    local totaltick = character:get_totaltick()
    if localtick < starttick then return end

    local yingzhi = character:get_actionatt(ActionAttType.YINGZHI)
    if yingzhi > 0 then
        character:set_actionatt(ActionAttType.YINGZHI, yingzhi - 1)
    end

    theactionhelper = theactionhelper or g_actionhelper
    local theidleaction = theactionhelper.idleaction

    -- if character:has_tag("repeatskill") and not character:has_tag("repeatskillfirstbasicround") and not character:has_tag("repeatskillnointerrupt") and character:has_tag("repeatskillinterrupt") then
    --     theidleaction.enter(character, 1, character)
    --     return
    -- end

    local skillid = character:get_actionatt(ActionAttType.SKILL_ID)
    local skilllevel = character:get_actionatt(ActionAttType.SKILL_LV)
    theskillhelper = theskillhelper or g_skillhelper

    if character:has_tag("repeatskill") and not character:has_tag("repeatskillfirstbasicround") and not character:has_tag("repeatskillnointerrupt") and character:has_tag("repeatskillinterrupt") then
        if not is_repeat_skill_has_ending(skillid) then
            theidleaction.enter(character, 1, character)
            return
        else
            character:erase_tag("repeatskill")
            character:add_tag("repeatskillending")
            starttick = localtick + 1
            totaltick = get_repeat_skill_ending_tick(skillid)
            character:set_starttick(starttick)
            character:set_totaltick(totaltick)
        end
    end

    if character:has_tag("repeatskillending") then
        update_repeatskillending(character, localtick, starttick, totaltick, skillid, skilllevel)
        return 
    end

    local deltatick = localtick - starttick
    theskillhelper.execute_execidintick(skillid, skilllevel, deltatick, character, character:get_currenttarget())

    if character:has_tag("attacknobrokenwhenenter") then
        character:erase_tag("attacknobrokenwhenenter")
    end

    thedatahelper = thedatahelper or g_datahelper
    if character:has_tag("energyskill") then
        local currentenergyskillid = character:get_val("currentenergyskillid")
        local skilldata = thedatahelper.get_skilldata(currentenergyskillid)
        local maxtick = totaltick
        if skilldata ~= nil then
            if character:has_tag("energyskillfirstround") ~= true then
                local energy = character:get_attr(AttrType.Energy)
                local endelta = -skilldata.conf[5]
                if character:has_effect(315) then
                    endelta = endelta - g_effecthelper.get_eff(315).trigger(character, endelta)
                    if endelta > 0 then endelta = 0 end
                end
                character:update_energy(energy + endelta) -- consumeEn
            end
            local castparams = skilldata.conf[11]
            if #castparams > 0 then
                maxtick = castparams[1]
            end
        end

        if character:has_tag("energyskillfirstround") ~= true then
            local cest = character:get_val("currentenergyskilltick")
            cest = cest + 1
            character:set_val("currentenergyskilltick", cest)
            if character:get_attr(AttrType.Energy) <= 0 or cest > maxtick then
                character:add_tag("attacknotinterrupt")
                theidleaction.enter(character, 1, character)
                return
            end
        end
    elseif character:has_tag("repeatskill") then
        local crst = character:get_val("currentrepeatskilltick")
        if localtick - starttick < totaltick then
            crst = crst + 1
            character:set_val("currentrepeatskilltick", crst)
        end
        local currentrepeatskillid = character:get_val("currentrepeatskillid")
        local skilldata = thedatahelper.get_skilldata(currentrepeatskillid)
        local readytick = get_repeatskill_readytick(skilldata)
        local maxtick = character:get_val("currentrepeatskillmaxrepeattick")
        if crst > maxtick + readytick then
            if not is_repeat_skill_has_ending(skillid) then
                character:add_tag("attacknotinterrupt")
                theidleaction.enter(character, 1, character)
            else
                character:erase_tag("repeatskill")
                character:add_tag("repeatskillending")
                character:add_tag("repeatskillnotinterruptbeforeending")
                starttick = localtick + 1
                totaltick = get_repeat_skill_ending_tick(skillid)
                character:set_starttick(starttick)
                character:set_totaltick(totaltick)
            end

            return
        end

        if get_repeatskill_repeattype(skilldata) == 2 then -- 使用基础重复执行时间，当施法者拥有某技能时延长
            local basictick = get_repeatskill_basictick(skilldata)
            if crst > basictick + readytick and character:has_tag("repeatskillfirstbasicround") then
                character:erase_tag("repeatskillfirstbasicround")
            end
        end
    end

    if localtick > starttick + totaltick then
        local skilldata = thedatahelper.get_skilldata(skillid)
        local targettype = skilldata.conf[12]
        local the_get_realsearchtype = g_targethelper.get_realsearchtype
        local realtype = the_get_realsearchtype(targettype).realtype
        if skilldata ~= nil and realtype == 41 then
            character:set_val("mattackconfirmedtarget", false)
        elseif skilldata ~= nil and realtype == 26 then
            character:set_val("flagfirstfound26target", false)
        end

        if character:has_tag("repeatattacking") then
            local repeatcnt = character:get_actionatt(ActionAttType.ACCU_ATTACK_REPEAT)
            repeatcnt = repeatcnt - 2
            if repeatcnt > 0 then
                character:set_starttick(localtick + 1)
                character:set_totaltick(theskillhelper.calculate_accumulate_skill_release_length(skillid))
                character:set_actionatt(ActionAttType.ACCU_ATTACK_REPEAT, repeatcnt)
                return
            end
        end

        if character:has_tag("energyskill") then
            character:set_starttick(localtick + 1)
            if character:has_tag("energyskillfirstround") then
                character:erase_tag("energyskillfirstround")
                character:set_totaltick(theskillhelper.calculate_accumulate_skill_release_length(skillid))
            end
            return
        elseif character:has_tag("repeatskill") then
            character:set_starttick(localtick + 1)
            character:set_totaltick(theskillhelper.calculate_accumulate_skill_release_length(skillid))
            if character:has_tag("repeatskillfirstround") then
                character:erase_tag("repeatskillfirstround")
            end
            return
        end

        character:add_tag("attacknotinterrupt")
        theactionhelper.action_selector(character)
    end
end

local function cast_ultra(character, is_external)
    theactionhelper = theactionhelper or g_actionhelper
    local thecharacterhelper = g_characterhelper
    theskillhelper = theskillhelper or g_skillhelper
    local currentaction = character:get_currentaction()
    local teamside = character:get_teamside()
    if theactionhelper.is_deadaction(currentaction) or currentaction == ActionType.EFFECT_ACTION or theaicontext:get_is_fighting() ~= true or character:get_attr(AttrType.HP) <= 0 then
        return -1
    end

    --[[
    if thecharacterhelper.is_enemyteam(teamside) then
        local friendnum1, friendnum2, friendnum3 = theaicontext.get_friend_alive_nums()
        if friendnum1 + friendnum2 + friendnum3 < 1 then return -1 end
    elseif thecharacterhelper.is_ourteam(teamside) then
        local rivalnum1, rivalnum2, rivalnum3 = theaicontext.get_rival_alive_nums()
        if rivalnum1 + rivalnum2 + rivalnum3 < 1 then return -1 end
    end
    --]]

    if thecharacterhelper.is_rivalteam_alldead_for_checkultra(character) then
        return -1
    end

    thedatahelper = thedatahelper or g_datahelper
    local characterdata = thedatahelper.get_characterdata(character:get_attr(AttrType.ID))
    local ultraskillid = characterdata.sk[2]
    if theskillhelper.is_targetignoresilencebyskillid(character, ultraskillid) ~= true then
        if character:has_effect(114) then return -1 end
    end

    if character:has_effect(137) then return -1 end

    theeffecthelper = theeffecthelper or g_effecthelper
    if theeffecthelper.get_eff(248).check_do(character, 0) > 0 then return -1 end

    if currentaction == ActionType.ATTACK_ACTION and character:has_tag("nointerruptbyultra") then return -1 end

    local cancastultra = character:get_val("canultraskill")
    if cancastultra == 2 then
        local jxlv = character:get_val("juexinglevel")
        ultraskillid = characterdata.jxj[3 * jxlv + 1]
    end

    local skilllist = character:get_skill_list()
    if #skilllist < 1 then return -1 end

    local has_ultraskill = false
    local ultraskilllevel = 0
    for i=1, #skilllist do
        local skill = skilllist[i]
        if ultraskillid == skill[1] then
            has_ultraskill = true
            ultraskilllevel = skill[2]
            break
        end
    end

    if has_ultraskill ~= true then
        return 0
    end

    if character:has_effect(294) and theeffecthelper.get_eff(294).is_skillidblocked(character, ultraskillid) then
        return -1
    end

    local skilldata = thedatahelper.get_skilldata(ultraskillid)
    if skilldata == nil then return 0 end

    local charactermyid = character:get_myid()

    local freezetime = skilldata.conf[9]
    local ultralock = character:get_val("ifultralock")
    if freezetime > 0 and ultralock > 0 then
        -- OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezeTime, teamSide
        local ultrafreeze = {0,0,0,0,0,0,0,0,0,0}
        ultrafreeze[1] = freezetime
        ultrafreeze[2] = theskillhelper.calculate_skill_length(ultraskillid)
        ultrafreeze[3] = charactermyid
        ultrafreeze[4] = character:get_val("clusternum")
        ultrafreeze[5] = theaicontext.get_currenttick()
        ultrafreeze[6] = freezetime
        ultrafreeze[7] = teamside

        ultrafreeze[8] = skilldata.conf[19]
        ultrafreeze[9] = skilldata.conf[20]
        ultrafreeze[10] = skilldata.conf[21]

        theaicontext.pushfront_ultrafreeze(ultrafreeze)
    end

    local rettype = cancastultra

    local skillcastcondition = math_floor(skilldata.conf[10])
    if is_accumulate_skill(skillcastcondition) then
        if rettype == 1 then -- 能释放普通大招
            rettype = 3 -- 能释放普通蓄力大招
        elseif rettype == 2 then -- 能释放觉醒技
            rettype = 4 -- 能释放蓄力觉醒技
        end
    elseif skillcastcondition == 19 then -- 持续消耗能量型技能
        if rettype == 1 then
            rettype = 5
        elseif rettype == 2 then
            rettype = 6
        end
    end

    character:set_val("canultraskill", 0)

    if is_energy_skill(skillcastcondition) ~= true then
        local energy = character:get_attr(AttrType.Energy)
        local systemconfig = thedatahelper.get_systemconfig()
        local energymax = systemconfig.max_energy
        if energy > energymax then energy = energymax end

        local endelta = -skilldata.conf[5]
        -- if character:has_effect(315) then
        --     endelta = endelta - g_effecthelper.get_eff(315).trigger(character, endelta)
        --     if endelta > 0 then endelta = 0 end
        -- end
        character:update_energy(energy + endelta)
    end

    thetargethelper = thetargethelper or g_targethelper
    local skilltargettype = skilldata.conf[12]
    local realtargettype = thetargethelper.get_realsearchtype(skilltargettype).realtype
    if realtargettype == 30 or realtargettype == 31 or realtargettype == 32 or realtargettype == 45 or realtargettype == 46 or realtargettype == 47 then
        --[[
        local skillrange = skilldata.conf[14]

        local params = {}
        params.source = character
        params.target = character:get_currenttarget()
        params.search_type = skilltargettype
        params.x1 = -skillrange
        params.x2 = skillrange
        params.y1 = -skillrange
        params.y2 = skillrange
        params.z1 = -skillrange
        params.z2 = skillrange
        params.search_params = skilldata.conf[13]
        params.skillcaster = character
        params.is_usingskill = true
        params.skill_id = ultraskillid
        params.skill_level = ultraskilllevel
        params.is_hpratiohtexcludecharacter = false
        local targets = thetargethelper.search(params)
        --]]
        local targets = thetargethelper.search_by_skill(ultraskillid, character, character, character:get_currenttarget(), true)
        if targets and #targets > 0 then
            local ultratarget = targets[1]
            character:forceturntotarget(ultratarget)
        end
    end

    enter(character, -1, ultraskillid, ultraskilllevel, true, 0, true)

    if is_external then
        theaicontext.add_trackultra(theaicontext.get_currenttick(), charactermyid)
    end

    local theaicallback = theaicontext.get_aicallback()
    if theaicallback ~= nil then
        local postertype = 0
        if character:get_tag("posternone") > 0 then
            postertype = 2
        elseif character:get_tag("posterall") > 0 then
            postertype = 1
        end
        local posternum = character:get_tag("posternum")

        theaicallback:character_did_cast_ultra_skill(charactermyid, character:get_attr(AttrType.ID), teamside, rettype, postertype, posternum)
    end

    theaicontext.on_event("castultra", character)

    return rettype
end

-- jx:觉醒等级 0-普通大招 >0-觉醒大招
-- return:
local function force_cast_ultra(character, jx)
    thedatahelper = thedatahelper or g_datahelper
    local rettype = 1
    local characterdata = thedatahelper.get_characterdata(character:get_attr(AttrType.ID))
    local ultraskillid = characterdata.sk[2]
    if jx > 0 then
        ultraskillid = characterdata.jxj[3 * jx + 1]
        rettype = 2
    end

    local skilllist = character:get_skill_list()

    local ultraskilllevel = 1
    for i=1, #skilllist do
        local skill = skilllist[i]
        if ultraskillid == skill[1] then
            ultraskilllevel = skill[2]
            break
        end
    end

    local skilldata = thedatahelper.get_skilldata(ultraskillid)
    if skilldata == nil then return 0 end

    local charactermyid = character:get_myid()
    local teamside = character:get_teamside()

    local freezetime = skilldata.conf[9]
    local ultralock = character:get_val("ifultralock")
    if freezetime > 0 and ultralock > 0 then
        -- OpCountdown, EffCountdown, myID, myCluster, startGlobalTick, freezeTime, teamSide
        local ultrafreeze = {0,0,0,0,0,0,0,0,0,0}
        ultrafreeze[1] = freezetime
        ultrafreeze[2] = g_skillhelper.calculate_skill_length(ultraskillid)
        ultrafreeze[3] = charactermyid
        ultrafreeze[4] = character:get_val("clusternum")
        ultrafreeze[5] = theaicontext.get_currenttick()
        ultrafreeze[6] = freezetime
        ultrafreeze[7] = teamside

        ultrafreeze[8] =  skilldata.conf[19]
        ultrafreeze[9] = skilldata.conf[20]
        ultrafreeze[10] = skilldata.conf[21]

        theaicontext.pushfront_ultrafreeze(ultrafreeze)
    end

    local skillcastcondition = math_floor(skilldata.conf[10])
    if is_accumulate_skill(skillcastcondition) then
        if rettype == 1 then -- 能释放普通大招
            rettype = 3 -- 能释放普通蓄力大招
        elseif rettype == 2 then -- 能释放觉醒技
            rettype = 4 -- 能释放蓄力觉醒技
        end
    elseif skillcastcondition == 19 then -- 持续消耗能量型技能
        if rettype == 1 then
            rettype = 5
        elseif rettype == 2 then
            rettype = 6
        end
    end

    character:set_val("canultraskill", 0)

    if is_energy_skill(skillcastcondition) ~= true then
        local energy = character:get_attr(AttrType.Energy)
        local systemconfig = thedatahelper.get_systemconfig()
        local energymax = systemconfig.max_energy
        if energy > energymax then energy = energymax end

        local endelta = -skilldata.conf[5]
        if character:has_effect(315) then
            endelta = endelta - g_effecthelper.get_eff(315).trigger(character, endelta)
            if endelta > 0 then endelta = 0 end
        end
        character:update_energy(energy + endelta)
    end

    local currenttarget = character:get_currenttarget()
    if currenttarget == nil then
        local targets = g_targethelper.search_by_skill(ultraskillid, character, character, currenttarget, true)
        if targets and #targets > 0 then
            local ultratarget = targets[1]
            character:set_currenttarget(ultratarget)
            character:forceturntotarget(ultratarget)
            currenttarget = ultratarget
        end
    end

    enter(character, -1, ultraskillid, ultraskilllevel, true, 0, true)

    theaicontext.add_trackultra(theaicontext.get_currenttick(), charactermyid)

    local theaicallback = theaicontext.get_aicallback()
    if theaicallback ~= nil then
        local postertype = 0
        if character:get_tag("posternone") > 0 then
            postertype = 2
        elseif character:get_tag("posterall") > 0 then
            postertype = 1
        end
        local posternum = character:get_tag("posternum")

        theaicallback:character_did_cast_ultra_skill(charactermyid, character:get_attr(AttrType.ID), teamside, rettype, postertype, posternum)
    end

    theaicontext.on_event("castultra", character)

    return rettype
end

local function check_target(character, canultraskill)
    local currenttarget = character:get_currenttarget()
    if not currenttarget then return 0 end

    thedatahelper = thedatahelper or g_datahelper
    thetargethelper = thetargethelper or g_targethelper
    theeffecthelper = theeffecthelper or g_effecthelper
    
    local characterdata = thedatahelper.get_characterdata(character:get_attr(AttrType.ID))
    local ultraskillid = math.floor(characterdata.sk[2])
    if canultraskill == 2 then
        local jxlv = character:get_val("juexinglevel")
        ultraskillid = characterdata.jxj[3 * jxlv + 1]
    end

    local skilldata = thedatahelper.get_skilldata(ultraskillid)
    if not skilldata then return 0 end

    if character:has_effect(294) and theeffecthelper.get_eff(294).is_skillidblocked(character, ultraskillid) then
        return -1
    end

    if thetargethelper.is_skilltargetforsingle(skilldata) and currenttarget:has_effect(297) then
        if not theeffecthelper.get_eff(297).is_findable(currenttarget, character) then
            local anothertarget = locktarget(character, true)
            if anothertarget then
                character:set_currenttarget(anothertarget)
                currenttarget = anothertarget
            else
                -- character:set_currenttarget(nil)
                return -1
            end
        end
    end

    local skilltargettype = skilldata.conf[12]
    local realtargettype = thetargethelper.get_realsearchtype(skilltargettype).realtype

    if realtargettype == 1 and theactionhelper.is_deadaction(currenttarget:get_currentaction()) then
        local anothertarget = locktarget(character)
        if anothertarget and anothertarget:get_myid() ~= currenttarget:get_myid() then
            character:set_currenttarget(anothertarget)
            currenttarget = anothertarget
        else
            character:set_currenttarget(nil)
            return 0
        end
    end

    if not is_targettyperandom(realtargettype) then
        --[[
        local skillrange = skilldata.conf[14]
        local x1,x2,y1,y2,z1,z2 = -skillrange, skillrange, -skillrange, skillrange, -skillrange, skillrange
        local search_params = skilldata.conf[13]
        local targets = thetargethelper.search_with(character, character, currenttarget, nil, skilltargettype, x1, x2, y1, y2, z1, z2, search_params, false, ultraskillid, 0, 0, nil, nil, false)
        --]]
        local targets = thetargethelper.search_by_skill(ultraskillid, character, character, currenttarget, false)
        if not targets or #targets < 1 then
            canultraskill = 0
        end
    else
        local allenemydied = false
        if realtargettype == 7 or realtargettype == 25 or realtargettype == 23 or realtargettype == 26 or realtargettype == 41 then
            allenemydied = true
            local tmprivals = character:get_tmp_rivals()
            if tmprivals then
                for i=1,#tmprivals do
                    local arival = tmprivals[i]
                    if arival and not theactionhelper.is_deadaction(arival:get_currentaction()) then
                        allenemydied = false
                        break
                    end
                end
            end
        end

        if not allenemydied and theaicontext.get_adjustingdistance() <= 0 then
            return canultraskill
        else
            return 0
        end
    end

    return canultraskill
end

local function can_cast_ultra(character, is_external)
    local canultraskill = character:get_val("canultraskill")
    if canultraskill <= 0 then return 0 end

    theactionhelper = theactionhelper or g_actionhelper
    local thecharacterhelper = g_characterhelper
    theskillhelper = theskillhelper or g_skillhelper
    local currentaction = character:get_currentaction()
    local teamside = character:get_teamside()

    if theactionhelper.is_deadaction(currentaction) or currentaction == ActionType.EFFECT_ACTION or not theaicontext:get_is_fighting() or character:get_attr(AttrType.HP) <= 0 then
        return -1
    end

    --[[
    if thecharacterhelper.is_enemyteam(teamside) then
        local friendnum1, friendnum2, friendnum3 = theaicontext.get_friend_alive_nums()
        if friendnum1 + friendnum2 + friendnum3 < 1 then return -1 end
    elseif thecharacterhelper.is_ourteam(teamside) then
        local rivalnum1, rivalnum2, rivalnum3 = theaicontext.get_rival_alive_nums()
        if rivalnum1 + rivalnum2 + rivalnum3 < 1 then return -1 end
    end
    --]]

    if thecharacterhelper.is_rivalteam_alldead_for_checkultra(character) then
        return -1
    end

    local skilllist = character:get_skill_list()
    if #skilllist < 2 then return -1 end
    if skilllist[2][1] < 1 then return -1 end

    if character:has_tag("energyskill") then return -1 end
    if character:has_tag("repeatskill") then return -1 end
    if character:has_tag("besuppressed") then return -1 end
    if currentaction == ActionType.ATTACK_ACTION and character:has_tag("nointerruptbyultra") then return -1 end
    if character:has_effect(137) then return -1 end

    theeffecthelper = theeffecthelper or g_effecthelper
    if character:has_effect(248) and theeffecthelper.get_eff(248).check_do(character, 0) > 0 then return -1 end

    thedatahelper = thedatahelper or g_datahelper
    local characterdata = thedatahelper.get_characterdata(character:get_attr(AttrType.ID))
    local ultraskillid = math.floor(characterdata.sk[2])

    if theskillhelper.is_targetignoresilencebyskillid(character, ultraskillid) ~= true then
        if character:has_effect(114) then return -1 end
    end

    local effectid = character:get_actionatt(ActionAttType.EFFECT_ID)
    if effectid > 0 then
        local effectdata = thedatahelper.get_effectdata(effectid)
        if effectdata and effectdata.conf[9] > 0 then return -1 end -- stateanimation
    end
    
    if character:has_tag("unactionable") then
        return -1
    end

    if not theaicontext.is_characterinbattlefield(character) then return -1 end

    local rettype = check_target(character, canultraskill)
    if rettype > 0 then
        if rettype == 2 then
            local jxlv = character:get_val("juexinglevel")
            ultraskillid = characterdata.jxj[3 * jxlv + 1]
        end

        local currenttarget = character:get_currenttarget()
        local skilldata = thedatahelper.get_skilldata(ultraskillid)
        local skillrange = skilldata.conf[14]
        if currenttarget and theactioncontroller.distance_to_character(character, currenttarget) > skillrange then
            return -1
        end

        if character:get_attr(AttrType.Energy) < skilldata.conf[4] then
            --character:set_val("canultraskill", 0)
            return -1
        end

        local skillcastcondition = math_floor(skilldata.conf[10])
        if is_accumulate_skill(skillcastcondition) then
            if rettype == 1 then -- 能释放普通大招
                rettype = 3 -- 能释放普通蓄力大招
            elseif rettype == 2 then -- 能释放觉醒技
                rettype = 4 -- 能释放蓄力觉醒技
            end
        end
    end

    return rettype
end

local function on_exit(character)
    theactioncontroller.attackaction_exit(character)
    character:erase_tag("repeatskillending")
    character:erase_tag("repeatskillnotinterruptbeforeending")
end

local function exit(character)
	if character:has_effect(328) then
		theeffecthelper = theeffecthelper or g_effecthelper
		local skid, sklv, sktype, isforce = character:get_attackactioninfo()
		theeffecthelper.get_eff(328).trigger(character, skid, sklv, sktype, isforce, 1)
	end
    theactioncontroller.attackaction_exit(character)
    character:erase_tag("repeatskillending")
    character:erase_tag("repeatskillnotinterruptbeforeending")
    if character:has_tag("exitattackbyforceattack") then
        character:erase_tag("exitattackbyforceattack")
    else
        if character:get_tag("foskeffcnt") > 0 then
            g_aihelper.add_schedule_task(function() theaieffectcaster.kill_follow_skill_tick_effect(character) end)
        end
    end
end

return {
    force_end_accu = force_end_accu,
    impl_enter = impl_enter,
    force_enter = force_enter,
    enter = enter,
    update = update,
    cast_ultra = cast_ultra,
    can_cast_ultra = can_cast_ultra,
    check_target = check_target,
    on_exit = on_exit,
    exit = exit,
    force_cast_ultra = force_cast_ultra
}
