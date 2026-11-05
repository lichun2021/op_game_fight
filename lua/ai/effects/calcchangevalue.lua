-- 通用计算效果值

local calcchangevalue = function(innerexec, skilllevel, target)
    local changevalue = (innerexec.changevalue + innerexec.levelup * skilllevel) / 100
    if innerexec.changetype == 1 and innerexec.valuebase > 0 and innerexec.valuebase < 38 then
        local attrval = target:get_attr(innerexec.valuebase)
        changevalue = changevalue / 100 * attrval;
    end
    
    if innerexec.limitonoff == 1 then
        local limitupper = (innerexec.limitvalupperbase + innerexec.limitvaluppergrowth * skilllevel) / 100
        local limitlower = (innerexec.limitvallowerbase + innerexec.limitvallowergrowth * skilllevel) / 100
        
        changevalue = math.min(changevalue, limitupper)
        changevalue = math.max(changevalue, limitlower)
    end

    return math.floor(changevalue)
end

return calcchangevalue