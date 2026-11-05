-- 275 拥有该效果的角色在攻击时计算暴击，闪避，抵抗，吸血，无视公式上下限
local rethelper = {}

rethelper.check_do = function(character)
    return aieffectcaster.check_exert_special_effect(character, 275, 0)
end

return rethelper