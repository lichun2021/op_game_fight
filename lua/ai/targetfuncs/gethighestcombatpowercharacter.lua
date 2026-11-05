-- gethighestcombatpowercharacter

local calc_charactercombatpower = require "targetfuncs/calccharactercombatpower"

local function get_highestcombatpowercharacter(characterlist, params)
    local thehighestcharacter
    local thehighestcombatpower = -1
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if not g_actionhelper.is_deadaction(a_character:get_currentaction()) and a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            -- local a_combatpower = calc_charactercombatpower(a_character)
            local a_combatpower = a_character:get_val("combatpower")
            if a_combatpower > thehighestcombatpower then
                thehighestcombatpower = a_combatpower
                thehighestcharacter = a_character
            end
        end
    end

    if thehighestcharacter ~= nil then
        return { thehighestcharacter }
    end

    return nil
end

return get_highestcombatpowercharacter