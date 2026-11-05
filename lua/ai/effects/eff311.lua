-- 311 判断目标身上是否有指定执行ID

local get_innereffdata = require "effects/getinnereffdata"
local AttrType = require "datadefs/attrtype"
-- local CasterType = require "datadefs/castertype"

local function on_enter(self)
    if self:get_is_debuffed() then return false end

    self:check_clean_same_type_exec()

    local target = self:get_target()

    self:set_is_ignorehitlabel(true)
    aieffectcaster.start_innereffect_cd(target, self)

    local effdata = get_innereffdata(self)
    local skilllevel = self:get_skill_level()
    local changevalue = effdata.changevalue + effdata.levelup * skilllevel
    self:set_changevalue(changevalue)

    --[[
        目标属性：指定效果执行的ID
        数值计算基础：判断模式  0-目标身上有此效果执行且执行完后效果消失  1-目标身上没有此效果执行且执行完后效果消失
        参数列表：触发执行ID
        数值大小：可触发阵营（指效果目标与施法者的阵营关系） 0-无限制 1-敌军 2-友军
        等级成长：可触发角色ID，填0表示没有限制（指效果目标的角色ID）
        上下限开关：指定的执行类型
            0-动画（不支持）
            1-伤害
            2-移动
            3-飞行道具
            4-效果
            5-UI特效（不支持）
            6-声音（不支持）
            7-特殊执行（不支持）
            8-标记
            9-召唤
            10-变身（不支持）
            11-身体特效（不支持）
    ]]

    local origin = self:get_origin()

    -- 可触发阵营（指效果目标与施法者的阵营关系） 0-无限制 1-敌军 2-友军
    local the_relationshiptype = effdata.changevalue
    if the_relationshiptype == 1 then
        if origin:get_teamside() == target:get_teamside() then 
            return false 
        end
    elseif the_relationshiptype == 2 then
        if origin:get_teamside() ~= target:get_teamside() then
            return false
        end
    end

    -- 可触发角色ID，填0表示没有限制（指效果目标的角色ID）
    local the_characterid = effdata.levelup
    if the_characterid > 0 then
        if the_characterid ~= target:get_attr(AttrType.ID) then
            return false
        end
    end

    local the_casterid = effdata.targetattribute
    local the_castertype = effdata.limitonoff

    local is_target_has_caster = target:has_caster(the_castertype, the_casterid)

    local can_trigger = false
    local the_comparetype = effdata.valuebase -- 判断模式  0-目标身上有此执行  1-目标身上没有此执行
    if the_comparetype == 0 then
        can_trigger = is_target_has_caster
    elseif the_comparetype == 1 then
        can_trigger = not is_target_has_caster
    end

    if can_trigger then
        for i=1,#effdata.params do 
            local anexecid = math.floor(effdata.params[i])
            g_exechelper.execute(anexecid, self:get_skill_id(), self:get_skill_level(), origin, nil, target, 0, false, 0)
        end
    end

    -- 执行完后效果消失
    return false
end

return {
    on_enter = on_enter
}