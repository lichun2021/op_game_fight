-- 259 受到指定标记原始伤害减免
local eff = {}

eff.check_do = function (character, markid, hpdelta)
    return aieffectcaster.check_do_259(character, markid, hpdelta)
end

return eff