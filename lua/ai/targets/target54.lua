-- 54 拥有所有指定标签的敌方单位
return function(params)
    return g_targethelper.get_characterhaslabels(g_targethelper.pick_filteredrivallist(params), params, params.search_params)
end