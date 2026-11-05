-- 166 物理暴击倍数减免
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_do_float(character, 166, param)
end

return rethelper