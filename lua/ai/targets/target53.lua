-- 53 范围内的施法者召唤单位
local AttrType = require "datadefs/attrtype"
local CD_DIRECTION = g_cddata.CD_DIRECTION

return function(params)
    local source = params.skillcaster
    local source_x = source:get_attr(AttrType.X)
    local source_y = source:get_attr(AttrType.Y)
    local x11,x22
    if source:get_data(CD_DIRECTION) == 1 then
        x11 = source_x + params.x1
        x22 = source_x + params.x2
    else
        x11 = source_x - params.x2
        x22 = source_x - params.x1
    end
    local y11 = source_y + params.y1
    local y22 = source_y + params.y2
    local characterlist = source:get_summons()
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(source, params.is_ignore_canbeattack) then
            local x = a_character:get_attr(AttrType.X)
            local y = a_character:get_attr(AttrType.Y)
            if x >= x11 and x <= x22 and y >= y11 and y <= y22 then
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end
        end
    end
    return retlist
end