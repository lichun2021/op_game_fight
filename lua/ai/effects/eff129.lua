-- 129 不能攻击别人也不能被攻击 仅给召唤镜像使用

local function check_do(character, param)
    return aieffectcaster.check_exert_special_effect(character, 129, param)
end

return {
    check_do = check_do
}