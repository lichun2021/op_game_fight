-- 59 所有已死亡的友军
return function(params)
    return g_targethelper.get_deadcharacter(g_targethelper.pick_filteredfriendlist(params))
end