-- 174 施法者属性伤害加成（火，雷，水等 !目标属性字段对应属性类型）
local eff = {}
eff.check_do = function(source, hurtprop, hpdelta, is_simu)
    return aieffectcaster.check_do_174(source, hurtprop, hpdelta, is_simu)
end
return eff