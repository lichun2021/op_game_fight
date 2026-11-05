-- locktarget

local AttrType = require "datadefs/attrtype"

local theeffecthelper

local function distance_to_character(chara1, chara2)
--[[
        int x = chara1->getX() - chara2->getX();
        int y = chara1->getY() - chara2->getY();
        int distance = sqrt(x*x + y*y);
        return distance;
]]
    return aiactioncontroller.distance_to_character(chara1, chara2)
end

local function is_attacker_face_target(attacker, target)
    local direction_attacker = attacker:get_direction()
    local x_attacker = attacker:get_attr(AttrType.X)
    local x_target = target:get_attr(AttrType.X)

    if direction_attacker == 1 and x_attacker <= x_target then
        return true
    elseif direction_attacker == -1 and x_attacker >= x_target then
        return true
    end

    return false
end

local function locktarget(character, is_judgeby297)
    theeffecthelper = theeffecthelper or g_effecthelper

    if theeffecthelper.get_eff(248).check_do(character, 0) > 0 then return character end

    local duel_target = character:get_duel_target()
    -- 12.0
    -- if duel_target ~= nil then return duel_target end

    local currenttarget = character:get_currenttarget()

    if character:is_currenttargetconfirmed() == true then 
        -- 12.0
        if duel_target and currenttarget and duel_target:get_myid() ~= currenttarget:get_myid() then
            return nil
        end
        --
        return currenttarget 
    end

    if character:is_in_taunt() == true then 
        -- 12.0
        if duel_target and currenttarget and duel_target:get_myid() ~= currenttarget:get_myid() then
            return nil
        end
        --
        -- return currenttarget --OP2
        return character:get_taunttarget()
    end

    local is_taunt_extra = false

    if theeffecthelper.get_eff(253).check_do(character, 0) > 0 or character:has_tag("summonincharm253") == true then
        is_taunt_extra = true
    else
        if character:is_in_charm() == true or character:is_in_half_charm() == true then
            -- do nothing
        else
            if character:has_effect(280) then
                local target_280 = character:get_target_for_focus()
                if target_280 ~= nil and g_actionhelper.is_deadaction(target_280:get_currentaction()) ~= true then
                    return target_280
                end
            end
        end
    end

    local targets = nil
    if is_taunt_extra == true then
        targets = character:get_tmp_friends()
    else
        targets = character:get_tmp_rivals()
    end

    if targets == nil then return nil end

    local rettarget = nil -- 混合最近目标
    local tnormal = nil -- 非幻象最近目标

    local distance = 100000000
    local distnormal = distance
    local is_chas313 = character:has_effect(313) -- 优先搜索最近的非幻象目标
    for i=1,#targets do
        local atarget = targets[i]
        if is_taunt_extra == true and atarget:get_myid() == character:get_myid() then
            -- do nothing
        elseif not atarget:canbeattack(character, false) then
            -- do nothing
        elseif is_judgeby297 and atarget:has_effect(297) and not theeffecthelper.get_eff(297).is_findable(atarget, character) then
            -- do nothing
        elseif duel_target and duel_target:get_myid() ~= atarget:get_myid() then -- 12.0
            -- do nothing
        else
            if character:has_effect(293) ~= true or is_attacker_face_target(character, atarget) then
                local dist = distance_to_character(character, atarget)
                if dist < distance then
                    distance = dist
                    rettarget = atarget
                end

                if is_chas313 then
                    if not atarget:is_summon() then
                        if dist < distnormal then
                            distnormal = dist
                            tnormal = atarget
                        end
                    end
                end
            end
        end
    end

    if is_chas313 and tnormal then
        return tnormal
    end

    return rettarget
end

return locktarget