-- 14 距离最远的敌人
return function( params )
    return g_targethelper.get_farthestcharacter(g_targethelper.pick_filteredrivallist(params), params)
end