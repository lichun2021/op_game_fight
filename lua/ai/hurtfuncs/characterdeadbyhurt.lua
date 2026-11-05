-- characterdeadbyhurt

local AttrType = require "datadefs/attrtype"
local CD_CURRENTACTION = g_cddata.CD_CURRENTACTION
local CD_KILLER = g_cddata.CD_KILLER
local CD_KILLER_SKILL_ID = g_cddata.CD_KILLER_SKILL_ID

return function(target, source, skillid, skill_level, hurtid, innereffid, innereffdata)
    if g_actionhelper.is_deadaction(target:get_data(CD_CURRENTACTION)) then return end 
    
    target:set_data(CD_KILLER, source)
    target:set_data(CD_KILLER_SKILL_ID, skillid)
    if hurtid ~= nil and hurtid > 0 then 
        local hurtdata = g_datahelper.get_hurtdata(hurtid)
        local deadtype = 1
        if aicontext.get_randomnumber() < hurtdata.dtrate then
            deadtype = hurtdata.deadtype
        end
        g_actionhelper.deadaction.enter(target, deadtype, false)
        if g_actionhelper.is_deadaction(target:get_data(CD_CURRENTACTION)) then 
            for i=1, #hurtdata.hitdeadexecids do
                local execid = hurtdata.hitdeadexecids[i]
                if execid ~= nil and execid > 0 then
                    g_exechelper.execute(execid, skillid, source:get_attr(AttrType.Lv), source, nil, target, 0, false, 0)
                end
            end
        end 
    elseif innereffid ~= nil and innereffid > 0 then 
        -- local is_fatalhit = innereffid == 160
        -- g_actionhelper.deadaction.enter(target, 1, is_fatalhit)

        -- local innereffectdata = g_datahelper.get_innereffectdata(innereffid) -- TODO
        -- if g_actionhelper.is_deadaction(target:get_data(CD_CURRENTACTION)) then 
        --     for i=1, #innereffectdata.hitdeadexecids do
        --         local execid = innereffectdata.hitdeadexecids[i]
        --         g_exechelper.execute(execid, skillid, source:get_attr(AttrType.Lv), source, nil, target, 0, false, 0)
        --     end
        -- end 

        -- if g_actionhelper.is_deadaction(target:get_data(CD_CURRENTACTION)) and innereffhitdeadexecids ~= nil then
        --     for i=1, #innereffhitdeadexecids do
        --         local execid = innereffhitdeadexecids[i]
        --         g_exechelper.execute(execid, skillid, source:get_attr(AttrType.Lv), source, nil, target, 0, false, 0)
        --     end
        -- end
    elseif innereffdata ~= nil then
        local effid = innereffdata.ID
        local is_fatalhit = effid == 160
        g_actionhelper.deadaction.enter(target, 1, is_fatalhit)

        if g_actionhelper.is_deadaction(target:get_data(CD_CURRENTACTION)) then 
            for i=1, #innereffdata.hitdeadexecids do
                local execid = innereffectdata.hitdeadexecids[i]
                if execid ~= nil and execid > 0 then
                    g_exechelper.execute(execid, skillid, source:get_attr(AttrType.Lv), source, nil, target, 0, false, 0)
                end
            end
        end 
    else
        g_actionhelper.deadaction.enter(target, 1)
    end 
end
