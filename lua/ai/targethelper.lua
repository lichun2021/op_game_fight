-- targethelper
local CD_MYID = g_cddata.CD_MYID
local CD_CURRENTACTION = g_cddata.CD_CURRENTACTION
local CD_DIRECTION = g_cddata.CD_DIRECTION
local CD_TEAMSIDE = g_cddata.CD_TEAMSIDE
local CD_NORMAL_SKILL_ID = g_cddata.CD_NORMAL_SKILL_ID

local AttrType = require "datadefs/attrtype"
local TeamSide = require "datadefs/teamside"

local thecharacterhelper 
local theeffecthelper
local thedatahelper

local SearchFuncs = {}
SearchFuncs[0] = function(params) return { params.source } end
SearchFuncs[1] = require "targets/target1"
SearchFuncs[2] = require "targets/target2"
SearchFuncs[3] = require "targets/target3"
SearchFuncs[4] = require "targets/target4" --
SearchFuncs[5] = require "targets/target5"
SearchFuncs[7] = require "targets/target7"
SearchFuncs[8] = require "targets/target8"
SearchFuncs[9] = require "targets/target9"
-- 10
-- 11
-- 12
-- 13
SearchFuncs[14] = require "targets/target14" --
SearchFuncs[15] = require "targets/target15"
-- 16
-- 17
-- 18
-- 19
-- 20
-- 21
-- 22
-- 23
SearchFuncs[24] = require "targets/target24"
-- 25
-- ...
SearchFuncs[31] = require "targets/target31"
SearchFuncs[32] = require "targets/target32"
-- ...
SearchFuncs[43] = require "targets/target43"
SearchFuncs[44] = require "targets/target44" --
SearchFuncs[51] = require "targets/target51" --
-- ...
SearchFuncs[63] = require "targets/target63"
-- 99
SearchFuncs[99] = require "targets/target99"

SearchFuncs[64] = require "targets/target64"
SearchFuncs[65] = require "targets/target65"
SearchFuncs[66] = require "targets/target66"

for ti=1, 67 do
    if not SearchFuncs[ti] then
        SearchFuncs[ti] = require ("targets/target"..tostring(ti))
    end
end

-- test
g_test_target_type = 0

local function get_realsearchtype(search_type)
    local info = {}
    local the_type = search_type
    info.realtype = search_type
    info.is_excludesummon = false
    info.is_ignore_canbeattack = false
    info.is_searchagain_ifexcludesummon = false
    -- +10000:无视不能成为目标的效果
    if the_type >= 10000 then 
        info.is_ignore_canbeattack = true
        the_type = the_type - 10000
        info.realtype = the_type
    end

    -- +2000:先排除召唤物，如果没有找到，再从召唤物中找
    if the_type >= 2000 then
        info.is_searchagain_ifexcludesummon = true
        the_type = the_type - 1000
        info.realtype = the_type
    end

    -- +1000:目标类型为不含召唤物的原目标类型
    if the_type >= 1000 then
        info.is_excludesummon = true
        the_type = the_type - 1000
        info.realtype = the_type
    end

    return info
end

local singletargettypes = {0,1,3,4,6,7,14,16,17,18,19,20,21,22,23,24,25,26,30,31,32,41,42,45,46,47,48,50,51,56,57,58,62,63,64,65,66,67}
local thesingletypes = {}
for i=1,#singletargettypes do
    thesingletypes[singletargettypes[i]] = 1
end

local function is_targettypeforsingle(search_type)
    -- singletargettypes = singletargettypes or {0,1,3,4,6,7,14,16,17,18,19,20,21,22,23,24,25,26,30,31,32,41,42,45,46,47,48,50,51,56,57,58,62,63,64,65,66,67}
    -- for i=1,#singletargettypes do
    --     if singletargettypes[i] == search_type then return true end
    -- end
    -- return false

    return thesingletypes[search_type] == 1
end

