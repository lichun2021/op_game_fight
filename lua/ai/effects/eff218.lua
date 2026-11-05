-- 218 对同一敌人连续攻击将造成更多伤害
local rethelper = {}

rethelper.check_do = function(source, target, hpdelta, hurttype)
    return aieffectcaster.check_do_218(source, target, hpdelta, hurttype)
end

return rethelper