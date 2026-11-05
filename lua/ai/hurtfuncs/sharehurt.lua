-- sharehurt

local AttrType = require "datadefs/attrtype"
local CD_MYID = g_cddata.CD_MYID
local CD_CURRENTACTION = g_cddata.CD_CURRENTACTION
local CD_TEAMSIDE = g_cddata.CD_TEAMSIDE

local theeffecthelper

return function (source, target, param)
    if param.hpdelta <= 0 then return param.hpdelta end 
    if target:has_effect(180) ~= true then return param.hpdelta end 

    theeffecthelper = theeffecthelper or g_effecthelper
    local sharetargets = {}
    -- local targetfriends = aicontext.get_clusterorigin(target:get_data(CD_TEAMSIDE), target:get_val("clusternum"))
    local targetfriends = target:get_currentfriendlist()
    for i=1, #targetfriends do 
        local a_character = targetfriends[i]
        if a_character:get_data(CD_MYID) == target:get_data(CD_MYID) or g_actionhelper.is_deadaction(a_character:get_data(CD_CURRENTACTION)) then
            -- do nothing
        else
            if theeffecthelper.get_eff(180).get_firstvalideff(a_character) ~= nil then 
                -- table.insert(sharetargets, a_character)
                sharetargets[#sharetargets + 1] = a_character
            end 
        end
    end 

    if #sharetargets < 1 then return param.hpdelta end 

    local hpdelta = param.hpdelta
    local rethurt = 0

    local avghurt = hpdelta / (#sharetargets + 1)
    avghurt = math.floor( avghurt )
    if avghurt < 1 then avghurt = 1 end 

    rethurt = rethurt + avghurt
    hpdelta = hpdelta - avghurt -- 先扣除原始目标伤害

    for i=1, #sharetargets do 
        if hpdelta <= 0 then break end 

        hpdelta = hpdelta - avghurt
        if param.is_simu == true then
            --[[do nothing]]
        else
            local a_character = sharetargets[i]
            theeffecthelper.get_eff(180).trigger(a_character)

            local shareparam = {}
            for k,v in pairs(param) do 
                shareparam.k = v 
            end 
            shareparam.hpdelta = avghurt
            shareparam.is_rawhurt = true
            shareparam.is_simu = param.is_simu

            g_hurthelper.hurttarget(source, a_character, shareparam)
            if a_character:get_attr(AttrType.HP) <= 0 then 
                g_hurthelper.characterdeadbyhurt(a_character, source, shareparam.skillid, shareparam.skill_level, shareparam.hurtid) -- TODO
            end 
        end
    end 

    if hpdelta < 0 then hpdelta = 0 end 

    return math.floor( rethurt + hpdelta )
end

