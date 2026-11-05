-- attrtype
local Attr = {
    ID = 0,         -- 角色id
    HPTop = 1,      -- 最大血量 显示用
    Lv = 2,         -- 等级
    X = 3,
    Y = 4,
    Z = 5,

    -- 一级属性
    PHY = 6,        -- 力量
    AGILE = 7,      -- 敏捷
    INT = 8,        -- 智力
    ACCURATE = 9,   -- 精准
    SPIRIT = 10,    -- 精神

    -- 二级属性
    HP = 11,        -- 生命
    Energy = 12,    -- 能量
    Speed = 13,     -- 移动速度
    BAPower = 14,   -- 物攻
    MAPower = 15,   -- 法术攻击
    BACrit = 16,    -- 物理暴击
    MACrit = 17,    -- 法术暴击
    BACritM = 18,   -- 物理暴击倍数
    MACritM = 19,   -- 法术暴击倍数
    BAHit = 20,     -- 物理命中
    MAHit = 21,     -- 法术命中
    BARepel = 22,   -- 物理护甲
    MARepel = 23,   -- 法术抗性
    BABreak = 24,   -- 物理穿透
    MABreak = 25,   -- 法术穿透
    BADodge = 26,   -- 闪避几率
    MADodge = 27,   -- 抵抗几率
    HPAdd1 = 28,    -- 吸血等级
    HPAdd2 = 29,    -- 生命值回复 过节点回血
    ENAdd1 = 30,    -- 过节点回能量
    HPAdd3 = 31,    -- 治疗加成
    AInterval = 32, -- 施法间隔
    AIDist = 33,    -- 攻击距离
    BACritResist = 34, -- 物理抗暴
    MACritResist = 35, -- 法术抗暴
    BACritMAdd = 36,   -- 物理暴击倍数减免
    MACritMAdd = 37,   -- 法术暴击倍数减免
    FuncHurtAdd = 38, -- 增伤
    FuncHurtSub = 39, -- 减伤
    BACritAdd = 40, -- 额外物理暴击
    MACritAdd = 41, -- 额外法术暴击
    ATTR_CNT = 42,

    ENG = 12, -- 能量
    SPEED = 13, -- 移动速度
    PHY_AT = 14, -- 物攻
    MAG_AT = 15, -- 法术攻击
    PHY_CRIT = 16, -- 物理暴击
    MAG_CRIT = 17, -- 法术暴击
    PHY_CRIT_MUL = 18, -- 物理暴击倍数
    MAG_CRIT_MUL = 19, -- 法术暴击倍数
    PHY_HIT = 20, -- 物理命中
    MAG_HIT = 21, -- 法术命中
    PHY_DE = 22, -- 物理护甲
    MAG_DE = 23, -- 法术抗性
    PHY_PE = 24, -- 物理穿透
    MAG_PE = 25, -- 法术穿透
    DODGE_RATE = 26, -- 闪避几率
    RESIST_RATE = 27, -- 抵抗几率
    BLOOD_SUCK = 28, -- 吸血等级
    HP_RECOVER = 29, -- 生命值回复
    ENG_RECOVER = 30, -- 能量回复
    CURE = 31, -- 治疗效果
    ATT_INTER = 32, -- 施法间隔
    AT_DIST = 33, -- 攻击距离
    PHY_CRIT_RISIST = 34,-- 物理抗暴
    MAG_CRIT_RISIST = 35,-- 法术抗暴
    PHY_CRIT_MUL_ADD = 36,-- 物理暴击倍数减免
    MAG_CRIT_MUL_ADD = 37,-- 法术暴击倍数减免
    FUNC_HURT_ADD = 38, -- 增伤
    FUNC_HURT_SUB = 39, -- 减伤
    PHY_CRIT_ADD = 40, -- 额外物理暴击
    MAG_CRIT_ADD = 41, -- 额外法术暴击

    --新增加元素之力属性 cui_2023_6_29
    SLASH_ADD = 42,--斩击属性增伤
    SLASH_SUB = 43,--斩击属性减伤
    LIGHT_ADD = 44,--光属性增伤
    LIGHT_SUB = 45,--光属性减伤
    DARK_ADD = 46,--暗属性增伤
    DARK_SUB = 47,--暗属性减伤
    WATER_ADD = 48,--水属性增伤
    WATER_SUB = 49,--水属性减伤
    POISON_ADD = 50,--毒属性增伤
    POISON_SUB = 51,--毒属性减伤
    ELECTRIC_ADD = 52,--雷属性增伤
    ELECTRIC_SUB = 53,--雷属性减伤
    FIRE_ADD = 54,--火属性增伤
    FIRE_SUB = 55,--火属性减伤
    NONE_ADD = 56,--无属性增伤
    NONE_SUB = 57,--无属性减伤
    FINAL_ADD = 58,
    FINAL_SUB = 59,
}
return Attr