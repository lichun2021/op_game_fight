--
-- local innereffectid = {282,283,284,285}
-- local effconf = {}
-- for i=1,#innereffectid do
--     local effid = innereffectid[i]
--     effconf[effid] = true
-- end

local function is_innereffectbylua(effid)
    if effid >= 282 and effid <= 500 then return true end
    if effid == 273 then 
        return true 
    end
    if effid == 10001 then
        return true
    end
    return false
end

return {
    is_innereffectbylua = is_innereffectbylua
}