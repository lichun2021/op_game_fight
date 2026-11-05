-- 67 效果 288 统计结果中对目标造成伤害最高的敌方单位
return function(params)
    local the_target = g_effecthelper.get_eff(288).get_first_mosthpdelta_attacker(params.source)
    if the_target ~= nil then
        return { the_target }
    else
        local currenttarget = params.source:get_currenttarget()
        if currenttarget then
            return { currenttarget }
        end
    end
    
    return nil
end
