-- 39 定点范围内的所有敌/友方(不包含自己），不随角色转向而转向
local CD_MYID = g_cddata.CD_MYID
return function(params)
    params.exclude_myid = params.source:get_data(CD_MYID)
    return g_targethelper.get_characterinabsoluterange(g_targethelper.pick_filteredalllist(params), params, params.source)
end