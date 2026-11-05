-- 167 法术暴击倍数减免
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_do_float(character, 167, param)
end

return rethelper