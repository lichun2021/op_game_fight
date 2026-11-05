-- 170 物理暴击值减免
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 170, param)
end

return rethelper