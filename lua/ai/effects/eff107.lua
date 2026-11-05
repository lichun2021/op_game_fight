-- 107 法术命中率降低
local rethelper = {}

rethelper.check_do = function(character, missprob)
    return aieffectcaster.check_exert_special_effect(character, 107, missprob)
end

return rethelper