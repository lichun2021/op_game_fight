-- 50 对所有敌人依次遍历，一次只选一个，时间间隔内只能选一次，选过后找不到目标
local CD_LOCALTICK = g_cddata.CD_LOCALTICK
local CD_TRAVERSAL_STARTTICK = g_cddata.CD_TRAVERSAL_STARTTICK

return function(params)
    local theintervaltick = params.x1
    local thetotalresettick = params.x2
    local source_tranversal_starttick = params.source:get_data(CD_TRAVERSAL_STARTTICK)
    local source_localtick = params.source:get_data(CD_LOCALTICK)
    if source_tranversal_starttick <= 0 or source_localtick > source_tranversal_starttick + thetotalresettick then
        params.source:reset_traversal_targetrecord()
        params.source:set_data(CD_TRAVERSAL_STARTTICK, source_localtick)
    end

    local source_tranversal_lastpicktick = params.source:get_val("tranversallastpicktick")
    if source_tranversal_lastpicktick > 0 and source_localtick < source_tranversal_lastpicktick + theintervaltick then
        return nil
    end

    local characterlist = g_targethelper.pick_filteredrivallist(params)
    local retlist = {}
    for i=1,#characterlist do
        local a_character = characterlist[i]
        if a_character:canbeattack(params.source, params.is_ignore_canbeattack) and not params.source:is_targetlockedbytraversal(a_character) then
            -- table.insert(retlist, a_character)
            retlist[#retlist + 1] = a_character
            params.source:pick_traversal_target(a_character, source_localtick)
        end
    end
    return retlist
end