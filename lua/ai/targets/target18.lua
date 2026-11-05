-- 18 最近的友方
return function( params )
    return g_targethelper.get_nearestcharacter(g_targethelper.pick_filteredfriendlist(params), params)
end