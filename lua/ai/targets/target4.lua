-- 4 本方随机单位
return function(params)
    return g_targethelper.get_randomcharacter(g_targethelper.pick_filteredfriendlist(params), params)
end