-- 231 跳过攻击序列中指定序号的技能

local function check_do(character, attackseq, nextseqindex)
    return aieffectcaster.check_do_231(character, attackseq, nextseqindex)
end

return {
    check_do = check_do
}