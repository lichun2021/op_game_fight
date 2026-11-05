-- transferhurt

local AttrType = require "datadefs/attrtype"

local thehurthelper
local theeffecthelper
local theactionhelper
local theeff172

return function(source, transfer, param)
    theeffecthelper = theeffecthelper or g_effecthelper
    theeff172 = theeff172 or theeffecthelper.get_eff(172)

    local currentworldtick = aicontext:get_currenttick()
    local valideff = theeff172.get_valideff(transfer, currentworldtick, source)
    if valideff == nil then return false end 
    if param.is_simu == true then return true end 

    thehurthelper = thehurthelper or g_hurthelper

    local receiver = valideff:get_origin()
    thehurthelper.hurttarget(source, receiver, param)

    theeff172.trigger(transfer, currentworldtick, source, valideff)

    theactionhelper = theactionhelper or g_actionhelper
    if receiver:get_attr(AttrType.HP) <= 0 and theactionhelper.is_deadaction(receiver:get_currentaction()) ~= true then 
        thehurthelper.characterdeadbyhurt(receiver, source, param.skillid, param.skill_level, nil, nil)
    end 

    return true
end