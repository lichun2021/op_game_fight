-- 248 强制普攻自己N次
local eff = {}

eff.check_do = function(character, param)
  if not character then 
    return 0
  end
    return aieffectcaster.check_exert_special_effect(character, 248, param)
end

return eff