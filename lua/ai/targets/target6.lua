-- 6 对方血量百分比最低单位
return function(params)
    local characterlist = g_targethelper.pick_filteredrivallist(params)
    return g_targethelper.get_lowesthpratiocharacter(characterlist, params)
end