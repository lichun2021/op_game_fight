-- 56 随机一个已死亡的友军
return function( params )
    return g_targethelper.get_randomdeadcharacter(g_targethelper.pick_filteredfriendlist(params))
end