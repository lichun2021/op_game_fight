-- 103 不死

local function check_do(character, param)
    return aieffectcaster.check_exert_special_effect(character, 103, param)
end

return {
    check_do = check_do
}