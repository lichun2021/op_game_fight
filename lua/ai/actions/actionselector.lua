-- actionselector

local ActionType = require "actions/actiontype"
local AttrType = require "datadefs/attrtype"
local locktarget = require "targetfuncs/locktarget"

local theaicontext = aicontext
local theactionhelper
local theeffecthelper
local theskillhelper
local thetargethelper
local thehostconfig
local thecachehelper

local function is_skillignorecd(skillcastcondition)
    return skillcastcondition == 13 or skillcastcondition == 16 or skillcastcondition == 17 
end

local function is_skillswapwithnormal(skillid)
    if skillid <= 0 then return false end
    local skilldata = g_datahelper.get_skilldata(skillid)
    if not skilldata then return false end
    local skillcastcondition = math.floor(skilldata.conf[10])
    if skillcastcondition ~= 6 then return false end

    local castparams = skilldata.conf[11]
    local paramtype = type(castparams)
    local firstparam
    if paramtype == "table" then
        if #castparams < 1 then return false end
        firstparam = castparams[1]
    elseif paramtype == "number" then
        firstparam = castparams
    end

    return theaicontext.get_randomnumber() < firstparam
end

local function captain_action_selector(character)
    aiactioncontroller.captain_action_selector(character)
end

local function action_selector(character)
    if character:get_roletype() == 2 then
        captain_action_selector(character)
        return
    end

    theactionhelper = theactionhelper or g_actionhelper
    theeffecthelper = theeffecthelper or g_effecthelper

    local currentaction = character:get_currentaction()
    if theactionhelper.is_deadaction(currentaction) then return end

    -- -- 295 clair 
    -- if theeffecthelper.get_eff(295).check_do(character, currentaction) > 0 then return end
    
    if theeffecthelper.get_eff(129).check_do(character, 0) > 0 then return end



    -- if currentaction == ActionType.ATTACK_ACTION and character:has_tag("deadattacking") then
    if character:has_tag("deadattacking") then
        character:erase_tag("deadattacking")
        if character:get_attr(AttrType.HP) > 0 then
            character:set_actionatt(5, ActionType.ATTACK_ACTION) -- oldaction
            theactionhelper.idleaction.enter(character, 1, character)
        else
            character:add_tag("afterdeadattacking")
            theactionhelper.enter_deadaction(character, 1, false)
            character:erase_tag("afterdeadattacking")
        end
        return
    end

    if theeffecthelper.get_eff(295).check_do(character, currentaction) > 0 then return end

    if character:is_needadjusting() then
        -- if character:has_effect(295) and not character:has_effect(298) then
        --     theactionhelper.idleaction.enter(character, 1, character)
        -- else
            theactionhelper.moveaction.enter(character)
        -- end
        return
    end

    if character:has_effect(266) and theeffecthelper.get_eff(266).check_do(character) then
        return
    end

    local skillidindex,attackseqindex,specialskillid,specialskillcastcondition = theactionhelper.pickskill(character)

    if skillidindex == -1 and specialskillid <= 0 then
        theactionhelper.idleaction.enter(character, 1, character)
        return
    end

    local is_no_normal_attack = character:has_effect(115)
    local is_no_skill = character:has_effect(114)
    local is_only_ultra = character:has_effect(124)
    local is_ignore_silence = aiskillcontroller.is_targetignoresilencebyskillindex(character, skillidindex)
    local is_in_confusion = character:has_effect(515)

    if is_ignore_silence then
        is_no_normal_attack = false
        is_no_skill = false
    end

    if is_in_confusion then
        is_no_skill = true
    end

    theskillhelper = theskillhelper or g_skillhelper
    thetargethelper = thetargethelper or g_targethelper

    if skillidindex == 0 then -- normal attack
        if is_no_normal_attack or is_only_ultra then
            if attackseqindex > -1 then
                if character:has_effect(126) then
                    character:set_skillseqindex2(attackseqindex)
                else
                    character:set_skillseqindex(attackseqindex)
                end
                local skillid = theskillhelper.get_skillid_byindex(character, skillidindex)
                character:set_actionatt(1, skillid)
            end

            theactionhelper.idleaction.enter(character, character:get_attr(AttrType.AInterval), character)
            return
        end
    else
        if is_no_skill then
            skillidindex = 0
        elseif is_only_ultra and skillidindex ~= 1 then
            if attackseqindex > -1 then
                if character:has_effect(126) then
                    character:set_skillseqindex2(attackseqindex)
                else
                    character:set_skillseqindex(attackseqindex)
                end
                local skillid = theskillhelper.get_skillid_byindex(character, skillidindex)
                character:set_actionatt(1, skillid)
            end

            theactionhelper.idleaction.enter(character, character:get_attr(AttrType.AInterval), character)
            return
        end
    end

    local skillid = specialskillid
    local skilllevel = 0
    if skillid <= 0 then
        skillid = theskillhelper.get_skillid_byindex(character, skillidindex)
    end

    local is_switchtonormal = false
    local is_forceattackself = false

    if theeffecthelper.get_eff(248).check_do(character, 0) > 0 then
        is_switchtonormal = true
        is_forceattackself = true
    elseif is_skillswapwithnormal(skillid) then
        is_switchtonormal = true
    end

    if is_switchtonormal then
        skillid, skilllevel = character:get_normalskill()
    end

    if skillid <= 0 then
        theactionhelper.idleaction.enter(character, character:get_attr(AttrType.AInterval), character)
        return
    end

    local skilldata = g_datahelper.get_skilldata(skillid)
    if skilldata == nil then 
        theactionhelper.idleaction.enter(character, character:get_attr(AttrType.AInterval), character)
        return
    end

    skilllevel = character:get_skilllevel(skillid)
    local skillrange = skilldata.conf[14]

    local skilltargettype = skilldata.conf[12]
    local realtargettype = thetargethelper.get_realsearchtype(skilltargettype).realtype
    local is_judgeby297 = thetargethelper.is_skilltargetforsingle(skilldata) -- or thetargethelper.is_targettypeforsingle(realtargettype)
    local target = locktarget(character, is_judgeby297)
    character:set_specialskilltarget(nil)
    
    if is_forceattackself then
        target = character
        character:set_currenttarget(target)
    elseif realtargettype == 41 then
        local should_retarget = false
        if character:is_currenttargetconfirmed() then
            local confirmedtarget = character:get_currenttarget()
            if confirmedtarget == nil or theactionhelper.is_deadaction(confirmedtarget:get_currentaction()) then
                should_retarget = true
            end
        end

        if should_retarget then
            --[[
            local targetparams = skilldata.conf[13]
            local range = skillrange
            local params = { source = character, target = target, search_type = skilltargettype, x1 = -range, x2 = range, y1 = -range, y2 = range, z1 = -range, z2 = range, search_params = targetparams, skillcaster = character, is_usingskill = true, skill_id = skillid, hurt_id = 0, skill_level = 0, is_hpratiohtexcludecharacter = false }
            local targets = thetargethelper.search(params)
            --]]
            local targets = thetargethelper.search_by_skill(skillid, character, character, target, true)
            if targets ~= nil and #targets > 0 then
                target = targets[1]
                character:set_currenttarget(target)
                character:set_is_currenttargetconfirmed(true)
            end
        end
    else
        local specialtargettypes = {30,31,32,45,46,47}
        local is_specialtarget = false
        for i=1,#specialtargettypes do
            if specialtargettypes[i] == realtargettype then
                is_specialtarget = true
                break
            end
        end
        if is_specialtarget then
            --[[
            local targetparams = skilldata.conf[13]
            local range = skillrange
            local params = { source = character, target = target, search_type = skilltargettype, x1 = -range, x2 = range, y1 = -range, y2 = range, z1 = -range, z2 = range, search_params = targetparams, skillcaster = character, is_usingskill = true, skill_id = skillid, hurt_id = 0, skill_level = 0, is_hpratiohtexcludecharacter = false }
            local targets = thetargethelper.search(params)
            --]]
            local targets = thetargethelper.search_by_skill(skillid, character, character, target, true)
            if targets ~= nil and #targets > 0 then
                character:set_specialskilltarget(target)
            end
        end
        character:set_currenttarget(target)
    end

    --[[
    -- 13.0 fix
    thehostconfig = thehostconfig or g_hostconfig
    if thehostconfig.beginskill_lazytick > 0 then
        thecachehelper = thecachehelper or g_cachehelper
        local the_beginskill_releasetick = thecachehelper.cache.aibeginskill_releasetick
        if the_beginskill_releasetick and the_beginskill_releasetick == aicontext.get_currenttick() then
            -- print("aidebug: return action selector for begin skill lazytick")
            return
        end
    end
    --]]

    if character:has_tag("skonbegin") and character:get_tag("skonbegin") == 2 then
        return
    end

    if target == nil then
        theactionhelper.idleaction.enter(character, 1, character)
        return
    end

    if aiactioncontroller.distance_to_character(character, target) <= skillrange and aicontext.is_characterinbattlefield(character) then
        local skillid_forskipcd = 0
        local skilllv_forskipcd = 0
        if character:get_actionatt(10) > 0 then -- skill action cd
            local skillcastcondition = skilldata.conf[10]
            if is_skillignorecd(skillcastcondition) then
                character:set_actionatt(10, 0)
            end

            skillid_forskipcd = theeffecthelper.get_eff(240).check_do(character)
            if skillid_forskipcd > 0 then
                skilllv_forskipcd = character:get_skilllevel(skillid_forskipcd)
            end

            if skillid_forskipcd > 0 and skilllv_forskipcd > 0 then
                -- 240效果触发技能优先级比技能释放条件16低
                if specialskillid > 0 and specialskillcastcondition == 16 then
                    skillid_forskipcd = 0
                    skilllv_forskipcd = 0
                end

                character:set_actionatt(10, 0)
            else
                theactionhelper.idleaction.enter(character, 1, character)
                return 
            end
        end

        local target_fordirection = character:get_specialskilltarget()
        if target_fordirection == nil then target_fordirection = target end

        local is_cannot_turn = character:has_effect(182)
        if is_cannot_turn ~= true and character:get_direction() * (target_fordirection:get_attr(AttrType.X) - character:get_attr(AttrType.X)) < 0 then
            theactionhelper.on_action_selector_turntotarget(character)
            return
        end

        if skillid_forskipcd > 0 and skilllv_forskipcd > 0 then
            aieffectcaster.change_special_effect(character, 240, 1)
            theactionhelper.attackaction.force_enter(character, attackseqindex - 1, skillid_forskipcd, skilllv_forskipcd, true, 0, false)
            character:add_tag("nocdattack")
        else
            -- MARK: test hot fix
            theactionhelper.attackaction.enter(character, attackseqindex, skillid, skilllevel, true, 0, false)
            --theactionhelper.idleaction.enter(character, 3600, character)
        end

        if character:has_effect(239) then
            local eff239 = theeffecthelper.get_eff(239)
            local theslipeff = eff239.check_do(character)
            if theslipeff then
                local sliptick = eff239.get_slipticklen(theslipeff)
                if sliptick >= 0 then
                    local slipefforigin = theslipeff:get_origin()
                    theactionhelper.outofcontrolaction.enter(character, sliptick, ActionType.FALL_DOWN_ACTION, slipefforigin)
                    character:set_actionatt(13, 0) -- effect id
                    if character:has_effect(230) then
                        theeffecthelper.get_eff(230).check_do(character)
                    end
                end
            end
        end
    else
        character:set_actionatt(7, skillid) -- next skillid
        -- if character:has_effect(295) and not character:has_effect(298) then
        --     theactionhelper.idleaction.enter(character, 1, character)
        -- else
            theactionhelper.moveaction.enter(character)
        -- end
    end

end -- action_selector

return {
    action_selector = action_selector,
    captain_action_selector = captain_action_selector
}