-- 262 若伤害命中则必定暴击
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_do_262(character, param)
end

return rethelper