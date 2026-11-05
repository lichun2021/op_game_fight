-- 1 当前目标

local eff297_isfindable

return function(params)
    local target = params.target
    local source = params.source
    if target ~= nil and target:canbeattack(source, params.realsearchinfo.is_ignore_canbeattack) then 
        -- if target:has_effect(297) then
        --     eff297_isfindable = eff297_isfindable or g_effecthelper.get_eff(297).is_findable
        --     if eff297_isfindable(target, source) then
        --         return { target } 
        --     end 
        -- else
            return { target }
        -- end
    end
    return nil
end
