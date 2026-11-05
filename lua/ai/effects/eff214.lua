-- 214 目标的攻击序列中非普攻技能之前多加一个技能

local function get_altattackseq(character, is_useseq2)
    return aieffectcaster.get_eff_214_altattackseq(character, is_useseq2)
end

return {
    get_altattackseq = get_altattackseq
}