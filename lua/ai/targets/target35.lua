-- 35 所有女性
return function( params )
    local find_friendfemale = require "targets/target33"
    if params.filterlist ~= nil then
        return find_friendfemale(params)
    end

    local find_rivalfemale = require "targets/target34"
    local retlist = find_friendfemale(params)
    local rivallist = find_rivalfemale(params)
    for i=1,#rivallist do 
        -- table.insert(retlist, rivallist[i])
        retlist[#retlist + 1] = rivallist[i]
    end
    return retlist
end