-- 22 范围内随机的一个友军
--[[
            getRandomCharacterInRange(aiContext, source, pickFilterList(filterList, friendList), targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
]]
return function ( params )
    local thetargetsinrange = g_targethelper.get_targetsinrange(g_targethelper.pick_filteredfriendlist(params), params, params.source)
    if thetargetsinrange ~= nil then 
        return g_targethelper.get_randomcharacter(thetargetsinrange, params)
    else
        return nil
    end
end