-- 175 伤害在命中目标时，目标因受伤所获能量增加/减少
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 175, param)
end

return eff