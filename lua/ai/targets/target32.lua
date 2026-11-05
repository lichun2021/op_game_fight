-- 32 所有敌人中某项属性最高的目标

return function(params)
    if params.search_params == nil then return nil end
    local characterlist = g_targethelper.pick_filteredrivallist(params)

    local attrs = params.search_params
    local thelargestval
    local thecharacter = nil
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local a_val = g_targethelper.get_largestattrvalue(a_character, attrs)
            if thelargestval == nil or thelargestval < a_val then
                thelargestval = a_val
                thecharacter = a_character
            end
        end
    end

    if thecharacter ~= nil then return {thecharacter} else return nil end
end