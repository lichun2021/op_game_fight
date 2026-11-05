-- outofcontrolaction

local theaiactioncontroller = aiactioncontroller


local function enter(character, tick, actiontype, cause_character)
    return theaiactioncontroller.outofcontrolaction_enter(character, tick, actiontype, cause_character)
end

local function on_exit(character)
    character:erase_tag("hurtresultstack")
    character:erase_tag("unactionable")
    character:erase_tag("besuppressed")
end

local function exit(character)
    character:erase_tag("hurtresultstack")
    character:erase_tag("unactionable")
    character:erase_tag("besuppressed")
end

return {
    enter = enter,
    on_exit = on_exit,
    exit = exit
}