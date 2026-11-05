-- deadaction
local ActionType = require "actions/actiontype"
local AttrType = require "datadefs/attrtype"
local TeamSide = require "datadefs/teamside"
local summoncaster = require "casters/summoncaster"

local theactionhelper

local function is_deadaction(action)
    return action == ActionType.DEAD_ACTION1 or action == ActionType.DEAD_ACTION2 or action == ActionType.DEAD_ACTION3
end

local function adjust_teampos(character, is_revive)
    aiactioncontroller.deadaction_adjust_teampos(character, is_revive)
end

local function is_enemy(character)
    return character:get_teamside() == TeamSide.kEnemy
end

local function is_our_hero(character)
    return character:get_teamside() == TeamSide.kOur
end

local function claim_monster_dead_reward(character)
    aiactioncontroller.deadaction_claim_monster_dead_reward(character)
end

local function do_hero_dead(character)
    aiactioncontroller.deadaction_do_hero_dead(character)
end

local function implenter(character, deadtype, is_fatal)
    theactionhelper = theactionhelper or g_actionhelper
    local currentaction = character:get_currentaction()
    if currentaction == ActionType.ATTACK_ACTION then
        if character:has_tag("deadattacking") then return end
        if character:is_in_accu_state() then
            local skillid = character:get_actionatt(1)
            local skilldata = g_datahelper.get_skilldata(skillid)
            --if (skdata && eSkillCastConditionAccumulatePlus == skdata->castCondition)
            if skilldata ~= nil and skilldata.conf[10] == 15 then
                theactionhelper.attackaction.force_end_accu(character)
            end
        end

        -- skillcd
        character:set_actionatt(10, character:get_attr(AttrType.AInterval))
    end

    if is_deadaction(currentaction) then return end

    if g_effecthelper.get_eff(213).check_do(character) ~= true then
        if g_effecthelper.get_eff(103).check_do(character, 0) > 0 then
            return
        end 
    end

    if character:has_effect(296) ~= true and character:has_tag("afterdeadattacking") ~= true then
        local deadskill = character:get_special_skill(2)
        if deadskill ~= nil and deadskill:can_cast(character) then
            local skillid = deadskill:get_skillid()
            local is_deadskill_blocked = false
            if character:has_effect(294) then
                if g_effecthelper.get_eff(294).is_skillidblocked(character, skillid) then
                    is_deadskill_blocked = true
                end
            end
            
            if is_deadskill_blocked ~= true then
                local characterdata = g_datahelper.get_characterdata(character:get_attr(AttrType.ID))
                local is_ultra = skillid == characterdata.sk[2]
                character:add_tag("deadattacking")
                g_effecthelper.clean_eff(character, 251) -- MARK: hotfix
                theactionhelper.attackaction.force_enter(character, -1, skillid, deadskill:get_skilllevel(), true, 0, is_ultra)
                return
            end
        end
    end

    theactionhelper.exit_action(character)
    theactionhelper.cleanup(character, false)
    local theactiontype = ActionType.DEAD_ACTION3
    if deadtype == 1 then theactiontype = ActionType.DEAD_ACTION1
    elseif deadtype == 2 then theactiontype = ActionType.DEAD_ACTION2
    end
    character:set_currentaction(theactiontype)
    local killermyid = character:get_killer_myid()
    character:set_actioncauser_myid(killermyid)
    character:update_hp(0)
    character:set_starttick(character:get_localtick() + 1)
    character:set_totaltick(40)

    character:exit_duel()

    adjust_teampos(character, false)

    if is_enemy(character) and character:has_tag("revived") ~= true then
        claim_monster_dead_reward(character)
    end

    if is_our_hero(character) and character:has_tag("revived") ~= true then
        do_hero_dead(character)
    end

    -- character:marks_on_selfenterdead()

    -- aicontext.trigger_marks_on_character_dead()
    -- aicontext.remove_marks_on_character_dead(character)

    aimarkcaster.check_do_character_dead(character)

    if character:is_summon() then
        character:add_tag("summonkilled")
        summoncaster.check_execute_deadrun(character, 1)
        summoncaster.check_execute_deadrun(character, 3)
        summoncaster.check_execute_deadrun(character, 4)
    else
        local killer = aicontext.get_character_by_myid(killermyid)
        local charactermyid = character:get_myid()
        if killer ~= nil and killermyid ~= charactermyid and killer:get_teamside() ~= character:get_teamside() then
            local systemconfig = g_datahelper.get_systemconfig()
            local endelta = systemconfig.get_energy
            local energy = killer:get_attr(AttrType.Energy)
            if killer:has_effect(315) then
                endelta = endelta - g_effecthelper.get_eff(315).trigger(killer, endelta)
                if endelta < 0 then endelta = 0 end
            end

            killer:update_energy(energy + endelta,true)

            local energyadd = killer:get_attr(AttrType.Energy) - energy

            if energyadd ~= 0 then
                g_hitlabel.execute(100040, energyadd, killer)
            end
        end
    end

    aicontext.on_event("herodead250", character)

    local friends = character:get_currentfriendlist()
    for i=1,#friends do
        local afriend = friends[i]
        afriend:on_frienddead(character)
    end

    theactionhelper.did_enter_action(character)
end

local function impl_force_enter(character)
    local currentaction = character:get_currentaction()
    if currentaction == ActionType.ATTACK_ACTION then
        -- skillcd
        character:set_actionatt(10, character:get_attr(AttrType.AInterval))
    end

    if is_deadaction(currentaction) then return end

    g_actionhelper.exit_action(character)
    g_actionhelper.cleanup(character, false)
    character:set_currentaction(ActionType.DEAD_ACTION1)
    local killermyid = character:get_killer_myid()
    character:set_actioncauser_myid(killermyid)
    character:update_hp(0)
    character:set_starttick(character:get_localtick() + 1)
    character:set_totaltick(40)

    character:exit_duel()

    if character:is_summon() then
        character:add_tag("summonkilled")
    else
        adjust_teampos(character, false)
    end

    if is_enemy(character) and character:has_tag("revived") ~= true then
        claim_monster_dead_reward(character)
    end

    if is_our_hero(character) and character:has_tag("revived") ~= true then
        do_hero_dead(character)
    end

    --aicontext.trigger_marks_on_character_dead()
    aimarkcaster.check_trigger_on_character_dead(character)
    aicontext.on_event("herodead250", character)

    local friends = character:get_currentfriendlist()
    for i=1,#friends do
        local afriend = friends[i]
        afriend:on_frienddead(character)
    end

    g_actionhelper.did_enter_action(character)
end

local function enter(target, deadtype, is_fatalhit)
    -- aiactioncontroller.enter_deadaction(target, deadtype, is_fatalhit)
    implenter(target, deadtype, is_fatalhit)
end

local function force_enter(character)
    -- aiactioncontroller.force_enter_deadaction(character)
    impl_force_enter(character)
end

return {
    enter = enter,
    is_deadaction = is_deadaction,
    force_enter = force_enter
}