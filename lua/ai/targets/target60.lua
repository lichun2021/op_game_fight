-- 60 所有已死亡的敌军
return function(params)
    return g_targethelper.get_deadcharacter(g_targethelper.pick_filteredrivallist(params))
end