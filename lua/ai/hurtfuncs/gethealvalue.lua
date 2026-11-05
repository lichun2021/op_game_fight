-- gethealvalue

local AttrType = require "datadefs/attrtype"

return function(character, originvalue, markid)
    local healvalue = originvalue * (1 + character:get_attr(AttrType.HPAdd3) / 100)
    healvalue = healvalue + g_effecthelper.get_eff(131).check_do(character, originvalue)
    healvalue = healvalue + g_effecthelper.get_eff(148).check_do(character, originvalue)

    --镜像造成治疗的倍率
    if character:is_summon() then
        healvalue = healvalue * character:get_summonproperty_treatrate() / 100
    end
    
    if g_effecthelper.get_eff(255).check_do(character, 0) > 0 then
        if healvalue < 0 and markid > 0 and character:has_effect(259) then
            local reducevalue = g_effecthelper.get_eff(259).check_do(character, markid, -healvalue)
            healvalue = healvalue + reducevalue
        end
        return healvalue
    end

    return healvalue < 0 and 0 or math.floor(healvalue)
end