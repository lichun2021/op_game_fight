-- skillhelper
local AttrType = require "datadefs/attrtype"
local locktarget = require "targetfuncs/locktarget"

local mathfloor = math.floor
local theaiskillcontroller = aiskillcontroller

local thedatahelper
local thetargethelper
local theexechelper

local function execute_exec(eid, skill_id, skill_level, tick, source, target)
    if eid < 1 then return end
    
    thedatahelper = thedatahelper or g_datahelper
    thetargethelper = thetargethelper or g_targethelper
    theexechelper = theexechelper or g_exechelper

    local skilldata = thedatahelper.get_skilldata(skill_id)
    -- assert(skilldata ~= nil, "skilldata is nil!")
    if not skilldata then return end

    local skilltargettype = math.floor(skilldata.conf[12])
    local realtargettype = thetargethelper.get_realsearchtype(skilltargettype).realtype
    if realtargettype == 36 or realtargettype == 37 or realtargettype == 38 or realtargettype == 39 then
        source:set_val("stationarydirection", source:get_direction())
        source:set_val("stationaryx", source:get_attr(AttrType.X))
        source:set_val("stationaryy", source:get_attr(AttrType.Y)) 
    else
        source:set_val("stationarydirection", 0)
        source:set_val("stationaryx", 0)
        source:set_val("stationaryy", 0)
    end

    --[[
    local skillrange = math.floor(skilldata.conf[14])
    local search_params = skilldata.conf[13]
    local x1,x2,y1,y2,z1,z2 = -skillrange, skillrange, -skillrange, skillrange, -skillrange, skillrange
    local targets = thetargethelper.search_with(source, source, target, nil, skilltargettype, x1, x2, y1, y2, z1, z2, search_params, true, skill_id, skill_level, 0, nil, nil, false)
    if not targets or  #targets < 1 then
        -- 解决某一角色要放技能（eg:15801，道具ID:41580)，currentTargets却找不到目标（目标被其它人弄死了），重新锁定一目标
        -- ?
        local another_target = locktarget(source)
        if another_target then
            source:set_currenttarget(another_target)
            targets = thetargethelper.search_with(source, source, target, nil, skilltargettype, x1, x2, y1, y2, z1, z2, search_params, true, skill_id, skill_level, 0, nil, nil, false)
        end
    end
    --]]
    local targets = thetargethelper.search_by_skill(skill_id, source, source, target, true)
    if not targets or  #targets < 1 then
        -- 解决某一角色要放技能（eg:15801，道具ID:41580)，currentTargets却找不到目标（目标被其它人弄死了），重新锁定一目标
        -- ?
        local another_target = locktarget(source)
        if another_target then
            source:set_currenttarget(another_target)
            targets = thetargethelper.search_by_skill(skill_id, source, source, target, true)
        end
    end

    if #targets > 0 then
        for i=1,#targets do
            local atarget = targets[i]
            theexechelper.execute(eid, skill_id, skill_level, source, nil, atarget, 0, false, 0)
        end
        if realtargettype == 26 and source:get_val("flagfirstfound26target") then
            local the26target = targets[1]
            source:set_val("firstroundconfirm26target", the26target)
        end
    end

    if source:has_tag("energyskill") then return end

    local energyid = eid
    if theexechelper.is_projectileexec(energyid) then
        local projectiledata = thedatahelper.get_projectiledata(eid)
        if projectiledata then
            energyid = math.floor(projectiledata.conf[14])
        end
    end

    if theexechelper.is_hurtexec(energyid) then
        local hurtdata = thedatahelper.get_hurtdata(energyid)
        if hurtdata then
            local endelta = math.floor(hurtdata.conf[27])

            if endelta ~= 0 then
                endelta = endelta + aieffectcaster.check_exert_special_effect(source, 197, endelta)
            end

            if source:has_effect(315) then
                endelta = endelta - g_effecthelper.get_eff(315).trigger(source, endelta)
                if endelta < 0 then endelta = 0 end
            end

            local energy = source:get_attr(AttrType.Energy)
            source:update_energy(energy + endelta)

            if endelta > 0 then
                local normal_skill_id, normal_skill_lv = source:get_normalskill()
                if skill_id == normal_skill_id and aieffectcaster.check_exert_special_effect(source, 248, 0) > 0 then
                    aieffectcaster.change_special_effect(source, 248, 1)
                end
            end
        end
    end
end -- execute_exec

