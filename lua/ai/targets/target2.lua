-- 2 - 本方所有
return function(params)
    local characterlist = g_targethelper.pick_filteredfriendlist(params)

    local retlist = {}

    for i=1,#characterlist do 
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            -- table.insert( retlist, a_character )
            retlist[#retlist + 1] = a_character
        end
    end

    return retlist
end
