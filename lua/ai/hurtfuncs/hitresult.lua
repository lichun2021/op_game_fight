local HitResult = 
{
    HitResultType = 
    { 
        HIT_HASHURT = 0,
        HIT_NOHURT  = 1, -- 伤害值被减为0(抵消伤害数值的效果)
        HIT_BYPASS  = 2 -- 被盾吸收且不走其他的标记触发和击中执行
    },

    BlockType = 
    { 
        HIT_UNBLOCKED = 0,
        HIT_BLOCKED   = 1 -- 被直接免疫(不考虑伤害数值的免疫，如按类型免疫效果和按次数免疫护盾)
    }
}

return HitResult