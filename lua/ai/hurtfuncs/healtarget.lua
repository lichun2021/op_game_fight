-- healtarget

local AttrType = require "datadefs/attrtype"
local HurtType = require "hurtfuncs/hurttype"
local TalentTrigger = require "hurtfuncs/talenttrigger"
local Cdopcode = require "datadefs/characterdataopcode"

local get_healvalue = require "hurtfuncs/gethealvalue"
local checkandexecutetalent = require "hurtfuncs/checkandexecutetalent"

local CD_ATTACKER = Cdopcode.CD_ATTACKER

local theeffecthelper
local thecharacterhelper
local thehitlabel
local thestathelper

return function (source, target, originhealvalue, hitlabel, skillid, is_simu, markid)
    thecharacterhelper = thecharacterhelper or g_characterhelper
    local source_teamside = source:get_teamside()
    local target_teamside = target:get_teamside()
    if source_teamside ~= target_teamside and thecharacterhelper.is_cannon(source) ~= true and is_simu ~= true then 
        target:set_data(CD_ATTACKER, source)
    end

    theeffecthelper = theeffecthelper or g_effecthelper

    if theeffecthelper.get_eff(178).trigger(source, target, is_simu) then 
        return 0
    end 

    if originhealvalue == 0 then 
        return 0
    end 

    local realhealvalue = get_healvalue(target, originhealvalue, markid)
    if realhealvalue == 0 then 
        return 0
    end 

    if is_simu == true then 
        return realhealvalue
    end 

    if realhealvalue < 0 then 
        realhealvalue = realhealvalue + theeffecthelper.get_eff(265).check_do(target, -realhealvalue, is_simu)

        if target:has_effect(288) then
            local hpreduce = theeffecthelper.get_eff(288).check_do(target, source, -realhealvalue)
            realhealvalue = realhealvalue + hpreduce
        end

        if realhealvalue == 0 then 
            return 0
        end
    end 

    if target:has_effect(316) then
        local replnum = theeffecthelper.get_eff(316).trigger(target, realhealvalue)

        if math.abs(replnum) >= math.abs(realhealvalue) then
            return 0
        end

        realhealvalue = realhealvalue - replnum
    end

    checkandexecutetalent(source, target, TalentTrigger.BY_HURL)

    if hitlabel > 0 then 
        thehitlabel = thehitlabel or g_hitlabel
        thehitlabel.execute(hitlabel, realhealvalue, target)
    end 

    local oldhp = target:get_attr(AttrType.HP)
    target:update_hp(oldhp + realhealvalue)

    thestathelper = thestathelper or g_stathelper

    local hpdelta = target:get_attr(AttrType.HP) - oldhp
    if source_teamside == target_teamside then 
        if hpdelta > 0 and thestathelper then 
            thestathelper.append_hurt(source, HurtType.HEALING_HURT_TYPE, hpdelta, target)
        end 
    else
        if hpdelta < 0 and thestathelper then
            thestathelper.append_hurt(source, HurtType.PHYSICAL_HURT_TYPE, -hpdelta, target)
        end 
    end 

    target:on_hpdelta(hpdelta)

    -- if hpdelta < 0 then
    --     theeffecthelper.get_eff(288).on_hurt(target, source, -hpdelta, -originhealvalue)
    -- end

    theeffecthelper.get_eff(161).trigger(source, target)

    return realhealvalue
end
