-- getbasechp

local AttrType = require "datadefs/attrtype"
local HurtType = require "hurtfuncs/hurttype"
return function(hurttype, source)
    if hurttype == HurtType.HEALING_HURT_TYPE and source:get_attr(AttrType.BAPower) > source:get_attr(AttrType.MAPower) then
        hurttype = HurtType.PHYSICAL_HURT_TYPE
    end

    return source:get_attr(hurttype == HurtType.PHYSICAL_HURT_TYPE and AttrType.BACritM or AttrType.MACritM) / 100
end 