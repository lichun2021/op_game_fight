-- 13 所有人
--[[
            if (filterList) {
                addCharactInRange(source, filterList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            }
            else {
                addCharactInRange(source, friendList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
                addCharactInRange(source, rivalList, targetList, &source, projectile, param.x1, param.x2, param.y1, param.y2, param.z1, param.z2, ignoreCanbeattackEffect);
            }
]]
return function( params )
    local characterlist = params.filterlist
    if characterlist == nil then
        characterlist = {}
        local friendlist = g_targethelper.get_filteredfriendlist(params)
        local rivallist = g_targethelper.get_filteredrivallist(params)
        for i=1,#friendList do
            -- table.insert(characterlist, friendList[i])
            characterlist[#characterlist + 1] = friendlist[i]
        end
        for i=1,#rivallist do
            -- table.insert(characterlist, rivallist[i])
            characterlist[#characterlist + 1] = rivallist[i]
        end
    end
    return g_targethelper.get_targetsinrange(characterlist, params, params.source)

    -- local retlist = {}
    -- for i=1,#characterlist do
    --     local a_character = characterlist[i]
    --     if a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
    --         -- table.insert(retlist, a_character)
    --         retlist[#retlist + 1] = a_character
    --     end
    -- end
    -- return retlist
end