-- 292 使指定效果失效
local get_innereffdata = require "effects/getinnereffdata"

local cachehelper
local thedatahelper

local function get_currentcache(self)
    cachehelper = cachehelper or g_cachehelper
    local eff292cache = cachehelper.cache.eff292 or {}
    cachehelper.cache.eff292 = eff292cache

    local uid = self:get_uid()
    local currentcache = eff292cache[uid] or {}
    eff292cache[uid] = currentcache
    
    return currentcache
end

local function clear_currentcache(self)
    cachehelper = cachehelper or g_cachehelper
    local eff292cache = cachehelper.cache.eff292
    if not eff292cache then return end

    local uid = self:get_uid()
    local currentcache = eff292cache[uid]

    if currentcache then
        eff292cache[uid] = nil
    end
end

local function temp_exit_eff(self, character, effid, exittype, currentcache)
    local theeffs = character:get_innereffects(effid)
    if theeffs == nil or #theeffs < 1 then return end

    local invalidateeffs = currentcache.invalidateeffs or {}
    currentcache.invalidateeffs = invalidateeffs

    for i=1,#theeffs do
        local aneff = theeffs[i]
        if aneff ~= nil and aneff:is_temp_invalid() ~= true then
            local aneffdata = get_innereffdata(aneff)

            local changevalue = aneff:get_changevalue()
            local is_temp_exit = false

            -- if aneffdata.changetype == 0 then
            --     -- 0-正数失效 1-负数 2-所有正负
            --     if exittype == 0 and changevalue > 0 then
            --         is_temp_exit = true
            --     elseif exittype == 1 and changevalue < 0 then
            --         is_temp_exit = true
            --     elseif exittype == 2 then
            --         is_temp_exit = true
            --     end
            -- elseif aneffdata.changetype == 1 then
            --     -- 3-大于100%失效 4-小于 5-所有百分比
            --     local thechangevalue = aneffdata.changevalue
            --     if exittype == 3 and thechangevalue > 10000 then
            --         is_temp_exit = true
            --     elseif exittype == 4 and thechangevalue < 10000 then
            --         is_temp_exit = true
            --     elseif exittype == 5 then
            --         is_temp_exit = true
            --     end
            -- end
            
            -- 0-正数失效 1-负数 2-所有正负
            -- 3-大于100%失效 4-小于 5-所有百分比
            if 2 == exittype then
                is_temp_exit = true
            elseif 5 == exittype then
                if aneffdata.changetype == 1 then
                    is_temp_exit = true
                end
            elseif 0 == exittype then
                if changevalue > 0 then is_temp_exit = true end
            elseif 1 == exittype then
                if changevalue < 0 then is_temp_exit = true end
            elseif 3 == exittype then
                if aneffdata.changetype == 1 and aneffdata.changevalue > 10000 then
                    is_temp_exit = true
                end
            elseif 4 == exittype then
                if aneffdata.changetype == 1 and aneffdata.changevalue < 10000 then
                    is_temp_exit = true
                end
            end

            if is_temp_exit then
                aneff:remove_ui_effect()
                aneff:remove_body_effect()

                if aneff:is_attr_effect() == true then
                    local attridx = aneffdata.targetattribute
                    if aneffdata.exitclear == 1 then
                        local realchangeval = aneff:get_attr_realchangevalue()
                        local aneffchangeval = aneff:get_changevalue()
                        aneff:deapply_attr_on_target(character, attridx, aneffchangeval, realchangeval)

                        local anefforigin = aneff:get_origin()
                        if aneffdata.type == 3 and character:get_myid() ~= anefforigin:get_myid() then
                            local realchangeval_origin = aneff:get_attr_realchangevalue_origin()
                            if realchangeval_origin ~= 0 then
                                aneff:deapply_attr_on_target(anefforigin, attridx, -aneffchangeval, realchangeval_origin)
                            end
                        end
                    end
                end

                aneff:set_temp_invalid(true)
                aneff:set_valid(false)
                invalidateeffs[aneff:get_uid()] = true
            end

        end
    end
end -- temp_exit_eff

local function temp_enter_eff(self, character, effid, exittype, currentcache)
    if currentcache and currentcache.invalidateeffs then
        local invalidateeffs = currentcache.invalidateeffs
        local theeffs = character:get_innereffects(effid)
        if theeffs == nil or #theeffs < 1 then return end

        for i=1,#theeffs do
            local aneff = theeffs[i]
            if aneff and invalidateeffs[aneff:get_uid()] then
                aneff:cast_ui_effect(3600)
                aneff:cast_body_effect(3600)
                aneff:set_temp_invalid(false)
                aneff:set_valid(true)

                if aneff:is_attr_effect() == true then
                    local aneffdata = get_innereffdata(aneff)
                    local attridx = aneffdata.targetattribute
                    local realchangeval = aneff:apply_attr_on_target(character, attridx, aneff:get_changevalue())
                    aneff:set_attr_realchangevalue(realchangeval)
                    local anefforigin = aneff:get_origin()
                    if aneffdata.type == 3 and realchangeval ~= 0 and character:get_myid() ~= anefforigin:get_myid() and aneffdata.ID ~= 252 then
                        local realchangeval_origin = aneff:apply_attr_on_target(anefforigin, attridx, -realchangeval)
                        aneff:set_attr_realchangevalue(realchangeval_origin)
                    end
                end
            end
        end
    end
