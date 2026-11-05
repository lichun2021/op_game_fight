-- getoriginavoidancehurt

local AttrType = require "datadefs/attrtype"
local HurtType = require "hurtfuncs/hurttype"

local function get_basedp(hurttype, source, target)
    if hurttype == HurtType.PHYSICAL_HURT_TYPE then
        return target:get_attr(AttrType.BARepel) - source:get_attr(AttrType.BABreak)
    else
        return target:get_attr(AttrType.MARepel) - source:get_attr(AttrType.MABreak)
    end
end

local function getoriginavoidancehurt(hurttype, target, source, basehurtvalue)
    if hurttype < 0 then return basehurtvalue end 

    local param = g_datahelper.get_fightingparameter(hurttype == HurtType.PHYSICAL_HURT_TYPE and 1 or 2)
    local basedp = get_basedp(hurttype, source, target)
    basedp = math.floor( basedp )
    if basedp < 0 then basedp = 0 end 

    -- 免伤率=受伤者实际物理防御/(受伤者实际物理防御+攻击者等级*A+B)
    local avd = basedp / ( basedp + source:get_attr(AttrType.Lv) * param[1] + param[2] )
    if avd < param[7] then avd = param[7] end 
    if avd > param[8] then avd = param[8] end 
    if hurttype == HurtType.HEALING_HURT_TYPE then avd = 0 end 
    if avd > 1.0 then avd = 1.0 end 

    return math.floor( basehurtvalue * (1 - avd) )
end 

return getoriginavoidancehurt