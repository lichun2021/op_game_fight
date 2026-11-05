-- 230 处于某种受创类型时连续执行
local function check_do(character)
    return aieffectcaster.check_do_230(character)
end

return {
    check_do = check_do
}