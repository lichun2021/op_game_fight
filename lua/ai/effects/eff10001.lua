-- 10001 白刃战船只护盾(吸收所有伤害,优先级高-在OP1的护盾前使用)

local AttrType = require "datadefs/attrtype"
local get_innereffdata = require "effects/getinnereffdata"
local HitType = require "hurtfuncs/hittype"

local theaieffectcaster = aieffectcaster

-- local function check_do(character, param)
--     return aieffectcaster.check_exert_special_effect(character, __, param)
-- or
--     local effs = character:get_innereffects(__)
--     if not effs or #effs < 1 then return end
-- end

local function on_enter(self)
    -- if self:get_is_debuffed() then return false end

    -- self:check_clean_same_type_exec()

    self:set_is_ignorehitlabel(true)
    local target = self:get_target()
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local armorratio = self:get_skill_level()

    local changevalue = armorratio * target:get_attr(AttrType.HP) / 100
    self:set_changevalue(changevalue)

    -- MARK: 该效果为海战中船只护甲，一个技能里只配一个，可将该效果值视为初始护盾值
    target:set_tag("shiparmor_o", changevalue)

    return true
end

-- local function on_exit(self)
-- end

local function trigger(target, hurttype, hpdelta, is_simu, hittype)
    local effs = target:get_innereffects(10001)
    if not effs or #effs < 1 then return 0 end
    
    local repelnum = 0
    
    for i=1, #effs do
        local theeff = effs[i]
        if not theeff:is_temp_invalid() then
            local thechangevalue = theeff:get_changevalue()
            repelnum = repelnum + thechangevalue

            if repelnum <= hpdelta then
                    if not is_simu then
                        theeff:set_changevalue(0)
                        theeff:set_temp_invalid(true)
                        theeff:set_valid(false)
                        theeff:set_innerdata("isbroken", 1)
                        theeff:remove_ui_effect()
                        
                        -- -- fresh added
                        -- local effdata = get_innereffdata(theeff)
                        -- local params = effdata.params
                        -- local exec_max_cnt = #params
                        -- -- 参数前5位：破盾执行
                        -- if exec_max_cnt > 5 then
                        --     exec_max_cnt = 5
                        -- end
                        -- for j=1,exec_max_cnt do
                        --     local anexecid = params[j]
                        --     if anexecid > 0 then
                        --         theexechelper = theexechelper or g_exechelper
                        --         theexechelper.execute(anexecid, theeff:get_skill_id(), theeff:get_skill_level(), theeff:get_originsafe(), theeff:get_projectile(), target, 0, false, 0)
                        --     end
                        -- end
                    end
                
            else
                    if not is_simu then
                        local changevalue = repelnum - hpdelta
                        theeff:set_changevalue(changevalue)
                    end
                    repelnum = hpdelta
                    break
            end
        end

    end

    if not is_simu and repelnum > 0 then
        local remain_changeval = theaieffectcaster.get_effect_value(target, 10001, 0)
        target:set_tag("shiparmor", remain_changeval)

        -- 减护甲击中文字 3-普通 4-暴击
        local thelabel = 3
        if hittype ~= HitType.NORMAL_HIT then
            thelabel = 4
        end
        g_hitlabel.execute(thelabel, -repelnum, target)
    end
    
    return math.floor(repelnum)
end

return {
    -- check_do = check_do,
    on_enter = on_enter,
    -- on_exit = on_exit,
    trigger = trigger
}