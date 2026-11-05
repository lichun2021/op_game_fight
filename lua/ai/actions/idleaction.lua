--

local function enter(character, ticklen, attacker)
    aiactioncontroller.enter_idleaction(character, ticklen, attacker)
end

return {
    enter = enter
}