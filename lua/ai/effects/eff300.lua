-- 300 游戏音量渐变

local get_innereffdata = require "effects/getinnereffdata"

local theaiexternaldelegate = aiexternaldelegate

local AUDIO_ACTION_EFFECTS_TAG = 30001
local AUDIO_ACTION_BACKGROUND_TAG = 30002

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(self:get_target(), self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)

    local voltype = effdata.targetattribute
    local tovol = effdata.changevalue
    local duration = effdata.valuebase

    self:set_innerdata("vt", voltype)
    self:set_innerdata("tv", tovol)
    self:set_innerdata("d", duration)

    if voltype == 0 or voltype == 2 then -- effetsvolume
        local fv0 = theaiexternaldelegate.audioengine_get_effects_volume() * 100
        self:set_innerdata("fv0", fv0)

        theaiexternaldelegate.audioengine_fade_to_volume(duration/100, tovol/100, 0, AUDIO_ACTION_EFFECTS_TAG)
    end

    if voltype == 1 or voltype == 2 then -- backgroundmusicvolume
        local fv1 = theaiexternaldelegate.audioengine_get_background_volume() * 100
        self:set_innerdata("fv1", fv1)

        theaiexternaldelegate.audioengine_fade_to_volume(duration/100, tovol/100, 1, AUDIO_ACTION_BACKGROUND_TAG)
    end

    return true
end

local function on_exit(self)
    local voltype = self:get_innerdata("vt")
    local duration = self:get_innerdata("d")

    if voltype == 0 or voltype == 2 then -- effetsvolume
        local tovol = self:get_innerdata("fv0")
        theaiexternaldelegate.audioengine_fade_to_volume(duration/100, tovol/100, 0, AUDIO_ACTION_EFFECTS_TAG)
    end

    if voltype == 1 or voltype == 2 then -- backgroundmusicvolume
        local tovol = self:get_innerdata("fv1")
        theaiexternaldelegate.audioengine_fade_to_volume(duration/100, tovol/100, 1, AUDIO_ACTION_BACKGROUND_TAG)
    end
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}