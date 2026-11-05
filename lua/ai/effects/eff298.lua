-- 298 免疫295

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    return true
end

local function on_exit(self)
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}