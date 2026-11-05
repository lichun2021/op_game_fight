-- isonetimehurt
local get_innereffdata = require "effects/getinnereffdata"
local theexechelper
local thedatahelper

local function is_onetimehurt(casterid, innereffect)
    if casterid == nil or casterid <= 0 then return false end

    theexechelper = theexechelper or g_exechelper
    thedatahelper = thedatahelper or g_datahelper
    if theexechelper.is_hurtexec(casterid) then
        local hurtdata = thedatahelper.get_hurtdata(casterid)
        if hurtdata ~= nil then
            local duration = hurtdata.duration
            local freq = hurtdata.freq
            if duration > 0 and freq > 0 then
                if duration / freq > 1 then
                    return false
                end
            end
        end
    elseif theexechelper.is_effectexec(casterid) then
        local effectdata = thedatahelper.get_effectdata(casterid)
        local aneff = innereffect
        if effectdata ~= nil and aneff ~= nil then
            local duration = effectdata.period
            local innerdata = get_innereffdata(aneff)
            if innerdata ~= nil and innerdata.interval > 0 then
                if duration / innerdata.interval > 1 then
                    return false
                end
            end
        end
    end

    return true
end

return is_onetimehurt