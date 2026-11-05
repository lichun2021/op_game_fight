-- 29 所有召唤物
return function(params)
    if params.filterlist ~= nil then return g_targethelper.get_allsummon(params.filterlist, params) end

    local friendlist = g_targethelper.pick_filteredfriendlist(params)
    local rivallist = g_targethelper.pick_filteredrivallist(params)
    local characterlist = friendlist
    for i=1,#rivallist do
        -- table.insert(characterlist, rivallist[i])
        characterlist[#characterlist + 1] = rivallist[i]
    end
    return g_targethelper.get_allsummon(characterlist, params)
end