
local get_baseap = require "hurtfuncs/getbaseap"
local CD_ALTACCUTICK = g_cddata.CD_ALTACCUTICK

return function (source, skill_level, hurt_id)
    local hurtdata = g_datahelper.get_hurtdata(hurt_id)
    local base_ap = get_baseap(hurtdata.type, source)

    -- 攻击力系数=技能基础攻击力系数+系数成长*技能等级+ 蓄力时间 / 蓄力时间间隔*单位蓄力间隔增加的攻击力百分比
    local skill_ap_factor = (hurtdata.baseapfactor + hurtdata.apbonus * skill_level) / 100

    if hurtdata.accuinterval > 0 then
        --local the_accu_tick = source:get_accumulatetick()
        local the_accu_tick = source:get_actionatt(14)
        local the_alt_accu_tick = source:get_data(CD_ALTACCUTICK) --source:get_altaccutick()
        if the_alt_accu_tick > 0 then the_accu_tick = the_alt_accu_tick end
        skill_ap_factor = skill_ap_factor + (the_accu_tick * hurtdata.accuattaddperinterval / hurtdata.accuinterval) / 100
    end

    -- 额外伤害值=技能基础额外伤害值+额外伤害值成长*技能等级
    local skill_extra_damage = (hurtdata.baseextrahurt + hurtdata.extrahurtbonus * skill_level) / 100

    -- 未减伤伤害=（攻击方物理攻击*攻击力系数+额外伤害值）
    return base_ap * skill_ap_factor + skill_extra_damage
end