end -- temp_enter_eff

local function cache_innereff(self, parentcaster, character, i)
    local currentcache = get_currentcache(self)
    local delayedeff = currentcache.delayedeff or {}
    currentcache.delayedeff = delayedeff

    delayedeff[#delayedeff + 1] = { parentcaster:get_myid(), i }
end

local function is_delayed(theeffdata, exittype)
    local is_temp_exit = false
    local thechangevalue = theeffdata.changevalue
    -- if theeffdata.changetype == 0 then
    --     -- 0-正数失效 1-负数 2-所有正负
    --     if exittype == 0 and thechangevalue > 0 then
    --         is_temp_exit = true
    --     elseif exittype == 1 and thechangevalue < 0 then
    --         is_temp_exit = true
    --     elseif exittype == 2 then
    --         is_temp_exit = true
    --     end
    -- elseif theeffdata.changetype == 1 then
    --     -- 3-大于100%失效 4-小于 5-所有百分比
    --     if exittype == 3 and thechangevalue > 10000 then
    --         is_temp_exit = true
    --     elseif exittype == 4 and thechangevalue < 10000 then
    --         is_temp_exit = true
    --     elseif exittype == 5 then
    --         is_temp_exit = true
    --     end
    -- end

    if 2 == exittype then
        is_temp_exit = true
    elseif 5 == exittype then
        if theeffdata.changetype == 1 then
            is_temp_exit = true
        end
    elseif 0 == exittype then
        if thechangevalue > 0 then is_temp_exit = true end
    elseif 1 == exittype then
        if thechangevalue < 0 then is_temp_exit = true end
    elseif 3 == exittype then
        if theeffdata.changetype == 1 and thechangevalue > 10000 then
            is_temp_exit = true
        end
    elseif 4 == exittype then
        if theeffdata.changetype == 1 and thechangevalue < 10000 then
            is_temp_exit = true
        end
    end

    return is_temp_exit
end

local function check_eff_delayed(effectcaster, character, idx)
    local effs = character:get_innereffects(292)
    if not effs then return false end

    thedatahelper = thedatahelper or g_datahelper
    local theeffectcasterid = effectcaster:get_effectid()
    local theeffdata = thedatahelper.get_innereffectdata(theeffectcasterid, 0, idx)
    local theeffid = theeffdata.ID

    for i=1,#effs do
        local an292 = effs[i]
        if an292 and not an292:is_temp_invalid() and not an292:is_erased() then
            local an292data = get_innereffdata(an292)
            local effparams = an292data.params

            for j=1,#effparams,2 do
                local aneffid = effparams[j]
                local atype = effparams[j+1]
                if theeffid == aneffid and is_delayed(theeffdata, atype) then
                    cache_innereff(an292, effectcaster, character, idx - 1)
                    return true
                end
            end
        end
    end
    return false
end

local function is_eff_delayed(effectcaster, character, idx)
    local effs = character:get_innereffects(292)
    if not effs then return false end

    thedatahelper = thedatahelper or g_datahelper
    local theeffectcasterid = effectcaster:get_effectid()
    local theeffdata = thedatahelper.get_innereffectdata(theeffectcasterid, 0, idx)
    local theeffid = theeffdata.ID

    for i=1,#effs do
        local an292 = effs[i]
        if an292 and not an292:is_temp_invalid() and not an292:is_erased() then
            local an292data = get_innereffdata(an292)
            local effparams = an292data.params

            for j=1,#effparams,2 do
                local aneffid = effparams[j]
                local atype = effparams[j+1]
                if theeffid == aneffid and is_delayed(theeffdata, atype) then
                    return true
                end
            end
        end
    end
    return false
end

local function cast_delayed_effs(self, character)
    local currentcache = get_currentcache(self)
    local delayedeff = currentcache.delayedeff
    if not delayedeff then return end

    for i=1,#delayedeff do
        local adelayedinfo = delayedeff[i]
        local aneffectcaster = character:get_effectcaster(adelayedinfo[1]) 
        if aneffectcaster and not check_eff_delayed(aneffectcaster, character, adelayedinfo[2] + 1) then
            aneffectcaster:cast_innereffect_by_idx(adelayedinfo[2], character)
        end
    end
end

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    -- self:set_changevalue(changevalue)

    local effparams = effdata.params
    local target = self:get_target()
    for i=1,#effparams,2 do
        local aneffid = effparams[i]
        local atype = effparams[i+1]
        temp_exit_eff(self, target, aneffid, atype, get_currentcache(self))
    end

    return true
end

local function on_exit(self)
    local effdata = get_innereffdata(self)
    local effparams = effdata.params
    local target = self:get_targetsafe()
    if target == nil then return end

    self:set_temp_invalid(true)

    for i=1,#effparams,2 do
        local aneffid = effparams[i]
        local atype = effparams[i+1]
        temp_enter_eff(self, target, aneffid, atype, get_currentcache(self))
    end

    cast_delayed_effs(self, target)

    clear_currentcache(self)
end

return {
    on_enter = on_enter,
    on_exit = on_exit,
    check_eff_delayed = check_eff_delayed,
    is_eff_delayed = is_eff_delayed
}