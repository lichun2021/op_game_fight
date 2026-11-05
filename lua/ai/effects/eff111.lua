-- 111 法术免疫
local eff = {}
eff.check_do = function(target, hpdelta, is_simu)
    return aieffectcaster.check_do_repeleff(111, target, hpdelta, is_simu)
end
return eff