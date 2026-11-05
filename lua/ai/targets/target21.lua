-- 21 敌方的男性
local CD_SEX = g_cddata.CD_SEX
return function( params )
    local characterList = g_targethelper.pick_filteredrivallist(params)
    local retlist = {}
    for i=1,#characterList do
        local a_charater = characterList[i]
        if a_charater:get_data(CD_SEX) == g_characterhelper.sextype.MALE and a_charater:canbeattack(params.source, params.is_ignore_canbeattack) then
            -- table.insert(retlist, a_charater)
            retlist[#retlist + 1] = a_charater
        end
    end
    return retlist
end