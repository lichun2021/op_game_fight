-- 305 不能被指定阵营的技能施法者选中

local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    -- local skilllevel = self:get_skill_level()
    -- local changevalue = math.floor(effdata.changevalue)
    local relativeteamtype = math.floor(effdata.targetattribute)
    self:set_changevalue(relativeteamtype)

    -- 相对原始阵营类型 0-敌方 1-友方 2-全体
    if relativeteamtype == 0 or relativeteamtype == 2 then
        local cnt = target:get_val("cannotbefoundbyteamrelativetypecnt", 0)
        target:set_val("cannotbefoundbyteamrelativetypecnt", 0, cnt + 1)
    end

    if relativeteamtype == 1 or relativeteamtype == 2 then
        local cnt = target:get_val("cannotbefoundbyteamrelativetypecnt", 1)
        target:set_val("cannotbefoundbyteamrelativetypecnt", 1, cnt + 1)
    end

    return true
end

local function on_exit(self)
    local target = self:get_targetsafe()
    if not target then
        return
    end

    local relativeteamtype = self:get_changevalue()
    -- 相对原始阵营类型 0-敌方 1-友方 2-全体
    if relativeteamtype == 0 or relativeteamtype == 2 then
        local cnt = target:get_val("cannotbefoundbyteamrelativetypecnt", 0) - 1
        if cnt < 0 then cnt = 0 end
        target:set_val("cannotbefoundbyteamrelativetypecnt", 0, cnt)
    end

    if relativeteamtype == 1 or relativeteamtype == 2 then
        local cnt = target:get_val("cannotbefoundbyteamrelativetypecnt", 1) - 1
        if cnt < 0 then cnt = 0 end
        target:set_val("cannotbefoundbyteamrelativetypecnt", 1, cnt)
    end
end

return {
    on_enter = on_enter,
    on_exit = on_exit
}