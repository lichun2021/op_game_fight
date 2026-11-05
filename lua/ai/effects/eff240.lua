-- 240 跳过技能公共冷却时间并释放一个技能

local function check_do(character)
    return aieffectcaster.check_do_240(character)
end

return {
    check_do = check_do
}