--
-- // 战斗力=生命值*1+物理攻击力*5+法术攻击力*5+物理暴击*1+法术暴击*1+物理命中*1+法术命中*1+物理护甲*5+法术抗性*5+物理穿透*5+
-- //        法术穿透*5+闪避*5+抵抗*5+吸血等级*5+生命值恢复*1+能量值恢复*1+治疗效果*5+所有技能等级*25

local function calccharactercombatpower(character)
    local convertfplist = g_datahelper.get_convertfplist()

    local retval = 0
    
    local convertfplistlen = #convertfplist
    
    for i=1, convertfplistlen-1 do
        local a_convertfp = convertfplist[i]
        local attridx = a_convertfp[1]
        if attridx > 0 and attridx < 38 then
            retval = retval + (character:get_attr(a_convertfp[1]) * a_convertfp[2] / 100)
        end
    end

    local allskilllv = 0
    local sklist = character:get_skill_list()
    for i=2, #sklist do
        if i > 5 then break end
        allskilllv = allskilllv + sklist[i][2]
    end
    retval = retval + (allskilllv * convertfplist[convertfplistlen][2] / 100)

    --[[
    var jbPower = 0;
    for (var i=0;i<card['jb'].length;++i){
        var jbSkill = card['jb'][i];
        if (!jbSkill || !jbSkill.i || !jbSkill.lv)
            continue;

        var cfg = bond_skill_list[jbSkill.i];
        if (!cfg || !cfg.skillID || !cfg.score)
            continue;

        jbPower += cfg.score * jbSkill.lv;
    }
    power += jbPower;
    ]]

    return math.floor(retval)
end

return calccharactercombatpower