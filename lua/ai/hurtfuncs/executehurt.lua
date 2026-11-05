local roundtable = require "hurtfuncs/roundtable"
local checkandexecutetalent = require "hurtfuncs/checkandexecutetalent"
local get_basechp = require "hurtfuncs/getbasechp"
local calcinithurt = require "hurtfuncs/calcinithurt"
local get_realtimehurtprop = require "hurtfuncs/getrealtimehurtprop"
local hurttarget = require "hurtfuncs/hurttarget"
local healtarget = require "hurtfuncs/healtarget"
local recursive_random_one_valid_exec = require "hurtfuncs/recursiverandomonevalidexec"
local handlehurtresult = require "hurtfuncs/handlehurtresult"
local characterdeadbyhurt = require "hurtfuncs/characterdeadbyhurt"

local TalentTrigger = require "hurtfuncs/talenttrigger"
local HurtType = require "hurtfuncs/hurttype"
local HitType = require "hurtfuncs/hittype"
local HurtParam = require "hurtfuncs/hurtparam"
local HitResult = require "hurtfuncs/hitresult"

local ActionType = require "actions/actiontype"
local AttrType = require "datadefs/attrtype"
local CDOPCODE = require "datadefs/characterdataopcode"

local CD_LASTHURTEXEC_INITHURTDELTA = CDOPCODE.CD_LASTHURTEXEC_INITHURTDELTA
local CD_LASTHURTEXEC_REALHURTDELTA = CDOPCODE.CD_LASTHURTEXEC_REALHURTDELTA
local CD_LASTHURTEXEC_ORIGINCHARACTER = CDOPCODE.CD_LASTHURTEXEC_ORIGINCHARACTER
local CD_TEAMSIDE = CDOPCODE.CD_TEAMSIDE
local CD_LASTHURTEXEC_ID = CDOPCODE.CD_LASTHURTEXEC_ID
local CD_IS_EFF_181_RUNNING = CDOPCODE.CD_IS_EFF_181_RUNNING

local theaicontext = aicontext
local theexechelper
local theskillhelper
local theeffecthelper
local themarkhelper
local thesummonhelper
local thedatahelper
local theactionhelper

