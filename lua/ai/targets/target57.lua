-- 57 随机一个已死亡的敌军
return function( params )
    return g_targethelper.get_randomdeadcharacter(g_targethelper.pick_filteredrivallist(params))
end