local HurtPropertyType = require "hurtfuncs/hurtpropertytype"

return function(source, originprop)
    return source:has_effect(156) and HurtPropertyType.kNoProperty or originprop
end