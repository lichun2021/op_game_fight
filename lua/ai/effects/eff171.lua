-- 171 法术暴击值减免
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 171, param)
end

return rethelper