-- configdatahelper
local cjson = require "cjson"
local util = require "cjson/util"

local math_floor = math.floor

local mt_hurtdata = require "datadefs/hurtdata"
local mt_effectexecdata = require "datadefs/effectexecdata"

local configs = {
    "_system_config",
    "_fighting_parameter",
    "_convert_fp_list"
    -- "_character_list",
    -- "_skill_list",
    -- "_skill_hurt",
    -- "_domineer_site_list_hash", -- 霸气试炼
    --"_skill_effect",
    -- "_skill_summon_attr",
    -- "_skill_projectile"
}

local has_configdata_hotfix = (aicontext.get_configdata_json ~= nil)

if not has_configdata_hotfix then
    configs = {
        "_system_config",
        "_fighting_parameter",
        "_convert_fp_list",
        "_character_list",
        "_skill_list",
        "_skill_hurt",
        "_domineer_site_list_hash", -- 霸气试炼
        "_skill_effect",
        "_skill_summon",
        "_skill_summon_attr",
        "_skill_projectile"
    }
end

local config_cache = {
    character_list = {},
    skill_list = {},
    skill_list_not_found = {},
    skill_effect = {},
    skill_effect_not_found = {},
    skill_hurt = {},
    skill_hurt_not_found = {},
    skill_projectile = {},
    skill_projectile_not_found = {},
    domineer_site_list_hash = {},
    domineer_site_list_hash_not_found = {},
    skill_summon = {},
    skill_summon_not_found = {},
    skill_summon_attr = {},
    skill_summon_attr_not_found = {}
}

local function init_a_skill(a_skill)
    local ignoreeffs = {}
    local the_effs = a_skill.conf[17]
    for i=1, #the_effs do
        local a_effid = the_effs[i]
        ignoreeffs[tostring(a_effid)] = true
    end
    a_skill.ignoreeffs = ignoreeffs
end

local function init_skill_list(skill_list)
    for _,v in pairs(skill_list) do 
        local ignoreeffs = {}
        local the_effs = v.conf[17]
        for i=1, #the_effs do
            local a_effid = the_effs[i]
            ignoreeffs[tostring(a_effid)] = true
        end
        v.ignoreeffs = ignoreeffs
    end
end

