-- 278 根据标记层数改变吸血等级
--get_extrahpadd1
local eff = {}

eff.get_extrahpadd1 = function(source, target)
    return aieffectcaster.get_extra_hpsuck_278(source, target)
end

return eff