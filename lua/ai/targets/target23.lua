-- 23 范围内随机的一个敌军
return function ( params )
    local thetargetsinrange = g_targethelper.get_targetsinrange(g_targethelper.pick_filteredrivallist(params), params, params.source)
    if thetargetsinrange ~= nil then 
        return g_targethelper.get_randomcharacter(thetargetsinrange, params)
    else
        return nil
    end
end