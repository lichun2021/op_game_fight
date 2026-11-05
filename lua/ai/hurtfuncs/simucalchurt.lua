-- simucalchurt

local calcinithurt = require "hurtfuncs/calcinithurt"
local healtarget = require "hurtfuncs/healtarget"
local get_realtimehurtproperty = require "hurtfuncs/getrealtimehurtprop"
local HurtParam = require "hurtfuncs/hurtparam"

local HurtType = require "hurtfuncs/hurttype"
local HitType = require "hurtfuncs/hittype"
local HitResult = require "hurtfuncs/hitresult"

local function simucalchurt(params, a_character)
    local source = params.source
    local projectile = params.projectile
    local hurtid = params.hurt_id
    local skill_id = 0
    local skill_level = params.skill_level
    local target = a_character

    local hurtdata = g_datahelper.get_hurtdata(hurtid)
    if hurtdata == nil then return 0 end
    if source == nil and target == nil then return 0 end 

    local hpdelta = calcinithurt(source, skill_level, hurtid)
    local hurttype = hurtdata.type
    if hurttype == HurtType.HEALING_HURT_TYPE then 
        hpdelta = healtarget(source, target, hpdelta, 100002, skill_id, true, 0)
        hpdelta = -hpdelta
    else
        local hurtprop = get_realtimehurtproperty(source, hurtdata.prop)
        local param = HurtParam.new(projectile, skill_id, skill_level, hpdelta, hurttype, hurtprop, HitType.NORMAL_HIT, hurtid, false, hurtid)
        param.is_simu = true

        local hitresult, blockresult = g_hurthelper.hurttarget(source, target, param)
        if hitresult == HitResult.HitResultType.HIT_BYPASS then return 0 end

        hpdelta = param.hpdelta
    end 

    return hpdelta
end

return simucalchurt