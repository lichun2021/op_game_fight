-- 37 定点范围内的所有友军，不随角色转向而转向
return function(params)
    -- params.exclude_myid = params.source:get_myid()
    return g_targethelper.get_characterinabsoluterange(g_targethelper.pick_filteredfriendlist(params), params, params.source)
end