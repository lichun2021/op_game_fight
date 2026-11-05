-- istargettyperandom

local ts = {4,7,16,22,23,24,25,26,41,56,57,58}
local randomtypes = {}
for i=1, #ts do
    randomtypes[ts[i]] = 1
end

local function istargettyperandom(type)
    return randomtypes[type] ~= nil
end

return istargettyperandom