-- targettype
local TargetType = {
    TargetCaster = 0, -- 目标不变化（与上一层目标一致）
    TargetCurrent = 1, -- 当前敌对目标
    TargetFriendAll = 2, -- 本方所有
    TargetFriendHPLowest = 3, -- 本方血量百分比最低单位
    TargetFriendRandom = 4, -- 本方随机单位
    TargetEnemyAll = 5, -- 对方所有
    TargetEnemyHPLowest = 6, -- 对方血量百分比最低单位
    TargetEnemyRandom = 7, -- 对方随机单位
    TargetCasterEnemyRange = 8, -- 自己范围内所有敌对单位
    TargetCasterFriendRange = 9, -- 自己范围内所有有方单位
    TargetCurrentEnemyRange = 10, -- 当前目标范围内所有敌对单位
    TargetCurrentFriendRange = 11, -- 当前目标范围内所有有方单位
    TargetCasterSummon = 12, -- 自己的所有召唤单位
    TargetAll = 13, -- 所有人
    TargetFurthest = 14, -- 距离最远的敌人
    TargetSkillCaster = 15, -- 技能的释放者
    TargetRandomAll = 16, -- 所有人中一个随机单位
    TargetFurthestFriend = 17,-- 最远的友方
    TargetNearestFriend = 18,-- 最近的友方
    TargetNearestEnemy = 19,-- 最近的敌方
    TargetFriendMale = 20,-- 友方的男性
    TargetEnemyMale = 21,-- 敌方的男性
    TargetRangeRandomFriendOne = 22,-- 范围内随机的一个友军
    TargetRangeRandomEnemyOne = 23,-- 范围内随机的一个敌军
    TargetFriendRandomWithoutSearcher = 24, -- 不含搜索者自己的随机友军(不会随到自己)
    TargetEnemyRandomWithoutSearcher = 25, -- 不含搜索者自己的随机敌军(不会随到自己)
    TargetOneEnemyRandomContinueHurtNotRandomTarget = 26, -- 随机一个敌人，并且若是持续伤害，每次伤害不再重新随机目标。
    TargetAllFriendSummon = 27,-- 所有友军召唤物
    TargetAllEnemySummon = 28,-- 所有敌军召唤物
    TargetAllSummon = 29,-- 所有召唤物
    TargetAllAttrHighest = 30,-- 所有敌/友方，某项属性最高的目标
    TargetFriendAttrHighest = 31,-- 友方，某项属性最高的目标
    TargetEnemyAttrHighest = 32,-- 敌方，某项属性最高的目标
    TargetFriendFemale = 33,-- 友方的女性
    TargetEnemyFemale = 34,-- 敌方的女性
    TargetAllFemale = 35,-- 敌我双方的女性
    TargetAbsoluteRangeEnemy = 36,-- 定点范围内的所有敌军，不随角色转向而转向
    TargetAbsoluteRangeFriend = 37,-- 定点范围内的所有友军，不随角色转向而转向
    TargetAbsoluteRangeAll = 38,-- 定点范围内的所有敌/友方，不随角色转向而转向
    TargetAbsoluteRangeAllWithoutSearcher = 39,-- 定点范围内的所有敌/友方(不包含自己），不随角色转向而转向
    TargetSomeoneCampAndRoleWithId = 40,-- 指定阵营的某个或某些角色id（作用对象类型填新类型，作用范围考虑支持数组填阵营和角色id）
    TargetSelfAttackRangeRandomEnemy = 41,-- 考虑自身攻击范围的随机敌人，比如近战A随机到远程B，那么A必须走到B附近才能攻击B。
    TargetByAttacker = 42, -- 攻击者
    TargetScreenAbsolutePointEnemy = 43, -- 绝对坐标内的所有敌军 绝对坐标是指，根据本阵营的出场方向，面朝敌人出场方向为x轴正方向，屏幕上方为y轴正方向，以屏幕正中心为坐标原点。
    TargetScreenAbsolutePointFriend = 44, -- 绝对坐标内的所有友军 绝对坐标是指，根据本阵营的出场方向，面朝敌人出场方向为x轴正方向，屏幕上方为y轴正方向，以屏幕正中心为坐标原点。
    TargetAllAttrLowest = 45,-- 所有敌/友方，某项属性最低的目标
    TargetFriendAttrLowest = 46,-- 友方，某项属性最低的目标
    TargetEnemyAttrLowest = 47,-- 敌方，某项属性最低的目标
    TargetEnemyMaxHurt = 48,-- 能造成最多伤害的敌方
    TargetAllEnemyInHPRange = 49,-- 生命值处于某个百分比区间的所有敌人
    TargetTraversal = 50,-- 对所有敌人依次遍历，一次只选一个，时间间隔内只能选一次，选过后找不到目标
    TargetSummonBySummoner = 51, -- 幻象的召唤者（一般用于幻象的技能）
    TargetHasMark = 52,-- 拥有指定标记的目标
    TargetSelfSummonInRange = 53,-- 范围内的施法者召唤单位
    TargetHasLabelEnemy = 54,-- 拥有所有指定标签的敌方单位
    TargetHasLabelAll = 55,-- 拥有所有指定标签的所有单位
    TargetRandomDeadFriend = 56,-- 随机一个已死亡的友军
    TargetRandomDeadEnemy = 57,-- 随机一个已死亡的敌军
    TargetRandomDeadAll = 58,-- 随机一个已死亡的友军/敌军
    TargetDeadAllFriend = 59,-- 所有已死亡的友军
    TargetDeadAllEnemy = 60,-- 所有已死亡的敌军
    TargetDeadAll = 61,-- 所有已死亡的友军/敌军
    TargetFriendHPHighest = 62,-- 血量百分比最高的友军
    TargetEnemyHPHighest = 63, -- 血量百分比最低的敌军
    -- 64 敌军之中战力最高的目标
    -- 65 友军之中战力最高的目标
    -- 66 所有人中战力最高的目标
    -- 67 效果286中记录的对自身造成伤害最高的敌方单位
    TargetAccordRingofLightFriend = 99 -- 查找友方符合（技能目标参数[为标签表中标签ID列表]）的友方
}

return TargetType