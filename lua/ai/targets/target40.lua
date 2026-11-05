-- 40 指定阵营的某个或某些角色id（作用对象类型填新类型，作用范围考虑支持数组填阵营和角色id）
return function(params)
    if params.search_params == nil then return nil end
    return g_targethelper.get_character_withid(g_targethelper.pick_filteredalllist(params), params)
end