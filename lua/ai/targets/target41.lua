-- 41 考虑自身攻击范围的随机敌人
-- 比如近战A随机到远程B，那么A必须走到B附近才能攻击B
return function ( params )
    if params.source:is_currenttargetconfirmed() then
        local thetarget = params.source:get_currenttarget()
        if thetarget ~= nil and thetarget:canbeattack(params.source, params.is_ignore_canbeattack) then
            return {thetarget}
        end
    end
    return g_targethelper.get_randomcharacter(g_targethelper.pick_filteredrivallist(params), params)
end