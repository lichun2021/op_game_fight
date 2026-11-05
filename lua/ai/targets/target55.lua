-- 55 拥有所有指定标签的所有单位
return function(params)
    return g_targethelper.get_characterhaslabels(g_targethelper.pick_filteredalllist(params), params, params.search_params)
end