-- 169 法术暴击率减免
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 169, param)
end

return rethelper