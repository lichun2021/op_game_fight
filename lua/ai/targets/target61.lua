-- 61 所有已死亡的友军/敌军
return function(params)
    return g_targethelper.get_deadcharacter(g_targethelper.pick_filteredalllist(params))
end