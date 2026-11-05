-- 7 对方随机单位
return function(params)
    local characterlist = g_targethelper.pick_filteredrivallist(params)
    return g_targethelper.get_randomcharacter(characterlist, params)
end
