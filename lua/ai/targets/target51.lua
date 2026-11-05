-- 51 幻象的召唤者（一般用于幻象的技能）
return function(params)
    local summon_source = params.source:get_summonsource()
    if params.source:is_summon() and summon_source ~= nil and summon_source:canbeattack(params.source, params.is_ignore_canbeattack) then
        return { summon_source }
    end
    return nil
end