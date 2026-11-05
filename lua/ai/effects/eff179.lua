-- 179 薄葬 血量冻结在特定值
local eff = {}
eff.trigger = function(target, hpdelta, is_simu)
    return aieffectcaster.trigger_179(target, hpdelta, is_simu)
end
return eff