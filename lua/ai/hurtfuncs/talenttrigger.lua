-- 0-从来不触发, 1-默认触发，2-攻击命中触发，3-被攻击命中触发，4-闪避抵抗触发，5-被治疗时触发，7-被动作限制时
local TalentTrigger = 
{
    NEVER = 0,
    ALWAYS = 1,
    HIT = 2,
    BY_HIT = 3,
    DODGE = 4,
    BY_HURL = 5,
    IN_EFFECT_ACTION = 6
}

return TalentTrigger