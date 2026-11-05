-- 284 根据飞行道具飞行距离触发执行
local get_innereffdata = require "effects/getinnereffdata"

local eff = {}

eff.on_enter = function (self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local projectile = self:get_projectile()
    if projectile == nil then return false end

    local projectileid = projectile:get_projectileid()
    local projectiledata = g_datahelper.get_projectiledata(projectileid)
    assert(projectiledata ~= nil, "projectile data is nil!")
    local throwtype = projectiledata.conf[2]
    if throwtype == 6 or throwtype == 7 or throwtype == 8 or throwtype == 9 then
        -- is bouncing projetile
    else
        return false
    end

    local dist = projectile:get_distancefromlasthit()
    local projectile_skillid = projectile:get_skillid()
    local projectile_skill_level = projectile:get_skilllevel()
    local projectile_origin = projectile:get_origin()

    -- 12.0
    local parentcaster = self:get_parentcaster()
    local casterdata = g_datahelper.get_effectdata(parentcaster:get_effectid())
    local alt_skill_id = 0
    local caster_altskillidconf = casterdata.conf[16]
    if caster_altskillidconf and #caster_altskillidconf >= 2 then
        if caster_altskillidconf[1] == 1 and caster_altskillidconf[2] > 0 then
            alt_skill_id = caster_altskillidconf[2]
        end
    end

    -- print(string.format("altskillid:%d", alt_skill_id))

    -- param:[[dist1, dist2, level1, level2, execid, skillid],...]
    local effdata = get_innereffdata(self)
    local effparams = effdata.params

    for i=1,#effparams,6 do
        local dist1 = effparams[i]
        local dist2 = effparams[i+1]
        local level1 = effparams[i+2]
        local level2 = effparams[i+3]
        local execid = effparams[i+4]
        local skillid = alt_skill_id > 0 and alt_skill_id or math.floor(effparams[i+5])
        
        local trig_skillid = projectile_skillid
        local trig_skilllevel = projectile_skill_level
        local can_trig = false
        if skillid > 0 then
            -- print(string.format("check has skill:%d", skillid))
            if projectile_origin:has_skill(skillid) then
                trig_skillid = skillid
                trig_skilllevel = projectile_origin:get_skilllevel(skillid)
                -- print(string.format("284 trig skill,%d,lv,%d", skillid, trig_skilllevel))
                can_trig = (dist >= dist1 and dist <= dist2 and trig_skilllevel >= level1 and trig_skilllevel <= level2)
            end
        else
            can_trig = (dist >= dist1 and dist <= dist2 and trig_skilllevel >= level1 and trig_skilllevel <= level2)
        end
        if can_trig then
            -- print(string.format("trig exec:%d", execid))
            g_exechelper.execute(execid, trig_skillid, trig_skilllevel, projectile_origin, projectile, target, 0, false, 0)
        end
    end

    return true
end

eff.on_exit = function(self)
end

return eff