local function execute_hurt(source, projectile, hurtid, skillid, skill_level, target, markid)

    theexechelper = theexechelper or g_exechelper
    theskillhelper = theskillhelper or g_skillhelper
    thedatahelper = thedatahelper or g_datahelper
    thesummonhelper = thesummonhelper or g_summonhelper
    theeffecthelper = theeffecthelper or g_effecthelper
    
    if target == nil then return 0 end 
    if target:get_attr(AttrType.HP) < 1 then return 0 end
    local hurtdata = thedatahelper.get_hurtdata(hurtid)
    assert(hurtdata ~= nil)
    if hurtdata == nil then return 0 end 

    if thesummonhelper.can_hurtsummon(source, hurtid, target, skillid) ~= true then return 0 end 

    local hurttype = hurtdata.type
    local hurtprop = hurtdata.prop
    -- 未命中概率
    local missprob = roundtable.calc_missprob(source, target, hurttype)

    -- 受伤者基础闪避率
    local escprob = roundtable.calc_escprob(source, target, hurttype)

    -- 暴击率
    local chprob = roundtable.calc_chprob(source, target, hurttype)

    local totalprob = 100
    if missprob > totalprob then missprob = totalprob end 
    totalprob = totalprob - missprob
    if escprob > totalprob then escprob = totalprob end 
    totalprob = totalprob - escprob
    if chprob > totalprob then chprob = totalprob end 
    totalprob = totalprob - chprob 

    local poll = theaicontext.get_randomnumber()
    local presum = 0
    local hittype = 0

    checkandexecutetalent(source, target, TalentTrigger.ALWAYS)

    presum = presum + missprob
    presum = math.floor(presum)
    local is_missed = poll < presum

    presum = presum + escprob
    presum = math.floor(presum)
    local is_escaped = poll < presum

    presum = presum + chprob
    presum = math.floor(presum)
    local is_normalattack = poll > presum

    -- MARK: 11.0
    if target:has_effect(282) and theeffecthelper.get_eff(282).check_do(target, source) > 0 then
        is_missed = false
        is_escaped = false
        is_normalattack = false
    end

    if is_missed ~= true and is_escaped ~= true and is_normalattack then 
        if source:has_effect(235) and theeffecthelper.get_eff(235).check_do(source, hurttype) > 0 then 
            is_missed = false
            is_escaped = false
            is_normalattack = false
        end 
    end 

    local is_eff_181_firstrun = false
    if source:has_effect(181) and theeffecthelper.get_eff(181).check_do(source) > 0 and source:get_data(CD_IS_EFF_181_RUNNING) ~= true then 
        source:set_data(CD_IS_EFF_181_RUNNING, true)
        is_eff_181_firstrun = true
        if is_missed or is_escaped or is_normalattack then 
            if theeffecthelper.get_eff(181).try_trigger(source) then 
                is_missed = false
                is_escaped = false
                is_normalattack = false
            else
                theeffecthelper.get_eff(181).trigger(source, false)
            end 
        end 
    end 

    local is_must_crit =  source:has_effect(221) and theeffecthelper.get_eff(221).check_do(source, target) 
    if not is_must_crit then
        is_must_crit = source:has_effect(290) and theeffecthelper.get_eff(290).check_do(source, target)
    end
    if is_must_crit then 
        is_missed = false
        is_escaped = false
        is_normalattack = false
    end 

    if is_missed then 
        if g_actionhelper.is_deadaction(source:get_currentaction()) ~= true then
            g_hitlabel.execute(100009, 0, source)
        end
        theeffecthelper.get_eff(202).trigger(source)
        if is_eff_181_firstrun then 
            source:set_data(CD_IS_EFF_181_RUNNING, false)
        end 
        return 0
    elseif is_escaped then
        checkandexecutetalent(source, target, TalentTrigger.DODGE)
        g_hitlabel.execute(100007, 0, target)
        if hurttype == HurtType.PHYSICAL_HURT_TYPE then 
            theeffecthelper.get_eff(203).trigger(target)
        else
            theeffecthelper.get_eff(204).trigger(target)
        end 

        target:on_escape()

        if is_eff_181_firstrun then 
            source:set_data(CD_IS_EFF_181_RUNNING, false)
        end 
        return 0
    end 

    local is_replhurt = false -- 免疫单次伤害
    local oldhp = target:get_attr(AttrType.HP)
    hittype = HitType.CRITICAL_HIT
    local basechp = get_basechp(hurttype, source)
    local altchp = theeffecthelper.get_eff(262).check_do(target, skillid)
    if altchp > 0 then 
        is_normalattack = false
        basechp = altchp / 100
    end 

    local is_friendheal = false
    if hurttype == HurtType.HEALING_HURT_TYPE and source:get_data(CD_TEAMSIDE) == target:get_data(CD_TEAMSIDE) then 
        is_friendheal = true
    end 

    if is_friendheal ~= true then 
        -- 攻击者实际暴击倍数=攻击者暴击倍数-目标暴击倍数减免
        if hurttype == HurtType.PHYSICAL_HURT_TYPE then 
            basechp = basechp + theeffecthelper.get_eff(166).check_do(target, basechp) - target:get_attr(AttrType.BACritMAdd) / 100
        else
            basechp = basechp + theeffecthelper.get_eff(167).check_do(target, basechp) - target:get_attr(AttrType.MACritMAdd) / 100
        end 
    end 

    if basechp < 1.0 then basechp = 1.0 end

    if is_normalattack then 
        hittype = HitType.NORMAL_HIT
        basechp = 1.0
    else
        if is_eff_181_firstrun then 
            theeffecthelper.get_eff(181).trigger(source, true)
        end 
    end 

    if is_eff_181_firstrun then
        source:set_data(CD_IS_EFF_181_RUNNING, false)
    end 

    local hpdelta = calcinithurt(source, skill_level, hurtid)
    hpdelta = math.floor(hpdelta)
    if target:has_effect(291) then
        hpdelta = hpdelta + theeffecthelper.get_eff(291).check_do(source, target, hpdelta)
    end
    target:set_data(CD_LASTHURTEXEC_INITHURTDELTA, hpdelta)

    hpdelta = hpdelta * basechp
    hpdelta = math.floor(hpdelta)

    local disthpdelta = theeffecthelper.get_eff(217).check_do(source, target, hpdelta)
    local sametargethpdelta = theeffecthelper.get_eff(218).check_do(source, target, hpdelta, hurttype)

    hpdelta = hpdelta + disthpdelta + sametargethpdelta

    local realhurtprop = get_realtimehurtprop(source, hurtprop)

    if hurttype == HurtType.HEALING_HURT_TYPE then 
        local hitlabel = hittype == HitType.NORMAL_HIT and 100002 or 100003
        hpdelta = healtarget(source, target, hpdelta, hitlabel, skillid, false, markid)
    else
        local param = HurtParam.new(projectile, skillid, skill_level, hpdelta, hurttype, realhurtprop, hittype, hurtid, false, hurtid)
        param.markid = markid

        local hitresult, blockresult = hurttarget(source, target, param)

        target:set_data(CD_LASTHURTEXEC_REALHURTDELTA, oldhp - target:get_attr(AttrType.HP))
        target:set_data(CD_LASTHURTEXEC_ORIGINCHARACTER, source)
        target:set_data(CD_LASTHURTEXEC_ID, hurtid)
        if theeffecthelper.get_eff(267).check_do(target, source) > 0 then
            theeffecthelper.clean_eff(target, 267)
        end 

        if hitresult == HitResult.HitResultType.HIT_BYPASS then return 0 end
        is_replhurt = blockresult == HitResult.BlockType.HIT_BLOCKED

        target:on_hurt(false, hurtid, param.hpdelta, hurttype, source, hurtid)
    end 

    if hittype == HitType.CRITICAL_HIT then 
        target:on_criticalhitskillrelease()
        source:on_castcriticalhurt()
        if hpdelta ~= 0 then 
            theeffecthelper.get_eff(254).check_do(source, target, hurttype)
        end
    end 

    themarkhelper = themarkhelper or g_markhelper
    themarkhelper.triggerby_hurt(target, skillid, skill_level, hurttype, hurtprop, hpdelta, hittype, source, hurtid)

    -- hitexec
    local is_hitexec = false
    local hitexeccondition = hurtdata.hitexeccondition
    if hitexeccondition == 0 then 
        is_hitexec = true
    elseif hitexeccondition == 1 or hitexeccondition == 4 then -- 随等级增长
        is_hitexec = theaicontext.get_randomnumber() <= 100 + (skill_level - target:get_attr(AttrType.Lv)) * hurtdata.hitexecparam
    elseif hitexeccondition == 2 or hitexeccondition == 3 then -- 固定几率
        is_hitexec = theaicontext.get_randomnumber() < hurtdata.hitexecparam
    elseif hitexeccondition == 5 then -- 几率随等级成长
        is_hitexec = theaicontext.get_randomnumber() < hurtdata.hitexecparam + skill_level * hurtdata.hitlevelup / 100
    end 

    if is_hitexec then 
        local hitexecs = hurtdata.hitexecs
        if hitexeccondition == 3 or hitexeccondition == 4 then 
            hitexecs = recursive_random_one_valid_exec(target, hitexecs)
        end 

        for i=1,#hitexecs do 
            local execid = hitexecs[i]
            if execid > 0 then 
                theexechelper.execute(execid, skillid, skill_level, source, projectile, target, 0, false, markid)
            end 
        end 
    end 

    if hurtdata.hitspecialeffect > 0 then 
        theexechelper.execute(hurtdata.hitspecialeffect, skillid, skill_level, source, projectile, target, 0, false, markid)
    end 

    if hurtdata.hitaudio > 0 then 
        theexechelper.execute(hurtdata.hitaudio, skillid, skill_level, source, projectile, target, 0, false, markid)
    end 

    target:set_data(CD_LASTHURTEXEC_INITHURTDELTA, 0)
    target:set_data(CD_LASTHURTEXEC_REALHURTDELTA, 0)
    target:set_data(CD_LASTHURTEXEC_ORIGINCHARACTER, nil)
    target:set_data(CD_LASTHURTEXEC_ID, 0)
    
    if target:get_attr(AttrType.HP) > 0 then 
        if hurttype ~= HurtType.HEALING_HURT_TYPE then
            target:on_hurthit(hurtid)
        end 
        --target:get_effectid()
        --target:get_actionatt(13)
        if not is_replhurt then
            if target:get_currentaction() ~= ActionType.EFFECT_ACTION or target:has_effect(266) or (target:get_actionatt(13) > 0 and thedatahelper.get_effectdata(target:get_actionatt(13)).ifreact > 0 and target:is_entereffectaction() ~= true) then 
                handlehurtresult(source, target, hurtid)
            end 
        end
        
        theskillhelper.on_releasehpskill(target)
    else
        characterdeadbyhurt(target, source, skillid, skill_level, hurtid, 0)
    end 

    -- print(tostring(theaicontext:get_currenttick()) .. ":endhurt:" .. tostring(target:get_attr(11)))
    return hittype
end

return execute_hurt