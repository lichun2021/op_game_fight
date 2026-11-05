-- 221 目标属性达到条件时必定暴击
local rethelper = {}

rethelper.check_do = function(source, target)
    return aieffectcaster.check_do_221(source, target)
end

return rethelper