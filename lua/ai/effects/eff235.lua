-- 235 持续时间内，如果指定类型的（物理或法术或治疗）伤害击中敌人时，必定暴击
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_exert_special_effect(character, 235, param)
end

return rethelper