-- 178 林肯法球（在抵挡伤害后可以触发另一个效果）
local eff = {}

eff.trigger = function(source, target, is_simu)
    aieffectcaster.trigger_178(source, target, is_simu)
end

return eff