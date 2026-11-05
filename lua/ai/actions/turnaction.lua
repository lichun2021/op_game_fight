-- turnaction

local function enter(character)
    aiactioncontroller.enter_turnaction(character)
end

return {
    enter = enter
}