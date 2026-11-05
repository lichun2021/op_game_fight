-- stathelper

local theaibattlestat = aibattlestat

local function append_hurt( source, hurttype, realhpdelta, target )
    if source:get_myid() == target:get_myid() then return end
    theaibattlestat.append_hurt(source, hurttype, realhpdelta, target)
end

return {
    append_hurt = append_hurt
}