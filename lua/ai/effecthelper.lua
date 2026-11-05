-- effecthelper
local characterdeadbyhurt = require "hurtfuncs/characterdeadbyhurt"
local AttrType = require "datadefs/attrtype"
local CasterType = require "datadefs/castertype"

local theaieffectcaster = aieffectcaster

local effs = {}

local function get_eff(effid)
    -- effid = math.floor(effid)
    local theeff = effs[effid]
    if theeff then return theeff end

    theeff = require ("effects/eff"..tostring(effid))
    effs[effid] = theeff
    return theeff
end

local function clean_eff(character, effid)
    theaieffectcaster.zero_special_effect(character, effid)
end

local function is_anyinnerexec_incd(target, hitid)
    theaieffectcaster.is_anyinnerexec_incd(target, hitid)
end

local function innereffect_check_delayed(effectcaster, character, i)
    -- if not character:has_effect(292) then return false end
    return get_eff(292).check_eff_delayed(effectcaster, character, i + 1)
end

local function innereffect_is_delayed(effectcaster, character, i)
    -- if not character:has_effect(292) then return false end
    return get_eff(292).is_eff_delayed(effectcaster, character, i + 1)
end

local function innereffect_apply_hpchange(source, projectile, target, skillid, skilllevel, attridx, changeval, markid, casterid, eff)
    local realchangeval = theaieffectcaster.innereffect_apply_hpchange(source, projectile, target, skillid, skilllevel, attridx, changeval, markid, casterid, eff)
    if changeval > 0 and realchangeval <= 0 and target:get_attr(AttrType.HP) <= 0 then
        characterdeadbyhurt(target, source, skillid, skilllevel, 0, 0)
    elseif changeval < 0 and realchangeval == 1 and target:has_effect(103) and eff then
        -- MARK:防止效果执行击杀目标但因103不死效果而出现加血的击中文字
        return 0
    end
    return realchangeval
end

local function is_group_covered(source, target, casterid)
    -- local effectexec = g_datahelper.get_effectdata(casterid)
    -- if not effectexec then return false end

    -- local groupdata = effectexec["group"]
    -- if not groupdata then return false end

    -- local groupid = groupdata[1]
    -- local groupcoverrule = groupdata[2]
    -- local groupweight = groupdata[3]

    -- if groupid <= 0 then return false end

    -- local casters = target:get_effect_casters()
    -- if not casters then return false end

    -- for i=1,#casters do
    --     local a_caster = casters[i]
    --     -- ...
    -- end

    return theaieffectcaster.is_caster_group_covered(target, casterid)
end

local function check_cover_group_casters(target, casterid)
    --[[
    local effectexec = g_datahelper.get_effectdata(casterid)
    if not effectexec then return end

    local groupdata = effectexec["group"]
    if not groupdata then return end

    local groupid = groupdata[1]
    local groupcoverrule = groupdata[2]
    local groupweight = groupdata[3]

    if groupid <= 0 then return end

    local casters = target:get_effect_casters()
    if not casters then return end

    thecachehelper = thecachehelper or g_cachehelper
    local character_myid = target:get_myid()
    local caster_type = CasterType.eEffectExec

    for i=1,#casters do
        local a_caster = casters[i]
        local a_casterid = a_caster:get_effectid()
        local a_caster_exec = g_datahelper.get_effectdata(a_casterid)
        if a_caster_exec and a_caster_exec["group"] then
            local a_caster_group = a_caster_exec["group"]
            local a_caster_groupid = a_caster_group[1]
            if groupid == a_caster_groupid then
                local a_caster_groupweight = a_caster_group[3]
                local is_caster_covered = false
                if groupweight > a_caster_groupweight then
                    is_caster_covered = true
                elseif groupweight == a_caster_groupweight then
                    if groupcoverrule == 1 then -- 1-相同权重，后上的生效
                        is_caster_covered = true
                    end
                end
                
                if is_caster_covered then
                    local caster_myid = a_caster:get_myid()
                    a_caster:cleanup_innereffect()

                    local the_to_delete_cache = thecachehelper.cache.to_delete_casters
                    if not the_to_delete_cache then
                        the_to_delete_cache = {}
                        thecachehelper.cache.to_delete_casters = the_to_delete_cache
                    end

                    the_to_delete_cache[#the_to_delete_cache + 1] = { character_myid,caster_type,caster_myid }
                end
            end
        end
    end
    --]]

    local coveredcnt = theaieffectcaster.check_cover_group_casters(target, casterid, true)
    if coveredcnt > 0 then
        g_aihelper.add_to_delete_covered_caster_cnt(coveredcnt)
    end
end

return {
    get_eff = get_eff,
    clean_eff = clean_eff,
    is_anyinnerexec_incd = is_anyinnerexec_incd,
    innereffect_check_delayed = innereffect_check_delayed,
    innereffect_is_delayed = innereffect_is_delayed,
    innereffect_apply_hpchange = innereffect_apply_hpchange,
    is_group_covered = is_group_covered,
    check_cover_group_casters = check_cover_group_casters
}
