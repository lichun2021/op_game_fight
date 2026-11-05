-- exechelper

local theaicontext = aicontext

-- false:host should not call native api
local function on_aiactionconroller_execute_exec(execid, skillid, skill_level, source, projectile, target, parentticklen, is_uselogicpos, markid)
    return true
end

local function is_effectexec(eid)
    return (eid >= 50000 and eid <= 59999) or (eid >= 500000 and eid <= 599999)
end

local function is_moveexec(eid)
    return (eid >= 30000 and eid <= 39999) or (eid >= 300000 and eid <= 399999)
end

local function is_hurtexec(eid)
    return (eid >= 20000 and eid <= 29999) or (eid >= 200000 and eid <= 299999)
end

local function is_projectileexec(eid)
    return (eid >= 40000 and eid <= 49999) or (eid >= 400000 and eid <= 499999)
end

local function execute(execid, skillid, skill_level, source, projectile, target, parenttick, is_uselogicpos, markid)
    theaicontext.execute_exec(execid, skillid, skill_level, source, projectile, target, parenttick, is_uselogicpos, markid)
end

local function is_castersyncattackspeed(casterid)
    if is_effectexec(casterid) then
        return false
    end
    return true
end

return {
    on_aiactionconroller_execute_exec = on_aiactionconroller_execute_exec,
    is_effectexec = is_effectexec,
    is_moveexec = is_moveexec,
    is_hurtexec = is_hurtexec,
    is_projectileexec = is_projectileexec,
    execute = execute,
    is_castersyncattackspeed = is_castersyncattackspeed
}