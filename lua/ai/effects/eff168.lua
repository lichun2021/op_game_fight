-- 168 物理暴击率减免
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 168, param)
end

return rethelper