-- 253 新新魅惑(目标只能受到真正敌对阵营的伤害，自己会攻击队友，队友不会攻击被控制的单位)
local function check_do(character, param)
    return aieffectcaster.check_exert_special_effect(character, 253, param)
end

return {
    check_do = check_do
}