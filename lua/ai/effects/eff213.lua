-- 213 使指定阵营的指定角色的某个技能在造成伤害（伤害执行）时无视敌人的不死
local function check_do(character)
    return aieffectcaster.check_do_213(character)
end

return {
    check_do = check_do
}