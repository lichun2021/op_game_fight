-- 3 本方血量百分比最低单位
return function(params)
    local characterlist = g_targethelper.pick_filteredfriendlist(params)
    return g_targethelper.get_lowesthpratiocharacter(characterlist, params)
end