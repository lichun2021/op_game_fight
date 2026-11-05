-- 15 技能的释放者

local theactionhelper

return function(params)
    theactionhelper = theactionhelper or g_actionhelper
    local skillcaster = params.skillcaster
    if theactionhelper.is_deadaction(skillcaster:get_currentaction()) == false then
        return { skillcaster }
    end
    return nil
end