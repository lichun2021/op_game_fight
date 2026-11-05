-- 19 最近的敌方
return function( params )
    return g_targethelper.get_nearestcharacter(g_targethelper.pick_filteredrivallist(params), params)
end