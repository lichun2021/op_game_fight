-- 217 根据距离增加伤害
local rethelper = {}

rethelper.check_do = function(source, target, hpdelta)
    return aieffectcaster.check_do_217(source, target, hpdelta)
end

return rethelper