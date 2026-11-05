-- 288 统计目标受到的伤害，且免除部分伤害（如果填了免伤百分比）

-- 目标属性：统计ID
-- 效果改变值：记录免伤值使用的百分比大小，使用时除以100
-- = (数值大小 + (等级成长 x 技能等级) ) / 100
-- 如填10000，当免伤值为123，则记录的免伤值为 123 × 100%

local get_innereffdata = require "effects/getinnereffdata"
local AttrType = require "datadefs/attrtype"
local HurtParam = require "hurtfuncs/hurtparam"
local HurtType = require "hurtfuncs/hurttype"
local HitType = require "hurtfuncs/hittype"
local HitResult = require "hurtfuncs/hitresult"
local hurttarget = require "hurtfuncs/hurttarget"
local characterdeadbyhurt = require "hurtfuncs/characterdeadbyhurt"

local cachehelper
local theactionhelper
local theskillhelper

local eff = {}

eff.on_enter = function(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    self:set_innerdata("mid", effdata.targetattribute)
    self:set_innerdata("label", effdata.label)

    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)

    return true
end

-- eff.on_exit = function(self)
-- end

local function add_an_attacker(attackers, attacker_myid, hpdelta)
    if #attackers < 1 then
        attackers[#attackers + 1] = { attacker_myid, hpdelta }
        return
    end

    --{{myid,hpdelta}, ...}
    for i=1,#attackers do
        local adata = attackers[i]
        local an_attacker_myid = adata[1]
        if an_attacker_myid == attacker_myid then
            local an_attacker_hpdelta = adata[2]
            if hpdelta > an_attacker_hpdelta then
                adata[2] = hpdelta
            end
        end
    end
end

local function partition(arr, lo, hi)
    local pivot = arr[hi]
    local i = lo - 1
    local j = hi + 1
    while 1 do
        repeat i = i + 1
        until i >= hi or arr[i][2] <= pivot[2]

        repeat j = j - 1
        until j <= lo or arr[j][2] >= pivot[2]

        if i > j then return j end

        local tmp = arr[i]
        arr[i] = arr[j]
        arr[j] = tmp
    end
end

local function quick_sort(arr, lo, hi)
    if lo >= hi then return end

    local p = partition(arr, lo, hi)
    quick_sort(arr, lo, p)
    quick_sort(arr, p + 1, hi)
end

local function sort_attackers(attackers)
    if #attackers <= 1 then return end
    quick_sort(attackers, 1, #attackers)
end

local function get_charactercache(character)
    cachehelper = cachehelper or g_cachehelper
    local eff288cache = cachehelper.cache.eff288
    if not eff288cache then 
        eff288cache = {}
        cachehelper.cache.eff288 = eff288cache 
    end

    local c_myid = character:get_myid()
    local c_cache = eff288cache[c_myid]
    if not c_cache then
        c_cache = {}
        eff288cache[c_myid] = c_cache
    end

    return c_cache
end

local function get_currentcache(character, mid) -- mid:统计ID
    local c_cache = get_charactercache(character)
    local e_cache = c_cache[mid]
    if not e_cache then
        e_cache = {}
        c_cache[mid] = e_cache
    end

    return e_cache
end

local function try_get_currentcache(character, mid)
    local c_cache = get_charactercache(character)
    return c_cache[mid]
end

local function clear_currentcache(character, mid)
    local c_cache = get_charactercache(character)
    c_cache[mid] = nil
end

-- return absorbed hpdelta
eff.check_do = function(character, attacker, hpdelta)
    if character:get_teamside() == attacker:get_teamside() then return 0 end

    local effs = character:get_innereffects(288)
    if not effs then return 0 end

    local absorbedhpdelta = 0

    for i=1,#effs do
        local aneff = effs[i]
        local amid = aneff:get_innerdata("mid")

        local currentcache = get_currentcache(character, amid)

        local hpdeltareduce = currentcache.hpdeltareduce or 0

        local allhpdelta = currentcache.allhpdelta or 0
        allhpdelta = allhpdelta + hpdelta
        currentcache.allhpdelta = allhpdelta

        local mosthpdelta = currentcache.mosthpdelta or 0
        local mosthpdelta_attacker = currentcache.mosthpdelta_attacker
        if mosthpdelta < hpdelta then
            mosthpdelta = hpdelta
            mosthpdelta_attacker = attacker
        end
        currentcache.mosthpdelta = mosthpdelta
        currentcache.mosthpdelta_attacker = mosthpdelta_attacker

        local attackers = currentcache.attackers or {}
        currentcache.attackers = attackers
        add_an_attacker(attackers, attacker:get_myid(), hpdelta)
        sort_attackers(attackers)

        local absorbrate = aneff:get_changevalue() / 10000
        if absorbrate > 0 then
            absorbedhpdelta = math.floor( absorbedhpdelta + hpdelta * absorbrate )
        end

        hpdeltareduce = hpdeltareduce + absorbedhpdelta
        currentcache.hpdeltareduce = hpdeltareduce
    end

    return absorbedhpdelta
end

eff.check_do_simu = function(character, attacker, hpdelta)
    if character:get_teamside() == attacker:get_teamside() then return 0 end

    local effs = character:get_innereffects(288)
    if not effs then return 0 end

    local absorbedhpdelta = 0

    for i=1,#effs do
        local aneff = effs[i]
        local amid = aneff:get_innerdata("mid")

        local absorbrate = aneff:get_changevalue() / 10000
        if achangevalue > 0 then
            absorbedhpdelta = math.floor( absorbedhpdelta + hpdelta * absorbrate )
        end
    end

    return absorbedhpdelta
end

eff.on_hurt = function(character, attacker, hpdelta, inithpdelta)
    if character:get_teamside() == attacker:get_teamside() then return end

    local effs = character:get_innereffects(288)
    if not effs then return end

    for i=1,#effs do
        local aneff = effs[i]
        local amid = aneff:get_innerdata("mid")

        local currentcache = get_currentcache(character, amid)

        local allhpdelta = currentcache.allhpdelta or 0
        allhpdelta = allhpdelta + hpdelta
        currentcache.allhpdelta = allhpdelta

        local mosthpdelta = currentcache.mosthpdelta or 0
        local mosthpdelta_attacker = currentcache.mosthpdelta_attacker
        if mosthpdelta < hpdelta then
            mosthpdelta = hpdelta
            mosthpdelta_attacker = attacker
        end
        currentcache.mosthpdelta = mosthpdelta
        currentcache.mosthpdelta_attacker = mosthpdelta_attacker

        local attackers = currentcache.attackers or {}
        currentcache.attackers = attackers
        add_an_attacker(attackers, attacker:get_myid(), hpdelta)
        sort_attackers(attackers)

        local hpdeltareduce = currentcache.hpdeltareduce or 0
        if inithpdelta > hpdelta then
            local achangevalue = aneff:get_changevalue()
            hpdeltareduce = hpdeltareduce + (inithpdelta - hpdelta) * achangevalue / 10000
            hpdeltareduce = math.floor(hpdeltareduce)
        end
        currentcache.hpdeltareduce = hpdeltareduce
    end
end

local function get_mosthpdelta_attacker(character, mid)
    local currentcache = try_get_currentcache(character, mid)
    if not currentcache then return nil end
    
    local theattacker = currentcache.mosthpdelta_attacker
    theactionhelper = theactionhelper or g_actionhelper
    if theattacker and not theactionhelper.is_deadaction(theattacker:get_currentaction()) then
        return theattacker
    end
    local attackers = currentcache.attackers
    if not attackers then return nil end
    for i=1,#attackers do
        local attackermyid = attackers[i][1]
        local attacker = aicontext.get_character_by_myid(attackermyid)
        if attacker and not theactionhelper.is_deadaction(attacker:get_currentaction()) then
            return attacker
        end
    end
    return nil
end

eff.on_trigger = function(character, mid, hurttype, ratio, usetype, low, high, skillid, skill_level, triggercasterid, triggereff, label, target)
    local currentcache = try_get_currentcache(character, mid)
    if not currentcache then return end

    local allhpdelta = currentcache.allhpdelta or 0
    local hpdeltareduce = currentcache.hpdeltareduce or 0
    --local mosthpdelta_attacker = currentcache.mosthpdelta_attacker
    -- local mosthpdelta_attacker = get_mosthpdelta_attacker(character, mid)

    theactionhelper = theactionhelper or g_actionhelper

    -- if allhpdelta <= 0 then return clear_currentcache(character, mid) end
    -- if mosthpdelta_attacker == nil then return clear_currentcache(character, mid) end
    -- if theactionhelper.is_deadaction(mosthpdelta_attacker:get_currentaction()) then return clear_currentcache(character, mid) end
    if theactionhelper.is_deadaction(target:get_currentaction()) then return end

    local hpdelta = usetype == 1 and allhpdelta or hpdeltareduce
    hpdelta = hpdelta * ratio
    hpdelta = math.floor(hpdelta)
    if hpdelta < low then hpdelta = low end
    if hpdelta > high then hpdelta = high end
    -- if hpdelta <= 0 then return clear_currentcache(character, mid) end

    local param = HurtParam.new(nil, skillid, skill_level, hpdelta, hurttype, 0, HitType.NORMAL_HIT, 0, false, triggercasterid)
    param.is_fromeffect = true
    -- param.is_ignorehitlabel = true
    param.alternatehitlabel = label

    local source = character
    -- local target = mosthpdelta_attacker
    local hitresult, blockresult, hpdeltaresult = hurttarget(source, target, param)
    if blockresult ~= HitResult.BlockType.HIT_BLOCKED then
        target:on_hurt(true, 288, hpdeltaresult, hurttype, source, triggercasterid)
    end

    if target:get_attr(AttrType.HP) <= 0 then
        local effdata = get_innereffdata(triggereff)
        characterdeadbyhurt(target, source, skillid, skill_level, nil, effdata)
    else
        theskillhelper = theskillhelper or g_skillhelper
        theskillhelper.on_releasehpskill(target)
    end

    -- clear_currentcache(character, mid)
end

eff.get_first_mosthpdelta_attacker = function(character)
    local c_cache = get_charactercache(character)
    for k,v in pairs(c_cache) do
        local anattacker = get_mosthpdelta_attacker(character, k)
        if anattacker then return anattacker end
    end

    return nil
end

eff.on_clear_data = function(character, mid)
    clear_currentcache(character, mid)
end

return eff