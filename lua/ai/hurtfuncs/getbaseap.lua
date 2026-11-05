-- getbaseap

local AttrType = require "datadefs/attrtype"
local HurtType = require "hurtfuncs/hurttype"

return function (hurt_type, source)
    local source_ba_power = source:get_attr(AttrType.BAPower)
    local source_ma_power = source:get_attr(AttrType.MAPower)
    if hurt_type == HurtType.HEALING_HURT_TYPE and source_ba_power > source_ma_power then
        hurt_type = HurtType.PHYSICAL_HURT_TYPE 
    end
    if hurt_type == HurtType.PHYSICAL_HURT_TYPE then
        return source_ba_power
    else
        return source_ma_power
    end
end