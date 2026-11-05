-- 306 驱散并免疫指定效果执行的效果ID

local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()
    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)

    for i=1,#effdata.params,2 do
        local casterid = math.floor(effdata.params[i])
        local innereffectid = math.floor(effdata.params[i+1])
        local cnt = target:get_val("innereffectdebuffcount", casterid, innereffectid) + 1
        target:set_val("innereffectdebuffcount", casterid, innereffectid, cnt)

        aieffectcaster.dispel_innereffect(target, casterid, innereffectid)
    end

    return true
end

local function on_exit(self)
    local target = self:get_targetsafe()
    if not target then return end

    local effdata = get_innereffdata(self)
    
    for i=1,#effdata.params,2 do
        local casterid = math.floor(effdata.params[i])
        local innereffectid = math.floor(effdata.params[i+1])
        local cnt = target:get_val("innereffectdebuffcount", casterid, innereffectid) - 1
        if cnt < 0 then cnt = 0 end
        target:set_val("innereffectdebuffcount", casterid, innereffectid, cnt)
    end
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}