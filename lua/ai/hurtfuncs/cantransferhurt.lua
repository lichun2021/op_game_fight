-- cantransferhurt
local AttrType = require "datadefs/attrtype"

return function(transfer, hpdelta)
    local is_fatalhit = hpdelta >= transfer:get_attr(AttrType.HP)
    if is_fatalhit ~= true then return false end 

    local trans_val = g_effecthelper.get_eff(172).check_do(transfer, 0)
    if trans_val > 0 then return true end
    return false
end