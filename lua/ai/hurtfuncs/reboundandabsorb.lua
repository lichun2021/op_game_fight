-- reboundandabsorb

local AttrType = require "datadefs/attrtype"
-- local hurttarget = require "aihurttarget"
local characterdeadbyhurt = require "hurtfuncs/characterdeadbyhurt"
local HurtType = require "hurtfuncs/hurttype"

return function (source, target, param)
    -- print("begin."..tostring(source:get_attr(11)))

    local hpdelta = param.hpdelta
    local hurttype = param.hurttype
    local hittype = param.hittype

    local tmphpdelta1 = 0
    if hpdelta > 0 then 
        tmphpdelta1 = g_effecthelper.get_eff(113).check_do(target, source:get_attr(AttrType.HP))
        if hurttype == HurtType.PHYSICAL_HURT_TYPE then 
            tmphpdelta1 = g_effecthelper.get_eff(135).check_do(target, hpdelta)
        elseif hurttype == HurtType.MAGICAL_HURT_TYPE then 
            tmphpdelta1 = g_effecthelper.get_eff(136).check_do(target, hpdelta)
        end 
    end 

    local hpdelta_beforeavoidhurt = param.hpdelta_beforeavoidhurt
    assert(hpdelta_beforeavoidhurt ~= nil, "hpdelta before avoid hurt is nil!")
    if hpdelta_beforeavoidhurt > 0 and target:has_effect(283) then
        tmphpdelta1 = tmphpdelta1 + g_effecthelper.get_eff(283).check_do(target, hurttype, hpdelta_beforeavoidhurt)
    end

    local ps = g_datahelper.get_fightingparameter(10)
    -- 吸血百分比 = A%*攻击者吸血值 / (攻击者吸血值*B + 受伤者等级*C + D)
    local source_hpadd1 = source:get_attr(AttrType.HPAdd1)
    source_hpadd1 = source_hpadd1 + g_effecthelper.get_eff(278).get_extrahpadd1(source, target)
    local vmpps = ps[1] * source_hpadd1 / (source_hpadd1 * ps[2] + ps[3] * target:get_attr(AttrType.Lv) + ps[4])
    if g_effecthelper.get_eff(275).check_do(source) > 0 then 
        if vmpps < 0 then vmpps = 0 end 
        if vmpps > 1.0 then vmpps = 1.0 end 
    else 
        if vmpps < ps[7] then vmpps = ps[7] end 
        if vmpps > ps[8] then vmpps = ps[8] end 
    end 

    local tmphpdelta2 = 0 
    if hurttype == HurtType.PHYSICAL_HURT_TYPE or (hurttype == HurtType.MAGICAL_HURT_TYPE and g_effecthelper.get_eff(177).check_do(source, 0) > 0) then
        tmphpdelta2 = hpdelta * vmpps
    end

    local resulthp = tmphpdelta1 - tmphpdelta2
    if resulthp > 0 then 
        local reboundparam = {}
        for k,v in pairs(param) do 
            reboundparam[k] = v
        end 
        reboundparam.is_skiprebound = true
        reboundparam.hpdelta = resulthp
        reboundparam.stathurtsourcealt = target
        reboundparam.is_ignorehitlabel = false

        g_hurthelper.hurttarget(source, source, reboundparam)
        -- hurttarget(source, source, reboundparam)
        
        if source:get_attr(AttrType.HP) <= 0 then 
            characterdeadbyhurt(source, target, param.skillid, param.skill_level, 0, 0)
        end 
    elseif resulthp < 0 then 
        local hitlabel = hittype == 100000 and 100002 or 100003
        g_hitlabel.execute(hitlabel, -resulthp, source)

        local oldhp = source:get_attr(AttrType.HP)
        source:update_hp(oldhp - resulthp)
    end 
    -- print(tostring(source:get_attr(11)))
end