local function is_target297findabletosearcher(character, searcher, realtype)
    if is_targettypeforsingle(realtype) then
        theeffecthelper = theeffecthelper or g_effecthelper
        return theeffecthelper.get_eff(297).is_findable(a_character, params.source)
    end
    return true
end

local function is_skilltargetforsingle(skilldata)
    if skilldata.t then
        return skilldata.t == 1
    end
    return false
end

local function get_filteredoriginlist(characterlist, params, an_exclude_character)
    if characterlist == nil then return nil end
    thecharacterhelper = thecharacterhelper or g_characterhelper
    local source = params.source
    local is_ignore_canbeattack = params.is_ignore_canbeattack
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        local shouldadd = false
        if params.is_excludesummon and a_character:has_tag("fakesummoneff_summon") then
            -- do nothing
        elseif params.is_excludesummon and a_character:is_summon() and a_character:get_fakesummontype() == 0 and a_character:has_tag("fakesummoneff_notsummon") == false then
            -- do nothing
        elseif a_character:is_findabletoattacker(source, is_ignore_canbeattack) == false then
            -- do nothing
        elseif a_character:has_tag("misseff263withorigin") and a_character:is_findabletoattacker(params.skillcaster, is_ignore_canbeattack) ~= true then
            -- do nothing
        elseif params.skillcaster and a_character:is_findabletoskillcaster(params.skillcaster) == false then
            -- do nothing
        elseif params.hplowerlimitcharacter ~= nil and thecharacterhelper.gethpratio(a_character) <= thecharacterhelper.gethpratio(params.hplowerlimitcharacter) then
            -- do nothing
        elseif an_exclude_character ~= nil and a_character:get_data(CD_MYID) == an_exclude_character:get_data(CD_MYID) then
            -- do nothing
        elseif params.exclude_character ~= nil and a_character:get_data(CD_MYID) == params.exclude_character then
            -- do nothing
        -- elseif a_character:has_effect(297) and is_targettypeforsingle(params.realsearchtype) then
        --     theeffecthelper = theeffecthelper or g_effecthelper
        --     if theeffecthelper.get_eff(297).is_findable(a_character, source) then
        --         shouldadd = true
        --     end
        elseif a_character:has_effect(297) and params.skill_is_single_target then 
            theeffecthelper = theeffecthelper or g_effecthelper
            if theeffecthelper.get_eff(297).is_findable(a_character, source) then
                shouldadd = true
            end
        elseif source:is_induel() and source:get_duel_target():get_myid() ~= a_character:get_myid() then
            -- 12.0
            -- do nothing
        elseif a_character:is_summon() and a_character:get_fakesummontype() == 3 then
            -- 14.0
            -- do nothing
        else
            shouldadd = true
        end

        if shouldadd then
            -- table.insert(retlist, a_character)
            retlist[#retlist + 1] = a_character
        end
    end
    return retlist
end


local function get_filteredfriendlist(params)
    local source = params.source
    if source:get_data(CD_MYID) == params.skillcaster:get_data(CD_MYID) and (source:has_effect(253) or source:has_tag("summonincharm253")) then
        local retlist = get_filteredoriginlist(source:get_currentrivallist(), params, source)
        for i=1,#retlist do
            if retlist[i]:get_data(CD_MYID) == source:get_data(CD_MYID) then
                table.remove( retlist, i )
            end
        end
        retlist[#retlist + 1] = source 
        return retlist
    else
        return get_filteredoriginlist(source:get_currentfriendlist(), params)
    end
end


local function get_filteredrivallist(params)
    local source = params.source
    if source:get_data(CD_MYID) == params.skillcaster:get_data(CD_MYID) and (source:has_effect(253) == true or source:has_tag("summonincharm253")) then
        local retlist = get_filteredoriginlist(source:get_currentfriendlist(), params, source)
        for i=1,#retlist do
            if retlist[i]:get_data(CD_MYID) == source:get_data(CD_MYID) then
                table.remove( retlist, i )
            end
        end
        return retlist
    else
        return get_filteredoriginlist(source:get_currentrivallist(), params)
    end
end

local function pick_filteredfriendlist(params)
    if params.filterlist ~= nil then
        return params.filterlist
    else
        return get_filteredfriendlist(params)
    end
end

local function pick_filteredrivallist(params)
    if params.filterlist ~= nil then
        return params.filterlist
    else
        return get_filteredrivallist(params)
    end
end

local function pick_filteredalllist(params)
    if params.filterlist ~= nil then
        return params.filterlist
    else
        local retlist = get_filteredfriendlist(params)
        local rivallist = get_filteredrivallist(params)
        for i=1,#rivallist do 
            -- table.insert(retlist, rivallist[i])
            retlist[#retlist + 1] = rivallist[i]
        end 
        return retlist
    end
end

local function get_randomcharacter(characterlist, params)
    local tmplist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            -- table.insert( tmplist, a_character )
            tmplist[#tmplist + 1] = a_character
        end
    end

    if #tmplist > 0 then
        local idx = aicontext.get_randomnumber_within_range(#tmplist) + 1
        return { tmplist[idx] }
    end

    return nil
end

local function get_characterhighesthpratio(characterlist, params)
    local thehighestratio
    local thecharacter = nil
    for i=1,#characterlist do 
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local a_hpratio = g_characterhelper.gethpratio(a_character)
            if thehighestratio == nil or a_hpratio > thehighestratio then 
                thehighestratio = a_hpratio
                thecharacter = a_character
            end
        end
    end
    if thecharacter ~= nil then return { thecharacter } end
    return nil
end


local function get_lowesthpratiocharacter(characterlist, params)
    local theminhpratio
    local themincharacter = nil
    for i=1,#characterlist do
        local a_character = characterlist[i]
        local a_hpratio = g_characterhelper.gethpratio(a_character)
        if (theminhpratio == nil or a_hpratio < theminhpratio) and a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            theminhpratio = a_hpratio
            themincharacter = a_character
        end
    end
    if themincharacter ~= nil then return { themincharacter } else return nil end
end

local function get_targetsinrange(characterlist, params, origin)
    if origin == nil and params.projectile == nil then return nil end
    local x11,x22,y11,y22
    local ox,oy

    if params.projectile ~= nil then
        ox = params.projectile:get_currentx()
        oy = params.projectile:get_currenty()
    else
        ox = origin:get_attr(AttrType.X)
        oy = origin:get_attr(AttrType.Y)
    end

    if (origin:get_data(CD_DIRECTION) == 1) then
        x11 = ox + params.x1
        x22 = ox + params.x2
    else
        x11 = ox - params.x2
        x22 = ox - params.x1
    end

    y11 = oy + params.y1
    y22 = oy + params.y2

    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local x = a_character:get_attr(AttrType.X)
            local y = a_character:get_attr(AttrType.Y)
            if x >= x11 and x <= x22 and y >= y11 and y <= y22 then
                -- table.insert( retlist, a_character )
                retlist[#retlist + 1] = a_character
            end
        end
    end
    return retlist
end

local function get_characterinabsoluterange(characterlist, params, origin)
    if origin == nil and params.projectile == nil then return nil end
    local ostationary_direction = origin:get_stationary_direction()
    local ostationary_x = origin:get_stationary_x()
    local ostationary_y = origin:get_stationary_y()
    if ostationary_direction == 0 and ostationary_x == 0 and ostationary_y == 0 then return nil end

    local x11,x22,y11,y22
    local ox,oy

    if params.projectile ~= nil then
        ox = params.projectile:get_currentx()
        oy = params.projectile.get_currenty()
    else
        ox = ostationary_x
        oy = ostationary_y
    end

    if (ostationary_direction == 1) then
        x11 = ox + params.x1
        x22 = ox + params.x2
    else
        x11 = ox - params.x2
        x22 = ox - params.x1
    end

    y11 = oy + params.y1
    y22 = oy + params.y2

    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if (params.exclude_myid == nil or params.exclude_myid ~= a_character:get_data(CD_MYID)) and a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local x = a_character:get_attr(AttrType.X)
            local y = a_character:get_attr(AttrType.Y)
            if x >= x11 and x <= x22 and y >= y11 and y <= y22 then
                -- table.insert( retlist, a_character )
                retlist[#retlist +1] = a_character
            end
        end
    end
    return retlist
end

local function get_character_in_battlefield_absolute_range(characterlist, params)
    local theinitdirection = 1
    if params.source:get_data(CD_TEAMSIDE) == TeamSide.kEnemy then
        theinitdirection = -1
    end

    local thebattlefieldcenterx = aicontext.get_battlefield_center_x()
    local thebattlefieldcentery = (aicontext.get_delta_max_y() - aicontext.get_delta_min_y()) / 2.0

    local x11 = thebattlefieldcenterx + params.x1 * theinitdirection
    local x22 = thebattlefieldcenterx + params.x2 * theinitdirection
    local y11 = thebattlefieldcentery + params.y1
    local y22 = thebattlefieldcentery + params.y2

    if x11 > x22 then
        local tmpx = x11
        x11 = x22
        x22 = tmpx
    end

    if y11 > y22 then
        local tmpy = y11
        y11 = y22
        y22 = tmpy
    end

    local retlist = {}
    for i=1,#characterlist do 
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local x = a_character:get_attr(AttrType.X)
            local y = a_character:get_attr(AttrType.Y)
            if x >= x11 and x <= x22 and y >= y11 and y <= y22 then 
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end
        end
    end
    return retlist
end

local function get_distance(chara1, chara2)
    local x = chara1:get_attr(AttrType.X) - chara2:get_attr(AttrType.X)
    local y = chara1:get_attr(AttrType.Y) - chara2:get_attr(AttrType.Y)
    return math.sqrt( x * x + y * y )
end

local function get_farthestcharacter(characterlist, params)
    local thefarthestdist = -1
    local thefarthestcharacter = nil
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local adist = get_distance(a_character, params.source)
            if adist > thefarthestdist then
                thefarthestdist = adist
                thefarthestcharacter = a_character
            end
        end
    end
    if thefarthestcharacter ~= nil then return { thefarthestcharacter } else return nil end
end

local function get_nearestcharacter(characterlist, params)
    local thenearestdist = -1
    local thenearestcharacter = nil
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local adist = get_distance(a_character, params.source)
            if thenearestdist < 0 or adist < thenearestdist then
                thenearestdist = adist
                thenearestcharacter = a_character
            end
        end
    end
    if thenearestcharacter ~= nil then return { thenearestcharacter } else return nil end
end

local function get_allsummon(characterlist, params)
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:is_summon() and a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            -- table.insert(retlist, a_character)
            retlist[#retlist + 1] = a_character
        end
    end
    return retlist
end

local function get_character_withid(characterlist, params)
    local sideparams = params.search_params
    local friendside = {}
    local rivalside = {}
    for i=1,#sideparams do 
        local side = sideparams[i]
        local characterid = math.floor(sideparams[i+1]) -- TODO: safe check
        if side == 0 or side == 1 then
            friendside[characterid] = true
        elseif side == 0 or side == 2 then
            rivalside[characterid] = true
        end
    end 

    local retlist = {}
    local sourceteamside = params.source:get_teamside()
    for i=1,#characterlist do 
        local a_character = characterlist[i]
        local an_id = a_character:get_attr(AttrType.ID)
        if a_character:get_teamside() == sourceteamside then 
            if friendside[an_id] ~= nil then 
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end 
        else
            if rivalside[an_id] ~= nil then 
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end 
        end
    end 
    return retlist
end

local function get_largestattrvalue(character, attrs)
    local thelargestval = -1
    for i=1,#attrs do 
        local a_val = character:get_attr(attrs[i])
        if thelargestval < a_val then
            thelargestval = a_val
        end
    end 
    return thelargestval
end

local function get_lowestattrvalue(character, attrs)
    local thelowestval = 10000000
    for i=1,#attrs do 
        local a_val = character:get_attr(attrs[i])
        if thelowestval > a_val then
            thelowestval = a_val
        end
    end 
    return thelowestval
end

local function get_characterinhprange(characterlist, params, low, high)
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local a_hpratio = g_characterhelper.gethpratio(a_character)
            if a_hpratio >= low and a_hpratio <= high then 
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end
        end
    end
    return retlist
end

local function is_a_label_in_labels(a_label, labels)
    for i=1,#labels do
        if a_label == labels[i] then return true end
    end
    return false
end

local function is_label_in_characterdata(labels, characterdata_label)
    if not labels then return false end
    for i=1,#labels do 
        if is_a_label_in_labels(labels[i], characterdata_label) then return true end
    end 
    return false
end

local function get_characterhaslabels(characterlist, params, labels)
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            local characterdata = g_datahelper.get_characterdata(a_character:get_attr(AttrType.ID))
            if is_label_in_characterdata(labels, characterdata.label) then
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end 
        end
    end
    return retlist
end

local function get_randomdeadcharacter(characterlist)
    local tmplist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if g_actionhelper.is_deadaction(a_character:get_data(CD_CURRENTACTION)) and a_character:is_summon() == false then
            -- table.insert( tmplist, a_character )
            tmplist[#tmplist + 1] = a_character
        end
    end

    if #tmplist > 0 then
        local idx = aicontext.get_randomnumber_within_range(#tmplist) + 1
        return { tmplist[idx] }
    end

    return nil
end

local function get_deadcharacter(characterlist)
    local retlist
    for i=1,#characterlist do 
        local a_character = characterlist[i]
        if g_actionhelper.is_deadaction(a_character:get_data(CD_CURRENTACTION)) and a_character:is_summon() == false then
            -- table.insert( retlist, a_character )
            retlist[#retlist + 1] = a_character
        end
    end 
    return retlist
end

-- params:source, target, projectile, aicontext, search_type, x1, x2, y1, y2, z1, z2, search_params, skillcaster, is_usingskill, skill_id, hurt_id, skill_level, filter_list, exclude_character, is_hpratiohtexcludecharacter
local function search(params)
    local realsearchinfo = get_realsearchtype(params.search_type)
    params.realsearchinfo = realsearchinfo
    params.realsearchtype = realsearchinfo.realtype
    params.is_excludesummon = realsearchinfo.is_excludesummon
    params.is_ignore_canbeattack = realsearchinfo.is_ignore_canbeattack

    if params.is_hpratiohtexcludecharacter == true and params.exclude_character ~= nil then
        params.hplowerlimitcharacter = params.exclude_character
    end

    if params.skill_id and params.skill_id > 0 then
        thedatahelper = thedatahelper or g_datahelper
        local skilldata = thedatahelper.get_skilldata(params.skill_id)
        if skilldata and skilldata.t and skilldata.t == 1 then
            params.skill_is_single_target = true
        end
    end

    local realsearchtype = params.realsearchtype

    local searchfunc = SearchFuncs[realsearchtype]
    if searchfunc ~= nil then
        if realsearchtype == 0 or realsearchtype == 15 then
            return searchfunc(params)
        end

        theeffecthelper = theeffecthelper or g_effecthelper
        -- begin same
        local source = params.source
        if theeffecthelper.get_eff(248).check_do(source, 0) > 0 and params.skill_id == source:get_data(CD_NORMAL_SKILL_ID) then 
            return { source } 
        end
        if theeffecthelper.get_eff(248).check_do(params.skillcaster, 0) > 0 and params.skill_id == params.skillcaster:get_data(CD_NORMAL_SKILL_ID) then
            return { params.skillcaster }
        end
        -- if source:has_effect(280) then
        --     local target_280 = source:get_target_for_focus()
        --     if target_280 then
        --         return { target_280 }
        --     end
        -- end

        if realsearchtype == 1 then
            return searchfunc(params)
        end

        if source:is_induel() then params.filterlist = nil end
        -- end same

        local rettargets = searchfunc(params)
        if realsearchinfo.is_searchagain_ifexcludesummon and params.is_excludesummon and (rettargets == nil or #rettargets < 1) then
            params.is_excludesummon = false
            return searchfunc(params)
        else
            return rettargets
        end
    else
        assert(false, "targettype:" .. tostring(realsearchtype).. "(" ..tostring(params.search_type)..") is not supported")
    end
end

-- params:skillcaster, source, target, projectile, search_type, x1, x2, y1, y2, z1, z2, search_params, is_usingskill, skill_id, skill_level, hurt_id, filter_list, exclude_character, is_hpratiohtexcludecharacter
local function search_with(skillcaster, source, target, projectile, search_type, x1, x2, y1, y2, z1, z2, search_params, is_usingskill, skill_id, skill_level, hurt_id, filter_list, exclude_character, is_hpratiohtexcludecharacter)
    return search({skillcaster = skillcaster, source = source, target = target, projectile = projectile, search_type = search_type, x1 = x1, x2 = x2, y1 = y1, y2 = y2, z1 = z1, z2 = z2, search_params = search_params, is_usingskill = is_usingskill, skill_id = skill_id, skill_level = skill_level, hurt_id = hurt_id, filter_list = filter_list, exclude_character = exclude_character, is_hpratiohtexcludecharacter = is_hpratiohtexcludecharacter})
end

local function search_with_ctx(ctx)
    local params = {
        skillcaster = ctx[1],
        source = ctx[2],
        target = ctx[3],
        projectile = ctx[4],
        search_type = ctx[5],
        x1 = ctx[6], 
        x2 = ctx[7], 
        y1 = ctx[8], 
        y2 = ctx[9], 
        z1 = ctx[8], 
        z2 = ctx[9], 
        search_params = ctx[10], 
        is_usingskill = ctx[11], 
        skill_id = ctx[12], 
        skill_level = ctx[13], 
        hurt_id = ctx[14], 
        filter_list = ctx[15], 
        exclude_character = ctx[16], 
        is_hpratiohtexcludecharacter = ctx[17]
    }
    return search(params)
end

local function search_by_skill(skill_id, skill_caster, source, target, is_usingskill)
    return aiactioncontroller.search_target_by_skill(skill_id, skill_caster, source, target, is_usingskill)
end

return {
    get_realsearchtype = get_realsearchtype,
    search = search,
    search_with = search_with,
    search_with_ctx = search_with_ctx,
    search_by_skill = search_by_skill,
    get_filteredfriendlist = get_filteredfriendlist,
    get_filteredrivallist= get_filteredrivallist,
    pick_filteredfriendlist = pick_filteredfriendlist,
    pick_filteredrivallist = pick_filteredrivallist,
    pick_filteredalllist = pick_filteredalllist,
    get_randomcharacter = get_randomcharacter,
    get_characterhighesthpratio = get_characterhighesthpratio,
    get_lowesthpratiocharacter = get_lowesthpratiocharacter,
    get_targetsinrange = get_targetsinrange,
    get_characterinabsoluterange = get_characterinabsoluterange,
    get_character_in_battlefield_absolute_range = get_character_in_battlefield_absolute_range,
    get_farthestcharacter = get_farthestcharacter,
    get_nearestcharacter = get_nearestcharacter,
    get_allsummon = get_allsummon,
    get_character_withid = get_character_withid,
    get_largestattrvalue = get_largestattrvalue,
    get_lowestattrvalue = get_lowestattrvalue,
    get_characterinhprange = get_characterinhprange,
    get_characterhaslabels = get_characterhaslabels,
    get_randomdeadcharacter = get_randomdeadcharacter,
    get_deadcharacter = get_deadcharacter,

    is_targettypeforsingle = is_targettypeforsingle,
    is_target297findabletosearcher = is_target297findabletosearcher,
    is_skilltargetforsingle = is_skilltargetforsingle
}