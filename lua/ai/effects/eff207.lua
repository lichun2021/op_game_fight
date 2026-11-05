-- 207 属性伤害减免
local eff = {}

eff.check_do = function(character, hurtprop, hpdelta)
    return aieffectcaster.check_do_207(character, hurtprop, hpdelta)
end

return eff