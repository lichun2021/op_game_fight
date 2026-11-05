-- 108 闪避几率
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 108, param)
end

return rethelper