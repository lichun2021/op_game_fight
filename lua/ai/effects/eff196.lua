-- 196 无视目标闪避抵抗
local rethelper = {}

rethelper.check_do = function(character)
    return aieffectcaster.check_exert_special_effect(character, 196, 0)
end

return rethelper