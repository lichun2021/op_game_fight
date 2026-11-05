-- actionhelper

local deadaction = require "actions/deadaction"
local turnaction = require "actions/turnaction"
local idleaction = require "actions/idleaction"
local attackaction = require "actions/attackaction"
local moveaction = require "actions/moveaction"
local outofcontrolaction = require "actions/outofcontrolaction"

local pickskill = require "actions/pickskill"
local ActionType = require "actions/actiontype"
local ActionAttType = require "actions/actionatttype"
local actionselector = require "actions/actionselector"

local theactioncontroller = aiactioncontroller
local theaieffectcaster = aieffectcaster
local theeffecthelper

local CD_MYID = g_cddata.CD_MYID
local AttrType = require "datadefs/attrtype"

local function is_deadaction(the_action)
    return deadaction.is_deadaction(the_action)
end

local function on_action_selector_turntotarget(character)
    if character:has_effect(293) == true then
        idleaction.enter(character, 1, character)
    else
        turnaction.enter(character)
    end
end

local function on_moveaction_turntotarget(character)
    if character:has_effect(293) == true then
        idleaction.enter(character, 1, character)
    else
        turnaction.enter(character)
    end
end

local theexitactions = {}
theexitactions[ActionType.IDLE_ACTION] = outofcontrolaction.exit
theexitactions[ActionType.IN_HURT_ACTION] = outofcontrolaction.exit
theexitactions[ActionType.STANDUP_ACTION] = outofcontrolaction.exit
theexitactions[ActionType.STRUCK_FLY_AWAY_ACTION] = theactioncontroller.struckflyaction_exit
theexitactions[ActionType.STRUCK_BACKWORD_ACTION] = theactioncontroller.struckbackaction_exit
theexitactions[ActionType.FALL_DOWN_ACTION] = outofcontrolaction.exit
theexitactions[ActionType.STRUCK_HOVER_ACTION] = theactioncontroller.struckhoveraction_exit
theexitactions[ActionType.EFFECT_ACTION] = theactioncontroller.effectaction_exit
theexitactions[ActionType.ATTACK_ACTION] = attackaction.exit
theexitactions[ActionType.CONTROLLED_EFFECT_ACTION] = outofcontrolaction.exit

local function exit_action(character)
    --theactioncontroller.exit_action(character)

    local ca = character:get_currentaction()
    local anexitaction = theexitactions[ca]
    if anexitaction then 
        anexitaction(character)
    end 

    theaieffectcaster.eff279_check_do_exit_action(character, ca)
end

-- 303 effect   -- if character moveing, so trigger effectid
local function ischaractermovedcantrigger(character, movetype)
  theeffecthelper = theeffecthelper or g_effecthelper
  return theeffecthelper.get_eff(303).check_do_triggered_eff(character, movetype)
end


local function did_enter_action(character)
    --theactioncontroller.did_enter_action(character)

    local ca = character:get_currentaction()
    if ca ~= ActionType.EFFECT_ACTION then
        character:set_actionatt(ActionAttType.EFFECT_ID, 0)
    end

    local thecausermyid = character:get_val("actioncausermyid")
    theaieffectcaster.eff264_check_do(character, thecausermyid, ca)
    theaieffectcaster.eff279_check_do_enter_action(character, ca)
    theaieffectcaster.eff281_check_do(character)
    
	-- MARK: 25.0 没有配置过，屏蔽303效果
    -- if ca == 7 or ca == 8 or ca == 12 then
    --   local ctype = 0
    --   if ca == 7 then
    --     ctype = 1
    --   elseif ca == 8 then
    --     ctype = 3
    --   elseif ca == 12 then
    --     ctype = 2
    --   end
    
    --   ischaractermovedcantrigger(character, ctype)
    -- end
    
    if ca == ActionType.ATTACK_ACTION then
        theeffecthelper = theeffecthelper or g_effecthelper
        theeffecthelper.get_eff(309).check_do(character)
    end
end

local function cleanup(character, is_hurteff)
    theactioncontroller.cleanup(character, is_hurteff)
end

local function enter_deadaction(character, deadtype, is_fatal)
    deadaction.enter(character, deadtype, is_fatal)
end

local function force_enter_deadaction(character)
    deadaction.force_enter(character)
end

local function action_selector(character)
    --theactioncontroller.action_selector(character)
    actionselector.action_selector(character)
end

local function captain_action_selector(character)
    actionselector.captain_action_selector(character)
end

local theescstates = {}
theescstates[ActionType.STRUCK_FLY_AWAY_ACTION] = true
theescstates[ActionType.STRUCK_BACKWORD_ACTION] = true
theescstates[ActionType.STRUCK_HOVER_ACTION] = true
theescstates[ActionType.FALL_DOWN_ACTION] = true

local function is_escape_effect_stateanim(character)
    local ca = character:get_currentaction()
    return theescstates[ca] == true or character:has_tag("unactionable")
end

local function is_escape_hurt_result(character)
    local ca = character:get_currentaction()
    return theescstates[ca] == true or character:has_tag("unactionable") or character:has_effect(281)
end


return {
    deadaction = deadaction,
    turnaction = turnaction,
    idleaction = idleaction,
    attackaction = attackaction,
    moveaction = moveaction,
    outofcontrolaction = outofcontrolaction,

    actions = ActionType,
    is_deadaction = is_deadaction,
    on_action_selector_turntotarget = on_action_selector_turntotarget,
    on_moveaction_turntotarget = on_moveaction_turntotarget,
    pickskill = pickskill,
    exit_action = exit_action,
    did_enter_action = did_enter_action,
    cleanup = cleanup,
    enter_deadaction = enter_deadaction,
    force_enter_deadaction = force_enter_deadaction,
    action_selector = action_selector,
    is_escape_effect_stateanim = is_escape_effect_stateanim,
    is_escape_hurt_result = is_escape_hurt_result,
    ischaractermovedcantrigger = ischaractermovedcantrigger,
    captain_action_selector = captain_action_selector
}