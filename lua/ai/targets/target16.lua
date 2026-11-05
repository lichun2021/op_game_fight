-- 16 所有人中一个随机单位
return function(params)
    local characterlist = params.filterlist
    if characterlist == nil then
        characterlist = {}
        local friendlist = g_targethelper.get_filteredfriendlist(params)
        local rivallist = g_targethelper.get_filteredrivallist(params)
        for i=1,#rivallist do
            -- table.insert(characterlist, rivallist[i])
            characterlist[#characterlist + 1] = rivallist[i]
        end
        for i=1,#friendList do
            -- table.insert(characterlist, friendList[i])
            characterlist[#characterlist + 1] = friendlist[i]
        end
    end
    return g_targethelper.get_randomcharacter(characterlist, params)
end