-- 12 自己的所有召唤单位
local CD_MYID = g_cddata.CD_MYID
return function(params)
    if params.source ~= nil and params.source:is_summon() ~= true then 
        local characterlist = g_targethelper.pick_filteredfriendlist(params)
        local retlist = {}
        for i=1,#characterlist do
            local a_character = characterlist[i]
            if a_character:is_summon() and a_character:get_summonsource():get_data(CD_MYID) == params.source:get_data(CD_MYID) and a_character:canbeattack(params.source, params.is_ignore_canbeattack) then
                -- table.insert(retlist, a_character)
                retlist[#retlist + 1] = a_character
            end
        end
        return retlist
    else
        return nil
    end
end