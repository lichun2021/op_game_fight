-- 251 改变攻击速度
local function check_do(character, param)
    return aieffectcaster.check_exert_special_effect(character, 251, param)
end

return {
    check_do = check_do
}