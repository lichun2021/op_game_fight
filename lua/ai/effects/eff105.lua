-- 105 法术暴击率提升
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 105, param)
end

return rethelper