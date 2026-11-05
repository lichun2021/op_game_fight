-- recursiverandomonevalidexec
local function retfunc(target, execs)
    -- local the_execs = {table.unpack(execs)}
    local the_execs = {}
    for i=1,#execs do
        local execid = execs[i]
        if execid > 0 then
            the_execs[#the_execs + 1] = execid
        end
    end
    local execs_len = #the_execs
    if execs_len <= 1 then return the_execs end

    --[[
    local avgprob = 100 / execs_len
    if avgprob <= 0 then return {} end 

    local execprob = aicontext.get_randomnumber()
    local hitidx = 1

    while execprob > avgprob do
        hitidx = hitidx + 1
        avgprob = avgprob + avgprob
    end
    ]]

    local hitidx = aicontext.get_randomnumber_within_range(execs_len) + 1

    if hitidx > execs_len then 
        hitidx = execs_len
    end

    local hitid = the_execs[hitidx]
    if hitid <= 0 or (g_exechelper.is_effectexec(hitid) and g_effecthelper.is_anyinnerexec_incd(target, hitid)) then
        table.remove( the_execs, hitidx )
        the_execs = retfunc(target, the_execs)
    else
        the_execs = { hitid }
    end

    return the_execs
end

return retfunc