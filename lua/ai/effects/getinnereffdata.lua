local thedatahelper

local function get_innereffdata(the_eff)
    thedatahelper = thedatahelper or g_datahelper
    local effcasterid, innereffid, effidx = the_eff:get_innerexec()
    local innereffdata = thedatahelper.get_innereffectdata(effcasterid, innereffid, effidx + 1)
    assert(innereffid == innereffdata.ID, "innereff id different!")
    return innereffdata
end

return get_innereffdata