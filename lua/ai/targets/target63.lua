-- 63 血量百分比最高的敌军
return function(params)
    return g_targethelper.get_characterhighesthpratio(g_targethelper.pick_filteredrivallist(params), params)
end