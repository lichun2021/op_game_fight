-- 62 血量百分比最高的友军
return function(params)
    return g_targethelper.get_characterhighesthpratio(g_targethelper.pick_filteredfriendlist(params), params)
end