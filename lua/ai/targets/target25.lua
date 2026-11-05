-- 25 不含搜索者自己的随机敌军(不会随到自己)
local CD_MYID = g_cddata.CD_MYID
return function(params)
    local characterlist = g_targethelper.pick_filteredrivallist(params)
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:get_data(CD_MYID) == params.source:get_data(CD_MYID) then
            table.remove( characterlist, i )
            break
        end
    end
    return g_targethelper.get_randomcharacter(characterlist, params)
end