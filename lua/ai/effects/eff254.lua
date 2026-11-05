-- 254 持续时间内，如果物理法术伤害击中任何敌人且暴击，则触发一个执行（有触发间隔）
local rethelper = {}

rethelper.check_do = function(source, target, hurttype)
    return aieffectcaster.check_do_254(source, target, hurttype)
end

return rethelper