local function execute_repeatending_execidintick(skillid, skilllevel, deltatick, character, target)
    thedatahelper = thedatahelper or g_datahelper
    local skilldata = thedatahelper.get_skilldata(skillid)
    if not skilldata then return end

    local source_attackspeed = character:get_attackspeed()
    local is_need_change_speed = source_attackspeed > 0 and source_attackspeed ~= 100

    local execs = skilldata.exe
    for i=1,#execs,4 do
        local execid = execs[i]
        local comparetick = execs[i + 2]
        local repeattype = execs[i + 3]
        -- 2 循环结束后执行
        if repeattype == 2 then
            if is_need_change_speed then
                comparetick = comparetick * source_attackspeed / 100
            end
            comparetick = math.floor(comparetick)
            if comparetick == deltatick then
                execute_exec(execid, skillid, skilllevel, deltatick, character, target)
            end
        end
    end
end

local function on_releasehpskill(character)
    return aicontext.on_releasehpskill(character)
end

local function get_skillid_byindex(character, skillidindex)
    local characterdata = g_datahelper.get_characterdata(character:get_attr(AttrType.ID))
    -- local seq = characterdata.seq
    -- if character:has_effect(126) then
    --     seq = characterdata.seq2
    -- end

    -- if seqindex < 0 or seqindex >= #seq then
    --     seqindex = 0
    -- end

    -- local skillidindex = seq[seqindex + 1]
    local skillid = characterdata.sk[skillidindex + 1]
    if skillid and character:has_skill(skillid) then
        return mathfloor(skillid)
    end

    return mathfloor(characterdata.sk[1])
end

local function calculate_exec_length(execid)
    return theaiskillcontroller.calculate_exec_length(execid)
end

local function calculate_skill_length(skillid)
    return theaiskillcontroller.calculate_skilllength(skillid)
end

local function calculate_accu_skill_length(skillid, nextaltaccutick)
    return theaiskillcontroller.calculate_accu_skill_length(skillid, nextaltaccutick)
end

local function calculate_repeat_skill_first_round_tick_length(skillid)
    return theaiskillcontroller.calculate_repeat_skill_first_round_tick_length(skillid)
end

local function calculate_energy_skill_first_round_tick_length(skillid)
    return theaiskillcontroller.calculate_energy_skill_first_round_tick_length(skillid)
end

local function get_repeatskillmaxrepeattick(character, skillid)
    return theaiskillcontroller.get_repeatskillmaxrepeattick(character, skillid)
end

local function execute_execidintick(skillid, skilllevel, deltatick, character, target)
    theaiskillcontroller.execute_execidintick(skillid, skilllevel, deltatick, character, target)
end

local function calculate_accumulate_skill_release_length(skillid)
    return theaiskillcontroller.calculate_accumulate_skill_release_length(skillid)
end

local function is_targetignoresilencebyskillid(character, skillid)
    return theaiskillcontroller.is_targetignoresilencebyskillid(character, skillid)
end

local function get_skilltype(character, skillid)
    -- 0-normal skill 
    -- 1-ultra or extra ultra
    if skillid <= 0 then return -1 end
    thedatahelper = thedatahelper or g_datahelper
    local characterdata = g_datahelper.get_characterdata(character:get_attr(AttrType.ID))
    local skdata = characterdata.sk
    local juexinglv = character:get_val("juexinglevel")
    if skillid == skdata[1] then
        return 0
    elseif skillid == skdata[2] or skillid == characterdata.jxj[3 * juexinglv + 1] then
        return 1
    elseif skillid == skdata[3] then
        return 2
    elseif skillid == skdata[4] then
        return 3
    elseif skillid == skdata[5] then
        return 4
    end
    return -1
end

return {
    on_releasehpskill = on_releasehpskill,
    get_skillid_byindex = get_skillid_byindex,
    calculate_exec_length = calculate_exec_length,
    calculate_skill_length = calculate_skill_length,
    calculate_accu_skill_length = calculate_accu_skill_length,
    calculate_repeat_skill_first_round_tick_length = calculate_repeat_skill_first_round_tick_length,
    calculate_energy_skill_first_round_tick_length = calculate_energy_skill_first_round_tick_length,
    get_repeatskillmaxrepeattick = get_repeatskillmaxrepeattick,
    execute_execidintick = execute_execidintick,
    calculate_accumulate_skill_release_length = calculate_accumulate_skill_release_length,
    is_targetignoresilencebyskillid = is_targetignoresilencebyskillid,
    execute_repeatending_execidintick = execute_repeatending_execidintick,
    get_skilltype = get_skilltype
}