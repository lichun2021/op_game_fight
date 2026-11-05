-- 99 查找友方符合（技能目标参数[为标签表中标签ID列表]）的友方
return function(params)
    return g_targethelper.get_characterhaslabels(g_targethelper.pick_filteredfriendlist(params), params, params.search_params)
end