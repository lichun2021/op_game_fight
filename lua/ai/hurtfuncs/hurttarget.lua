--
local cantransferhurt = require "hurtfuncs/cantransferhurt"
local transferhurt = require "hurtfuncs/transferhurt"
local get_originalavoidancehurt = require "hurtfuncs/getoriginalavoidancehurt"
local checkandexecutetalent = require "hurtfuncs/checkandexecutetalent"
local sharehurt = require "hurtfuncs/sharehurt"
local reboundandabsorb = require "hurtfuncs/reboundandabsorb"
local is_onetimehurt = require "hurtfuncs/isonetimehurt"
local characterdeadbyhurt = require "hurtfuncs/characterdeadbyhurt"

local HitResult = require "hurtfuncs/hitresult"
local HurtType = require "hurtfuncs/hurttype"
local HurtPropertyType = require "hurtfuncs/hurtpropertytype"
local TalentTrigger = require "hurtfuncs/talenttrigger"
local BusinessType = require "businesstype"
local HitType = require "hurtfuncs/hittype"

local AttrType = require "datadefs/attrtype"
local CDOPCODE = require "datadefs/characterdataopcode"
local CD_TOTALTICK = CDOPCODE.CD_TOTALTICK
local CD_DIRECTION = CDOPCODE.CD_DIRECTION
local CD_TEAMSIDE = CDOPCODE.CD_TEAMSIDE
local CD_ATTACKER = CDOPCODE.CD_ATTACKER
local CD_ENERGYADD_ONHIT = CDOPCODE.CD_ENERGYADD_ONHIT

local theeffecthelper
local thedatahelper
local thehitlabel
local thestathelper
local thecharacterhelper

local get_repleffectforproperty = function(hurtprop)
    if hurtprop == HurtPropertyType.kNoProperty then return -1 end
    if hurtprop == HurtPropertyType.kHurtLei then return 142 end
    if hurtprop == HurtPropertyType.kHurtGuang then return 143 end
    if hurtprop == HurtPropertyType.kHurtHuo then return 144 end
    if hurtprop == HurtPropertyType.kHurtShui then return 145 end
    if hurtprop == HurtPropertyType.kHurtAn then return 146 end
    if hurtprop == HurtPropertyType.kHurtDu then return 147 end
    return -1
end

