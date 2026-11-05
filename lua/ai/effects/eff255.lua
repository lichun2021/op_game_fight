-- 255 允许受到负治疗值
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 255, 0)
end

return rethelper