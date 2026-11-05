-- 11 当前目标范围内所有友方单位
return function(params)
    return g_targethelper.get_targetsinrange(g_targethelper.pick_filteredfriendlist(params), params, params.target)
end