local function init_a_skill_hurt(a_skill_hurt)
    local v = a_skill_hurt
    local hitexecs = {0,0,0}
    -- table.insert(hitexecs, v.conf[16])
    -- table.insert(hitexecs, v.conf[17])
    -- table.insert(hitexecs, v.conf[18])
    hitexecs[1] = v.conf[16]
    hitexecs[2] = v.conf[17]
    hitexecs[3] = v.conf[18]
    local extraexecs = v.conf[42]
    if type(extraexecs) == "table" then
        for i=1,#extraexecs do
            -- table.insert(hitexecs, extraexecs[i])
            hitexecs[#hitexecs + 1] = extraexecs[i]
        end
    end
    v.hitexecs = hitexecs

    setmetatable(v, mt_hurtdata)
end

local function init_skill_hurt(skill_hurt)
    for _,v in pairs(skill_hurt) do
        -- v.id = k
        local hitexecs = {0,0,0}
        -- table.insert(hitexecs, v.conf[16])
        -- table.insert(hitexecs, v.conf[17])
        -- table.insert(hitexecs, v.conf[18])
        hitexecs[1] = v.conf[16]
        hitexecs[2] = v.conf[17]
        hitexecs[3] = v.conf[18]
        local extraexecs = v.conf[42]
        if type(extraexecs) == "table" then
            for i=1,#extraexecs do
                -- table.insert(hitexecs, extraexecs[i])
                hitexecs[#hitexecs + 1] = extraexecs[i]
            end
        end
        v.hitexecs = hitexecs

        setmetatable(v, mt_hurtdata)
    end
end

local function init_a_skill_effect(a_skill_effect)
    local innereffs = {0,0,0}
    local execs = a_skill_effect.exec
    local lenofanexec = 19
    local v = a_skill_effect
    for i=1,3 do
        local an_innereff = {}
        local offset = (i-1) * lenofanexec + 1
        an_innereff.ID = execs[offset]
        an_innereff.type = execs[offset + 1]
        an_innereff.label = execs[offset + 2]
        an_innereff.changetype = execs[offset + 4]
        an_innereff.targetattribute = execs[offset + 5]
        an_innereff.valuebase = execs[offset + 6]
        an_innereff.changevalue = execs[offset + 7]
        an_innereff.levelup = execs[offset + 8]
        an_innereff.limitonoff = execs[offset + 9]
        an_innereff.limitvallowerbase = execs[offset + 10]
        an_innereff.limitvallowergrowth = execs[offset + 11]
        an_innereff.limitvalupperbase = execs[offset + 12]
        an_innereff.limitvaluppergrowth = execs[offset + 13]

        an_innereff.exitclear = execs[offset + 15]
        an_innereff.interval = execs[offset + 16]

        an_innereff.hurttype = execs[offset + 18]
        an_innereff.params = v.param[i]
        an_innereff.hitdeadexecids = v.tok[i]
        innereffs[i] = an_innereff
    end
    a_skill_effect.innereffs = innereffs

    setmetatable(a_skill_effect, mt_effectexecdata)
end

local function init_skill_effect(skill_effect)
    local lenofanexec = 19
    for _,v in pairs(skill_effect) do
        local innereffs = {0,0,0}
        local execs = v.exec
        for i=1,3 do
            local an_innereff = {}
            local offset = (i-1) * lenofanexec + 1
            an_innereff.ID = execs[offset]
            an_innereff.type = execs[offset + 1]
            an_innereff.label = execs[offset + 2]
            an_innereff.changetype = execs[offset + 4]
            an_innereff.targetattribute = execs[offset + 5]
            an_innereff.valuebase = execs[offset + 6]
            an_innereff.changevalue = execs[offset + 7]
            an_innereff.levelup = execs[offset + 8]
            an_innereff.limitonoff = execs[offset + 9]
            an_innereff.limitvallowerbase = execs[offset + 10]
            an_innereff.limitvallowergrowth = execs[offset + 11]
            an_innereff.limitvalupperbase = execs[offset + 12]
            an_innereff.limitvaluppergrowth = execs[offset + 13]

            an_innereff.exitclear = execs[offset + 15]
            an_innereff.interval = execs[offset + 16]

            an_innereff.hurttype = execs[offset + 18]
            an_innereff.params = v.param[i]
            an_innereff.hitdeadexecids = v.tok[i]
            innereffs[i] = an_innereff
        end
        v.innereffs = innereffs

        setmetatable(v, mt_effectexecdata)
    end
end

local function init_skill_summon(a_skill_summon)
    local lenofanexec = 14
    local v = a_skill_summon
    -- for _,v in pairs(skill_summon) do
        local innersummons = {0,0,0}
        local execs = v.exec
        for i=1,3 do
            local a_summon = {}
            local offset = (i-1) * lenofanexec + 1
            a_summon.type = execs[offset]
            a_summon.rate = execs[offset + 1]
            a_summon.interval = execs[offset + 2]
            a_summon.summonnum = execs[offset + 3]
            a_summon.maxsummonnum = execs[offset + 4]
            a_summon.onesample = execs[offset + 5]
            a_summon.summonid = execs[offset + 6]
            a_summon.showsummoninterval = execs[offset + 7]

            innersummons[i] = a_summon
        end
        v.innersummons = innersummons
        if #v.conf >= 9 then
            v.group = {v.conf[7], v.conf[8], v.conf[9]} -- group:[id, weight, maxnum]
        end
    -- end
end

local theconfigdata

local function preload_allconfigdata(configpath)
    if theconfigdata then 
        print("data already loaded")
        return theconfigdata 
    end

    print("preloadconfig start")

    local retdata = {}

    local oploadfile = util.file_load
    if g_platform == "IOS" or g_platform == "ANDROID" then
        oploadfile = aicontext.get_string_from_file
    end

    for i=1, #configs do
        -- print("preload:"..i..":"..configs[i])

        --retdata[configs[i]] = cjson.decode(util.file_load(configdatapath .. configs[i] .. ".txt"))
        local filename = configpath .. configs[i] .. ".txt"
        local filestr = oploadfile(filename)
        retdata[configs[i]] = cjson.decode(filestr)
        -- print(j_var)
        -- retdata[configs[i]] = j_var

        -- print("finish")
    end

    if not has_configdata_hotfix then
        init_skill_list(retdata._skill_list)
        init_skill_hurt(retdata._skill_hurt)
        init_skill_effect(retdata._skill_effect)
    end

    for _,v in pairs(retdata._fighting_parameter) do
        for i=1, #v.p do 
            v.p[i] = v.p[i] / 100
        end 
    end

    print("preloadconfig finish")

    theconfigdata = theconfigdata or retdata

    return retdata
end

local function set_loaded_data(data)
    theconfigdata = theconfigdata or data
end

local function get_systemconfig()
    return theconfigdata._system_config
end

local function get_characterdata(character_id)
    if not has_configdata_hotfix then
        return theconfigdata._character_list[tostring(character_id)]
    end

    local the_id = math_floor(character_id)
    local the_data = config_cache.character_list[the_id]
    if the_data then
        return the_data
    else
        the_data = cjson.decode(aicontext.get_configdata_json(1, the_id))
        config_cache.character_list[the_id] = the_data
        return the_data
    end
end

local function get_skilldata(skill_id)
    if not has_configdata_hotfix then
        return theconfigdata._skill_list[tostring(math_floor(skill_id))]
    end

    local the_id = math_floor(skill_id)
    local the_data = config_cache.skill_list[the_id]
    if the_data then
        return the_data
    elseif config_cache.skill_list_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(2, the_id)
        if not datajson or datajson == "" then
            config_cache.skill_list_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.skill_list[the_id] = the_data
        init_a_skill(the_data)
        return the_data
    end
    return nil
end

local function get_hurtdata(hurt_id)
    if not has_configdata_hotfix then
        return theconfigdata._skill_hurt[tostring(hurt_id)]
    end

    local the_id = math_floor(hurt_id)
    local the_data = config_cache.skill_hurt[the_id]
    if the_data then
        return the_data
    elseif config_cache.skill_hurt_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(4, the_id)
        if not datajson or datajson == "" then
            config_cache.skill_hurt_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.skill_hurt[the_id] = the_data
        init_a_skill_hurt(the_data)
        return the_data
    end
    return nil

end

local function get_summonexecdata(summonexecid)
    if not has_configdata_hotfix then
        return theconfigdata._skill_summon[tostring(summonexecid)]
    end

    local the_id = math_floor(summonexecid)
    local the_data = config_cache.skill_summon[the_id]
    if the_data then
        return the_data
    elseif config_cache.skill_summon_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(8, the_id)
        if not datajson or datajson == "" then
            config_cache.skill_summon_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.skill_summon[the_id] = the_data
        init_skill_summon(the_data)
        return the_data
    end
    return nil
end

local function get_summonpropdata(summonprop_id)
    if not has_configdata_hotfix then
        return theconfigdata._skill_summon_attr[tostring(summonprop_id)]
    end

    local the_id = math_floor(summonprop_id)
    local the_data = config_cache.skill_summon_attr[the_id]
    if the_data then
        return the_data
    elseif config_cache.skill_summon_attr_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(7, the_id)
        if not datajson or datajson == "" then
            config_cache.skill_summon_attr_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.skill_summon_attr[the_id] = the_data
        return the_data
    end
    return nil
end

local function get_convertfplist()
    return theconfigdata._convert_fp_list
end

local function get_effectdata(effectid)
    if not has_configdata_hotfix then
        return theconfigdata._skill_effect[tostring(effectid)]
    end

    local the_id = math_floor(effectid)
    local the_data = config_cache.skill_effect[the_id]
    if the_data then
        return the_data
    elseif config_cache.skill_effect_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(3, the_id)
        if not datajson or datajson == "" then
            config_cache.skill_effect_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.skill_effect[the_id] = the_data
        init_a_skill_effect(the_data)
        return the_data
    end
    return nil
end

local function get_innereffectdata(effectid, innerid, inneridx)
    return get_effectdata(effectid).innereffs[inneridx]
end

local function get_projectiledata(projectile_id)
    if not has_configdata_hotfix then
        return theconfigdata._skill_projectile[tostring(projectile_id)]
    end

    local the_id = math_floor(projectile_id)
    local the_data = config_cache.skill_projectile[the_id]
    if the_data then
        return the_data
    elseif config_cache.skill_projectile_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(5, the_id)
        if not datajson or datajson == "" then
            config_cache.skill_projectile_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.skill_projectile[the_id] = the_data
        return the_data
    end
    return nil
end

local function get_fightingparameter(idx)
    return theconfigdata._fighting_parameter[idx].p
end

local function get_baqispecialexec(talentid, talentlv)
    if not has_configdata_hotfix then
        return theconfigdata._domineer_site_list_hash[tostring(talentid)][talentlv + 1]
    end

    local the_id = math_floor(talentid)
    local the_data = config_cache.domineer_site_list_hash[the_id]
    if the_data then
        return the_data[talentlv + 1]
    elseif config_cache.domineer_site_list_hash_not_found[the_id] then
        return nil
    else
        local datajson = aicontext.get_configdata_json(6, the_id)
        if not datajson or datajson == "" then
            config_cache.domineer_site_list_hash_not_found[the_id] = true
            return nil
        end
        the_data = cjson.decode(datajson)
        config_cache.domineer_site_list_hash[the_id] = the_data
        return the_data[talentlv + 1]
    end
    return nil
end

local function is_skill_ignoreeffect(skillid, effid)
    -- local skilldata = theconfigdata._skill_list[tostring(skillid)]
    local skilldata = get_skilldata(skillid)
    return skilldata.ignoreeffs[tostring(effid)] ~= nil
end

return {
    preload_allconfigdata = preload_allconfigdata,
    get_systemconfig = get_systemconfig,
    get_characterdata = get_characterdata,
    get_skilldata = get_skilldata,
    get_hurtdata = get_hurtdata,
    get_fightingparameter = get_fightingparameter,
    get_baqispecialexec = get_baqispecialexec,
    get_effectdata = get_effectdata,
    is_skill_ignoreeffect = is_skill_ignoreeffect,
    get_summonexecdata = get_summonexecdata,
    get_summonpropdata = get_summonpropdata,
    get_convertfplist = get_convertfplist,
    get_projectiledata = get_projectiledata,
    get_innereffectdata = get_innereffectdata
}
