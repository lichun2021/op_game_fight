-- roundtable

local AttrType = require "datadefs/attrtype"
local HurtType = require "hurtfuncs/hurttype"

local CD_TEAMSIDE = g_cddata.CD_TEAMSIDE

local roundtable = {}

function roundtable.calc_missprob(source, target, hurttype)
    if hurttype == HurtType.HEALING_HURT_TYPE then return 0 end
    
    local source_lv = source:get_attr(AttrType.Lv)
    local target_lv = target:get_attr(AttrType.Lv)

    --未命中概率 = A% - B%*攻击方命中值/(攻击方命中值*F+攻击方等级*C+D)+（防御者等级-攻击方等级）*E%
    local hitp1 = hurttype == HurtType.PHYSICAL_HURT_TYPE and source:get_attr(AttrType.BAHit) or source:get_attr(AttrType.MAHit)
    local ps = g_datahelper.get_fightingparameter(hurttype == HurtType.PHYSICAL_HURT_TYPE and 3 or 4)
    local missprob = ps[1] - ps[2] * hitp1 / (hitp1*ps[6] + ps[3] * source_lv + ps[4]) + (target_lv - source_lv) * ps[5]
    
    missprob = missprob < ps[7] and ps[7] or missprob
    missprob = missprob > ps[8] and ps[8] or missprob

    missprob = missprob - g_effecthelper.get_eff(hurttype == HurtType.PHYSICAL_HURT_TYPE and 106 or 107).check_do(source, missprob)

    missprob = missprob < ps[7] and ps[7] or missprob
    missprob = missprob > ps[8] and ps[8] or missprob

    return missprob
end


function roundtable.calc_escprob(source, target, hurttype)
    if hurttype == HurtType.HEALING_HURT_TYPE then return 0 end

    --受伤者基础闪避率=A%*受伤者闪避值/(受伤者闪避值*E+受伤者等级*B+C)+（防御者等级-攻击方等级）*D%
    local escp2 = target:get_attr(hurttype == HurtType.PHYSICAL_HURT_TYPE and AttrType.BADodge or AttrType.MADodge)
    local ps = g_datahelper.get_fightingparameter(hurttype == HurtType.PHYSICAL_HURT_TYPE and 5 or 6)
    local escprob = ps[1] * escp2 / (escp2*ps[5] + ps[2] * target:get_attr(AttrType.Lv) + ps[3]) + (target:get_attr(AttrType.Lv) - source:get_attr(AttrType.Lv)) * ps[4]

    local the_eff_275_val = g_effecthelper.get_eff(275).check_do(source)
    if the_eff_275_val > 0 then 
        if escprob < 0 then escprob = 0 end
        if escprob > 100.0 then escprob = 100.0 end
    else
        if escprob < ps[7] then escprob = ps[7] end
        if escprob > ps[8] then escprob = ps[8] end
    end

   -- escprob = escprob - g_effecthelper.get_eff(hurttype == HurtType.PHYSICAL_HURT_TYPE and 108 or 109).check_do(source, escprob)
    escprob = escprob + g_effecthelper.get_eff(hurttype == HurtType.PHYSICAL_HURT_TYPE and 108 or 109).check_do(target, escprob) -- 12.0 108效果 被施加此效果的人受到闪避率降低的效果。
    
    if the_eff_275_val > 0 then 
        if escprob < 0 then escprob = 0 end
        if escprob > 100.0 then escprob = 100.0 end
    else
        if escprob < ps[7] then escprob = ps[7] end
        if escprob > ps[8] then escprob = ps[8] end
    end

    --6.0:攻击者无视目标闪避，优先级比目标自身的闪避率上下限更高
    if g_effecthelper.get_eff(196).check_do(source) > 0 then
        escprob = 0
    end

    return escprob
end


function roundtable.calc_chprob(source, target, hurttype)
    local is_friendheal = hurttype == HurtType.HEALING_HURT_TYPE and source:get_data(CD_TEAMSIDE) == target:get_data(CD_TEAMSIDE)

    --攻击者物理暴击率=A%*攻击者实际物理暴击值/(攻击方实际物理暴击值*E+攻击方等级*B+C)+（攻击方等级-受伤者等级）*D%
    local chp1 = hurttype == HurtType.PHYSICAL_HURT_TYPE and source:get_attr(AttrType.BACrit) or source:get_attr(AttrType.MACrit)
    if is_friendheal ~= true then 
        chp1 = chp1 + g_effecthelper.get_eff(hurttype == HurtType.PHYSICAL_HURT_TYPE and 170 or 171).check_do(target, chp1)
        chp1 = chp1 - (hurttype == HurtType.PHYSICAL_HURT_TYPE and target:get_attr(AttrType.BACritResist) or target:get_attr(AttrType.MACritResist))
    end 

    if chp1 < 0 then chp1 = 0 end

    local source_lv = source:get_attr(AttrType.Lv)
    local target_lv = target:get_attr(AttrType.Lv)
    local ps = g_datahelper.get_fightingparameter(hurttype == HurtType.PHYSICAL_HURT_TYPE and 7 or 8)
    local chprob = ps[1] * chp1 / (chp1*ps[5] + ps[2] * source_lv + ps[3]) + (source_lv - target_lv) * ps[4]

    local the_eff_275_val = g_effecthelper.get_eff(275).check_do(source)
    if the_eff_275_val > 0 then
        if chprob < 0 then chprob = 0 end
        if chprob > 100.0 then chprob = 100.0 end
    else
        if chprob < ps[7] then chprob = ps[7] end
        if chprob > ps[8] then chprob = ps[8] end
    end

    --攻击者自身暴击率提升
    chprob = chprob + g_effecthelper.get_eff(hurttype == HurtType.PHYSICAL_HURT_TYPE and 104 or 105).check_do(source, chprob)

    --目标减免暴击率
    if is_friendheal ~= true then
        chprob = chprob + g_effecthelper.get_eff(hurttype == HurtType.PHYSICAL_HURT_TYPE and 168 or 169).check_do(target, chprob)
    end

    if the_eff_275_val > 0 then
        if chprob < 0 then chprob = 0 end
        if chprob > 100.0 then chprob = 100.0 end
    else
        if chprob < ps[7] then chprob = ps[7] end
        if chprob > ps[8] then chprob = ps[8] end
    end

    return chprob
end


return roundtable