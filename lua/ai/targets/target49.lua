-- 49 生命值处于某个百分比区间的所有敌人
return function(params)
    local characterlist = g_targethelper.pick_filteredrivallist(params)
    return g_targethelper.get_characterinhprange(characterlist, params, params.x1 / 100.0, params.x2 / 100.0)
end