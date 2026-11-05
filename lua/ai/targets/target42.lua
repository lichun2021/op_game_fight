-- 42 攻击者
return function(params)
    local thetarget = params.source:get_val("attacker")
    if thetarget ~= nil and thetarget:canbeattack(params.source, params.is_ignore_canbeattack) then return {thetarget}
    elseif params.target ~= nil and params.target:canbeattack(params.source, params.is_ignore_canbeattack) then return {params.target}
    else return nil
    end
end