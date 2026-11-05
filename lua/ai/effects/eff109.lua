-- 109 抵抗几率
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 109, param)
end

return rethelper