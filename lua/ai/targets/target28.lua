-- 28 所有敌军召唤物
return function(params)
    local characterlist = g_targethelper.pick_filteredrivallist(params)
    return g_targethelper.get_allsummon(characterlist, params)
end