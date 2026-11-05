-- 287 每受到X次攻击，第X次会触发执行
local get_innereffdata = require "effects/getinnereffdata"

local function on_enter(self)
    if self:get_is_debuffed() == true then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)
    self:set_innerdata("x", changevalue / 100)

    return true
end

local function on_exit(self)
end

local function on_onetimehurt(character)
    local effs = character:get_innereffects(287)
    if effs == nil or #effs < 1 then return end

    local theexechelper = g_exechelper

    for i=1,#effs do
        local aneff = effs[i]
        local uid = aneff:get_uid()

        local hurtcnt = aneff:has_innerdata("c") and aneff:get_innerdata("c") or 0
        hurtcnt = hurtcnt + 1
        aneff:set_innerdata("c", hurtcnt)

        local x = aneff:get_innerdata("x")
        if hurtcnt >= x then
            local effdata = get_innereffdata(aneff)
            local params = effdata.params
            local skillid = aneff:get_skill_id()
            local skill_level = aneff:get_skill_level()
            local source = aneff:get_origin()
            local projectile = aneff:get_projectile()
            aneff:set_innerdata("c", 0)
            
            for j=1,#params do
                local anexecid = params[j]
                if anexecid > 0 then
                    theexechelper.execute(anexecid, skillid, skill_level, source, projectile, character, 0, false, 0)
                end
            end
        end
    end
end

return {
    on_enter = on_enter,
    on_exit = on_exit,
    on_onetimehurt = on_onetimehurt
}