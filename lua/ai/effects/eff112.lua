-- 112 免疫所有攻击
local eff = {}
eff.check_do = function(target, hpdelta, is_simu)
    return aieffectcaster.check_do_repeleff(112, target, hpdelta, is_simu)
end
return eff