-- 267 若对目标造成的伤害[是否只取来自施法者某执行id（伤害执行&效果执行）的伤害。]低于X点/X百分比目标血量，则附加理论计算值（无视防御）一定百分比的纯粹伤害
local rethelper = {}

rethelper.check_do = function(character, param)
    return aieffectcaster.check_do_267(character, param)
end

return rethelper