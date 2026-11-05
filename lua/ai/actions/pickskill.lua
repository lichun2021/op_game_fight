-- pickskill
local AttrType = require "datadefs/attrtype"

local function is_com(character)
    return character:get_teamside() == 2
end

local function get_attackseq_firstmatchindex(character, skillindex, is_useseq2)
    local characterdata = g_datahelper.get_characterdata(character:get_attr(AttrType.ID))
    local attackseq = is_useseq2 and characterdata.seq2 or characterdata.seq
    if attackseq == nil then return -1 end
    for i=1,#attackseq do
        if attackseq[i] == skillindex then
            return i - 1
        end
    end

    return -1
end

local function picknormalskill(character, attackseq, currentseqindex)
    local nextseqindex = currentseqindex + 1
    if nextseqindex < 0 or nextseqindex >= #attackseq then nextseqindex = 0 end

    if character:has_effect(231) then
        nextseqindex = g_effecthelper.get_eff(231).check_do(character, attackseq, nextseqindex)
    end
    
    return nextseqindex
end

local function picknormalskill_witheff294(character, attackseq, currentseqindex, eff294)
    local seqlen = #attackseq
    local is_firstround = true
    local nextseqindex = 0
    local pickcnt = 1
    while true do
        nextseqindex = picknormalskill(character, attackseq, currentseqindex)
        local skillidindex = attackseq[nextseqindex + 1]
        if eff294.is_skillblocked(character, skillidindex) then
            currentseqindex = nextseqindex
        else
            return nextseqindex
        end

        pickcnt = pickcnt + 1
        if pickcnt > seqlen then 
            return -1
        end
    end

    return nextseqindex 
end

local function pickskill(character)
    local is_character_has_eff_294 = character:has_effect(294)
    local eff294 = is_character_has_eff_294 and g_effecthelper.get_eff(294) or nil

    -- if (is_com(character) == true or aicontext.get_autofight() > 0) and character:get_can_cast_ultra(false) > 0 then
    --     if is_character_has_eff_294 ~= true or eff294.is_skillblocked(character, 1) ~= true then
    --         return 1,-1,-1,-1
    --     end
    -- end

    local skilllist = character:get_skill_list()
    if #skilllist < 1 then return -1,-1,-1,-1 end

    if aicontext.is_monster_can_move() then
        local special_skill_conds = { 16, 17, 13, 3, 11, 9, 12 }
        for i=1,#special_skill_conds do
            local acond = special_skill_conds[i]
            local special_skill_info = character:get_special_skill(acond)
            if special_skill_info ~= nil and special_skill_info:can_cast(character) then
                local skillid = special_skill_info:get_skillid()
                if skillid > 0 then
                    local skillindex = special_skill_info:get_skillidindex()
                    if is_character_has_eff_294 ~= true or eff294.is_skillblocked(character, skillindex) ~= true then
                        return skillindex, -1, skillid, acond
                    end
                end
            end
        end
    end

    local is_useseq2 = character:has_effect(126)
    if character:has_effect(183) then
        local rangeskillindex = g_effecthelper.get_eff(183).get_rangeskillindex(character)
        if rangeskillindex < 0 then return -1,-1,-1,-1 end

        if is_character_has_eff_294 ~= true or eff294.is_skillblocked(character, rangeskillindex) ~= true then
            local seqindex = get_attackseq_firstmatchindex(character, rangeskillindex, is_useseq2)
            if is_useseq2 == true then
                character:set_skillseqindex2(seqindex)
            else
                character:set_skillseqindex(seqindex)
            end

            return rangeskillindex, seqindex, -1, -1
        else
            return -1,-1,-1,-1
        end
    end

    local attackseq = g_effecthelper.get_eff(214).get_altattackseq(character, is_useseq2)
    if attackseq == nil then
        local characterdata = g_datahelper.get_characterdata(character:get_attr(AttrType.ID))
        attackseq = is_useseq2 and characterdata.seq2 or characterdata.seq
    end

    local currentseqindex = is_useseq2 and character:get_skillseqindex2() or character:get_skillseqindex()
    local nextseqindex = 0
    --[[
    if currentseqindex < 0 or currentseqindex >= #attackseq - 1 then
        -- do nothing
    else
        nextseqindex = currentseqindex + 1
        if nextseqindex >= #attackseq - 1 then nextseqindex = 0 end
    end

    if character:has_effect(231) then
        nextseqindex = g_effecthelper.get_eff(231).check_do(character, attackseq, nextseqindex)
    end
    ]]

    if is_character_has_eff_294 then
        local recursivestopindex = currentseqindex
        if currentseqindex < 0 then recursivestopindex = 0 end
        nextseqindex = picknormalskill_witheff294(character, attackseq, currentseqindex, eff294)
    else
        nextseqindex = picknormalskill(character, attackseq, currentseqindex)
    end
    
    if nextseqindex < 0 then
        return -1,-1,-1,-1
    end
    
    return attackseq[nextseqindex+1], nextseqindex, -1, -1
end

return pickskill