-- 8 自己范围内所有敌对单位
return function(params)
    return g_targethelper.get_targetsinrange(g_targethelper.pick_filteredrivallist(params), params, params.source)
end