local function hurtfunc(source, target, param)
    if param.hpdelta == 0 then return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, 0 end

    theeffecthelper = theeffecthelper or g_effecthelper
    thedatahelper = thedatahelper or g_datahelper
    thehitlabel = thehitlabel or g_hitlabel
    thestathelper = thestathelper or g_stathelper
    thecharacterhelper = thecharacterhelper or g_characterhelper

    param.inithpdelta = param.inithpdelta or param.hpdelta

    local shiledabsorbhp = theeffecthelper.get_eff(265).check_do(target, param.hpdelta, param.is_simu)
    if shiledabsorbhp ~= 0 then
        param.hpdelta = param.hpdelta - shiledabsorbhp
        if param.hpdelta <= 0 then
            param.hpdelta = 0
            return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, 0
        end
    end

    if param.is_fromsacrificerecv == false then
        param.hpdelta = theeffecthelper.get_eff(242).check_do(source, target, param)
        if param.hpdelta == 0 then
            return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, 0
        end
    end

    if theeffecthelper.get_eff(172).has_activated_eff(target, aicontext:get_currenttick(), source) then 
        if transferhurt(source, target, param) then
            if param.is_simu then
                param.hpdelta = 0
            end
            return HitResult.HitResultType.HIT_HASHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
        end
    end

    if source:get_data(CD_TEAMSIDE) ~= target:get_data(CD_TEAMSIDE) and thecharacterhelper.is_cannon(source) ~= true and param.is_simu ~= true then 
        target:set_data(CD_ATTACKER, source)
    end

    if param.markid ~= nil and param.markid > 0 then 
        param.hpdelta = param.hpdelta - theeffecthelper.get_eff(259).check_do(target, param.markid, param.hpdelta)
        if param.hpdelta < 0 then param.hpdelta = 0 end 
    end 

    param.hpdelta = param.hpdelta + theeffecthelper.get_eff(243).check_do(source, param.hpdelta)
    param.hpdelta_beforeavoidhurt = param.hpdelta -- 计算减伤前伤害值

    if param.is_rawhurt ~= true then 
        param.hpdelta = param.hpdelta + theeffecthelper.get_eff(174).check_do(source, param.hurtprop, param.hpdelta, param.is_simu)

        param.hpdelta = get_originalavoidancehurt(param.hurttype, target, source, param.hpdelta)
        local is_targetfaceattacker = false 
        if source:get_data(CD_DIRECTION) ~= target:get_data(CD_DIRECTION) then
            is_targetfaceattacker = (target:get_attr(AttrType.X) - source:get_attr(AttrType.X)) * source:get_data(CD_DIRECTION) >= 0
        end

        local prophurtadd = 0
        if not param.is_fromeffect then
            prophurtadd = theeffecthelper.get_eff(207).check_do(target, param.hurtprop, param.hpdelta)
        end 
        
        if param.hurttype == HurtType.PHYSICAL_HURT_TYPE then 
            param.hpdelta = param.hpdelta + theeffecthelper.get_eff(117).check_do(target, param.hpdelta)
            if is_targetfaceattacker then 
                param.hpdelta = param.hpdelta + theeffecthelper.get_eff(190).check_do(target, param.hpdelta)
            end 
        elseif param.hurttype == HurtType.MAGICAL_HURT_TYPE then 
            param.hpdelta = param.hpdelta + theeffecthelper.get_eff(118).check_do(target, param.hpdelta)
            if is_targetfaceattacker then 
                param.hpdelta = param.hpdelta + theeffecthelper.get_eff(191).check_do(target, param.hpdelta)
            end 
        end 

        param.hpdelta = param.hpdelta + prophurtadd

        if is_targetfaceattacker then 
            param.hpdelta = param.hpdelta + theeffecthelper.get_eff(192).check_do(target, param.hpdelta)
        end 

        if param.hpdelta < 0 then param.hpdelta = 0 end 

        if param.is_fromeffect ~= true and source:is_summon() then 
            -- param.hpdelta = param.hpdelta * source:get_summonproperty().attackrate / 100  -- TODO
            local CD_SUMMON_PROP_ID = CDOPCODE.CD_SUMMON_PROP_ID
            local summonpropid = source:get_data(CD_SUMMON_PROP_ID)
            if summonpropid ~= nil and summonpropid > 0 then
                local summonpropdata = thedatahelper.get_summonpropdata(summonpropid)
                if summonpropdata ~= nil then
                    param.hpdelta = param.hpdelta * summonpropdata["conf"][22] / 100
                end
            end
            if source:get_fakesummontype() == 0 then
                param.hpdelta = param.hpdelta + theeffecthelper.get_eff(173).check_do(target, param.hpdelta)
            end
        end 

        if target:has_effect(149) then 
            param.hpdelta = theeffecthelper.get_eff(149).check_do(target, param.hpdelta)
        end 

        --免伤优先级
        -- 伤害阈值 > 林肯法球 > 物理/法术伤害免疫 > 伤害吸收护盾 > 属性伤害免疫

        local lowlimit = theeffecthelper.get_eff(141).check_do(target, target:get_attr(AttrType.HPTop))
        if lowlimit > 0 and param.hpdelta < lowlimit then 
            return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
        end 

        if param.hurttype == HurtType.PHYSICAL_HURT_TYPE or param.hurttype == HurtType.MAGICAL_HURT_TYPE then 
            lowlimit = theeffecthelper.get_eff(271).check_do(target, target:get_attr(AttrType.HPTop))
            if lowlimit > 0 and param.hpdelta < lowlimit then 
                if param.is_simu ~= true then 
                    theeffecthelper.get_eff(271).trigger(target)
                end 
                return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
            end 
        end 

        if theeffecthelper.get_eff(178).trigger(source, target, param.is_simu) then 
            return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
        end 

        local is_bypassed = false
        local hurttype = param.hurttype
        local repelnum = 0
        
        local is_bama = false

        if hurttype == HurtType.PHYSICAL_HURT_TYPE then 
            is_bama = true
            repelnum = theeffecthelper.get_eff(110).check_do(target, param.hpdelta, param.is_simu)
        elseif hurttype == HurtType.MAGICAL_HURT_TYPE then 
            is_bama = true
            repelnum = theeffecthelper.get_eff(111).check_do(target, param.hpdelta, param.is_simu)
        end 

        if repelnum > 0 and repelnum >= param.hpdelta then 
            is_bypassed = true
        end 

        param.hpdelta = param.hpdelta - repelnum 
        if param.hpdelta <= 0 then 
            if is_bypassed then return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
            else return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
            end 
        end 

        -- 15.0
        if is_bama and target:has_effect(312) then
            is_bypassed = false
            repelnum = theeffecthelper.get_eff(312).trigger(target, hurttype, param.hpdelta, param.is_simu)
            if repelnum > 0 and repelnum >= param.hpdelta then 
                is_bypassed = true
            end 

            param.hpdelta = param.hpdelta - repelnum 
            if param.hpdelta <= 0 then 
                if is_bypassed then return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
                else return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
                end 
            end 
        end

        -- OP2
        if target:has_effect(320) then
            is_bypassed = false
            repelnum = theeffecthelper.get_eff(320).trigger(target, hurttype, param.hurtprop, param.hpdelta, param.is_simu)
            if repelnum > 0 and repelnum >= param.hpdelta then 
                is_bypassed = true
            end 

            param.hpdelta = param.hpdelta - repelnum 
            if param.hpdelta <= 0 then 
                if is_bypassed then return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
                else return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
                end 
            end 
        end

        is_bypassed = false
        repelnum = theeffecthelper.get_eff(112).check_do(target, param.hpdelta, param.is_simu)
        if repelnum > 0 and repelnum >= param.hpdelta then
            is_bypassed = true
        end
        param.hpdelta = param.hpdelta - repelnum
        if param.hpdelta <= 0 then
            return is_bypassed and HitResult.HitResultType.HIT_BYPASS or HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
        end

        local repel = 0
        if hurttype == HurtType.PHYSICAL_HURT_TYPE then 
            repel = theeffecthelper.get_eff(138).check_do(target, param.hpdelta)
        elseif hurttype == HurtType.MAGICAL_HURT_TYPE then
            repel = theeffecthelper.get_eff(139).check_do(target, param.hpdelta)
        end 
        
        if repel > 0 then 
            if repel >= param.hpdelta then
                return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
            end 
            param.hpdelta = param.hpdelta - repel
        end 

        if target:has_effect(273) then 
            repel = theeffecthelper.get_eff(273).trigger_withparam(target, param.hpdelta, param.is_simu)
            if repel >= param.hpdelta then 
                return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
            end 
            param.hpdelta = param.hpdelta - repel
        end 

        local highlimit = theeffecthelper.get_eff(154).check_do(target, target:get_attr(AttrType.HPTop))
        if param.skillid and param.skillid > 0 and thedatahelper.is_skill_ignoreeffect(param.skillid, 154) then 
            highlimit = 0
        end 
        if highlimit > 0 and param.hpdelta > highlimit then 
            param.hpdelta = highlimit
        end 

        local is_blocked = false
        local blockeffid = get_repleffectforproperty(param.hurtprop)

        if blockeffid > 0 then 
            is_blocked = theeffecthelper.get_eff(blockeffid).check_do(target, 0) > 0
        end 

        if is_blocked ~= true then 
            if theeffecthelper.get_eff(226).check_do(target) > 0 then 
                is_blocked = true 
                if param.is_simu ~= true then 
                    theeffecthelper.get_eff(226).change(target, 1)
                end 
            end 
        end 

        if is_blocked ~= true and hurttype ~= HurtType.HEALING_HURT_TYPE and hurttype >= 0 then 
            local shieldeffid = hurttype == HurtType.PHYSICAL_HURT_TYPE and 157 or 158

            if theeffecthelper.get_eff(shieldeffid).check_do(target) > 0 then 
                is_blocked = true
                if param.is_simu ~= true then 
                    theeffecthelper.get_eff(shieldeffid).change(target, 1)
                end 
            elseif theeffecthelper.get_eff(159).check_do(target) > 0 then 
                is_blocked = true
                if param.is_simu ~= true then 
                    theeffecthelper.get_eff(159).change(target, 1)
                end 
            end 
        end 

        if is_blocked then 
            param.hpdelta = 0
            return HitResult.HitResultType.HIT_HASHURT, HitResult.BlockType.HIT_BLOCKED, param.hpdelta
        end 

        if param.is_simu ~= true then 
            checkandexecutetalent(source, target, TalentTrigger.HIT)
        end 

        if param.hurtid ~= nil and param.hurtid > 0 and param.is_simu ~= true then 
            local hurtdata = thedatahelper.get_hurtdata(param.hurtid)
            source:set_actionatt(9, hurtdata.hitstunp)
            source:set_data(CD_TOTALTICK, source:get_data(CD_TOTALTICK) + hurtdata.hitstunp)
        end 

        if param.is_fromeffect ~= true and target:is_summon() then 
            param.hpdelta = param.hpdelta * target:get_summon_beattackrate() / 100
        end 
    end -- is_rawhurt

    local resistnreboundval = theeffecthelper.get_eff(211).check_do(target, param.hpdelta)
    if resistnreboundval > 0 then 
        if param.is_simu then
            param.hpdelta = 0
        else
            local reboundparam = {}
            for k,v in pairs(param) do 
                reboundparam[k] = v
            end 
            reboundparam.is_skiprebound = true
            reboundparam.hpdelta = resistnreboundval
            reboundparam.stathurtsourcealt = target

            theeffecthelper.get_eff(211).trigger(target, source, param.skillid, param.skill_level)
            theeffecthelper.get_eff(211).clean(target)
            
            hurtfunc(source, source, reboundparam) -- MARK

            if source:get_attr(AttrType.HP) <= 0 then 
                characterdeadbyhurt(source, source, param.skillid, param.skill_level, 0, 0)
            end 
        end 

        return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_BLOCKED, param.hpdelta
	end
	
	-- 326
	if target:has_effect(326) then
		if param.is_simu then
			param.hpdelta = 0
		else
			theeffecthelper.get_eff(326).trigger(source, target, param.skillid, param.skill_level)
			aieffectcaster.zero_special_effect(target, 326)
		end
		return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_BLOCKED, param.hpdelta
	end

    -- OP2
    if target:has_effect(10001) then
        local repelnum = theeffecthelper.get_eff(10001).trigger(target, param.hurttype, param.hpdelta, param.is_simu, param.hittype)
        if repelnum ~= 0 then
            param.hpdelta = param.hpdelta - repelnum
            if param.hpdelta <= 0 then
                param.hpdelta = 0
                return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, 0
            end
        end
    end

    param.hpdelta = theeffecthelper.get_eff(179).trigger(target, param.hpdelta, param.is_simu)

    param.hpdelta = sharehurt(source, target, param)

    local realhpdelta = param.hpdelta

    local tmphpdelta = theeffecthelper.get_eff(102).check_do(target, param.hpdelta)
    if tmphpdelta > 0 then 
        if param.is_simu ~= true then 

            if target:has_effect(316) then
                local repltmp = theeffecthelper.get_eff(316).trigger(target, tmphpdelta)
                if tmphpdelta > repltmp then
                    tmphpdelta = tmphpdelta - repltmp
                else
                    tmphpdelta = 0
                end
            end

            if tmphpdelta ~= 0 then 
                local oldhp = target:get_attr(AttrType.HP)
                target:update_hp(oldhp + tmphpdelta)

                local hitlabel = 100002
                if param.hittype ~= HitType.NORMAL_HIT then hitlabel = 100003 end
                thehitlabel.execute(hitlabel, tmphpdelta, target)

                local the_real_tmphpdelta = target:get_attr(AttrType.HP) - oldhp

                if thestathelper ~= nil then 
                    local stat_source = source
                    if param.stathurtsourcealt ~= nil then stat_source = param.stathurtsourcealt end 
                    thestathelper.append_hurt(stat_source, HurtType.HEALING_HURT_TYPE, the_real_tmphpdelta, target)
                end 
    
                target:on_hpdelta(the_real_tmphpdelta)
            end 

            -- if the_real_tmphpdelta < 0 then
            --     theeffecthelper.get_eff(288).on_hurt(target, source, -the_real_tmphpdelta, param.inithpdelta)
            -- end
        end 
    elseif param.hpdelta ~= 0 then
        local is_hurttransfered = false
        if cantransferhurt(target, param.hpdelta) then 
            is_hurttransfered = transferhurt(source, target, param)
        end 

        if is_hurttransfered == true then 
            realhpdelta = 0
            param.hpdelta = 0
        else
            if theeffecthelper.get_eff(272).can_trigger(target, param.hpdelta) then 
                if param.is_simu ~= true then 
                    theeffecthelper.get_eff(272).trigger(target)
                    if theeffecthelper.get_eff(272).can_clean(target) then
                        theeffecthelper.clean_eff(target, 272)
                    end 
                end 

                param.hpdelta = target:get_attr(AttrType.HP) - 1
            end -- 272

            if theeffecthelper.get_eff(247).can_trigger(target, param.hpdelta) then 
                if param.is_simu ~= true then 
                    theeffecthelper.get_eff(247).trigger(target)
                    theeffecthelper.clean_eff(target, 247)
                end 

                param.hpdelta = 0
                
                if theeffecthelper.get_eff(272).check_do(target, 0) <= 0 then
                    -- 272,247同时存在
                    return HitResult.HitResultType.HIT_BYPASS, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
                end 
            end -- 247

            local extrahurt = theeffecthelper.get_eff(269).get_extrahurt(source, param.hpdelta)
            if param.is_simu then 
                param.hpdelta = param.hpdelta + extrahurt
                if target:has_effect(288) then
                    local hpreduce = theeffecthelper.get_eff(288).check_do_simu(target, source, param.hpdelta)
                    if hpreduce ~= 0 then
                        param.hpdelta = param.hpdelta - hpreduce
                    end
                end
            else
                local hpreduce_288 = 0
                if target:has_effect(288) then
                    hpreduce_288 = theeffecthelper.get_eff(288).check_do(target, source, param.hpdelta + extrahurt)
                    if hpreduce_288 >= param.hpdelta + extrahurt then
                        return HitResult.HitResultType.HIT_NOHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
                    elseif hpreduce_288 > 0 then
                        if extrahurt > 0 then
                            if extrahurt >= hpreduce_288 then
                                extrahurt = extrahurt - hpreduce_288
                            else
                                param.hpdelta = param.hpdelta + extrahurt - hpreduce_288
                                extrahurt = 0
                            end
                        else
                            param.hpdelta = param.hpdelta - hpreduce_288
                        end
                    end
                end

                -- the core hurt logic
                local oldhp = target:get_attr(AttrType.HP)
                target:update_hp(oldhp - param.hpdelta - extrahurt)

                local thehostconfig = thehostconfig or g_hostconfig
                
                if aicontext.get_businesstype() ~= BusinessType.CANNON_KING and param.is_ignorehitlabel ~= true and param.hpdelta ~= 0 then 
                    if param.hpdelta > 0 or not thehostconfig.is_ignore_positive_hurtvalue_hitlabel_and_stat then
                        local hitlabel = param.hittype
                        if param.alternatehitlabel ~= nil and param.alternatehitlabel > 0 then hitlabel = param.alternatehitlabel end 
                        thehitlabel.execute(hitlabel, -param.hpdelta, target)
                    end
                end 

                if extrahurt ~= 0 then 
                    local hitlabel = theeffecthelper.get_eff(269).get_hitlabel(source)
                    if hitlabel ~= nil and hitlabel ~= 0 then 
                        if extrahurt > 0 or not thehostconfig.is_ignore_positive_hurtvalue_hitlabel_and_stat then
                            thehitlabel.execute(hitlabel, -extrahurt, target)
                        end
                    end 
                end 

                realhpdelta = oldhp - target:get_attr(AttrType.HP)

                --stat
                if thestathelper ~= nil then
                    local stat_hurttype = param.hurttype
                    if not stat_hurttype or stat_hurttype < 0 then stat_hurttype = HurtType.PHYSICAL_HURT_TYPE end 
                    local stat_source = source
                    if param.stathurtsourcealt ~= nil then stat_source = param.stathurtsourcealt end 
                    if realhpdelta > 0 or not thehostconfig.is_ignore_positive_hurtvalue_hitlabel_and_stat then
                        thestathelper.append_hurt(stat_source, stat_hurttype, realhpdelta, target)
                    end
                end 

                target:on_hpdelta(-realhpdelta)
                
                -- if realhpdelta > 0 then
                --     theeffecthelper.get_eff(288).on_hurt(target, source, realhpdelta, param.inithpdelta)
                -- end

                param.hpdelta = param.hpdelta + extrahurt
                if param.is_skiprebound ~= true then 
                    reboundandabsorb(source, target, param)
                end 

                local casterid = param.casterid
                if realhpdelta > 0 and is_onetimehurt(casterid, param.innereffect) then
                    target:set_tag("lastonetimehurt", realhpdelta)
                    if target:has_effect(287) then
                        theeffecthelper.get_eff(287).on_onetimehurt(target)
                    end
                end
            end -- is_simu
        end -- is_hurttransfered
    end -- hpdelta
    
    if param.is_simu then 
        return HitResult.HitResultType.HIT_HASHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta
    end 

    local energyadd = realhpdelta / target:get_attr(AttrType.HPTop) * 100
    if energyadd > 99.0 then energyadd = 99.0 end
    if energyadd < 0 then energyadd = 0 end

    if target:has_tag("energyskill") ~= true then 
        local endelta = target:get_data(CD_ENERGYADD_ONHIT) * energyadd
        endelta = endelta + theeffecthelper.get_eff(208).check_do(target, endelta)
        endelta = endelta + theeffecthelper.get_eff(175).check_do(source, endelta)
        if target:has_effect(315) then
            endelta = endelta - g_effecthelper.get_eff(315).trigger(target, endelta)
            if endelta < 0 then endelta = 0 end
        end
        local energy = target:get_attr(AttrType.Energy)
        target:update_energy(energy + endelta)
    end 

    return HitResult.HitResultType.HIT_HASHURT, HitResult.BlockType.HIT_UNBLOCKED, param.hpdelta

end -- hurtfunc

return hurtfunc