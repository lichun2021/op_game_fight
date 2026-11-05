-- 46 友方中某项属性最低的目标

local thetargethelper

return function(params)
    if params.search_params == nil then return nil end

    thetargethelper = thetargethelper or g_targethelper

    local characterlist = thetargethelper.pick_filteredfriendlist(params)

    local attrs = params.search_params
    local thelargestval = -1
    local thecharacter = nil
    local thelowestval
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local a_val = thetargethelper.get_lowestattrvalue(a_character, attrs)
            if thelowestval == nil or thelowestval > a_val then
                thelowestval = a_val
                thecharacter = a_character
            end
        end
    end

    if thecharacter ~= nil then return {thecharacter} else return nil end
end