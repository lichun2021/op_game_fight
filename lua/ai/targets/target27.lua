-- 27 所有友军召唤物
return function(params)
    local characterlist = g_targethelper.pick_filteredfriendlist(params)
    return g_targethelper.get_allsummon(characterlist, params)
end