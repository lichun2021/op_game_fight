-- 52 拥有指定标记的目标
local function get_characterhasmark(characterlist, params, markid)
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:has_mark(markid) and a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
            -- table.insert(retlist, a_character)
            retlist[#retlist + 1] = a_character
        end
    end
    return retlist
end

return function(params)
    if params.search_params == nil or #(params.search_params) < 3 then return nil end
    local themarkid = params.search_params[1]
    local theside = params.search_params[2]
    local theusingcurrent_whennotarget = params.search_params[3]
    local characterlist = nil
    if theside == 0 then
        characterlist = g_targethelper.pick_filteredalllist(params)
    elseif theside == 1 then
        characterlist = g_targethelper.pick_filteredfriendlist(params)
    elseif theside == 2 then
        characterlist = g_targethelper.pick_filteredrivallist(params)
    end

    characterlist = get_characterhasmark(characterlist, params, themarkid)
    if theusingcurrent_whennotarget == 1 then
        if characterlist == nil or #characterlist < 1 then
            local source_currenttarget = params.source:get_currenttarget()
            if source_currenttarget ~= nil and source_currenttarget:canbeattack(params.source, params.is_ignore_canbeattack) then
                return { source_currenttarget }
            end
        end
    end

    return characterlist
end