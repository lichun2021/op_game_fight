-- 208 效果拥有者受击获取能量增加
local eff = {}

eff.check_do = function(character, param)
    return aieffectcaster.check_exert_mixchangetype_effect(character, 208, param)
end

return eff