-- efftype
local EffectType =
{
    eEFFECT_RESTORE                   = 101,-- 复活
    eEFFECT_RESTORE_TMP               = 102,-- 回光返照
    eEFFECT_NO_DIE                    = 103,-- 不死				
    eEFFECT_BACRIT_ADD                = 104,-- 物理暴击率提升
    eEFFECT_MACRIT_ADD                = 105,-- 法术暴击率提升
    eEFFECT_BAHIT_ADD                 = 106,-- 物理命中率降低
    eEFFECT_MAHIT_ADD                 = 107,-- 法术命中率降低
    eEFFECT_BAAVOID_ADD               = 108,-- 闪避几率
    eEFFECT_MAAVOID_ADD               = 109,-- 抵抗几率
    eEFFECT_BAREPEL_ADD               = 110,-- 物理免疫
    eEFFECT_MAREPEL_ADD               = 111,-- 法术免疫
    eEFFECT_ALL_REPEL                 = 112,-- 免疫所有攻击
    eEFFECT_REBOUND_HP                = 113,-- 反弹攻击方血量		
    eEFFECT_NO_SKILL_MARK             = 114,-- 不能释放技能 计数
    eEFFECT_NO_ATTACK_MARK            = 115,-- 不能普通攻击 计数
    eEFFECT_CHARACTER_HARD            = 116,-- 硬度buff
    eEFFECT_PHY_HARM_DOWN             = 117,-- 物理伤害减免
    eEFFECT_MAG_HARM_DOWN             = 118,-- 法术伤害减免
    eEFFECT_DEBUF_REPEL               = 119,-- 免疫所有限制状态,是指effect.statementAnimation > 0
    eEFFECT_NO_HIT_DOWN_FLY_BACK      = 120,-- 免疫被击飞击倒击退
    eEFFECT_ATTR_WITH_LOSEHP_ADD      = 121,-- 根据损失血量增加属性
    eEFFECT_CAUSE_HURT_BY_CUR_HP      = 122,-- 根据释放对象当前血量造成伤害
    eEFFECT_CAUSE_HURT_BY_LOSE_HP     = 123,-- 根据被释放者当前损失血量造成伤害
    eEFFECT_ONLY_FINAL_SKILL          = 124,-- 只能放大招(强高虚无)
    eEFFECT_CAUSE_HURT                = 125,-- 直接造成伤害的效果
    eEFFECT_OTHER_SKLIST              = 126,-- 使用备用的技能列表
    eEFFECT_CHARM                     = 127,-- 魅惑
    eEFFECT_STRUCK_HOVER              = 128,-- 击飞空中停留后落下
    eEFFECT_WUDI_NOSKILL_DISPEAR      = 129,-- 不能攻击别人也不能被攻击 仅给召唤镜像使用
    eEFFECT_ONLY_HURT_BY_CONTINUE     = 130,-- 只能被持续性伤害所伤害
    eEFFECT_HEALING_DOWN              = 131,-- 降低治疗效果
    eEFFECT_EXCHANGE_MYLOWER_HP       = 132,-- 己方血量最低单位的血量与随机敌方的血量调换
    eEFFECT_BE_TAUNTED                = 133,-- 被嘲讽 导致目标改变
    eEFFECT_WUDI_NOSKILL_APPEAR       = 134,-- 让镜像释放者出现 
    eEFFECT_REBOUND_BA_HURT           = 135,-- 反弹攻击方物理伤害
    eEFFECT_REBOUND_MA_HURT           = 136,-- 反弹攻击方魔法伤害
    eEFFECT_NO_UltraSkill             = 137,-- 不能放大招
    eEFFECT_BAREDUCE_ADD              = 138,-- 物理伤害吸收护盾
    eEFFECT_MAREDUCE_ADD              = 139,-- 法术伤害吸收护盾
    eEFFECT_EXCHANGE_BA_MA_POWER      = 140,-- 与一名随机己方的物理法术攻击力互换
    eEFFECT_ONLY_HURT_ABOVE_CONDITION = 141,-- 只能被高于阈值的伤害所伤害

    --雷光火水暗毒系伤害免疫
    eEFFECT_HURT_PROPERTY_REPL_LEI    = 142,--雷
    eEFFECT_HURT_PROPERTY_REPL_GUANG  = 143,--光
    eEFFECT_HURT_PROPERTY_REPL_HUO    = 144,--火
    eEFFECT_HURT_PROPERTY_REPL_SHUI   = 145,--水
    eEFFECT_HURT_PROPERTY_REPL_AN     = 146,--暗
    eEFFECT_HURT_PROPERTY_REPL_DU     = 147,--毒

    eEFFECT_HEAL_ADD                  = 148,-- 受治疗加成
    eEFFECT_HURT_ADD                  = 149,-- 受伤害加成
    eEFFECT_DEBUF_EFFECT              = 150,-- 免疫指定效果
    eEFFECT_GAIN_ENERGY_ON_ESCAPE     = 151,-- 闪避后获得能量
    eEFFECT_CAUSE_EXTRAHURT_BY_ALLFRIENDNUM = 152,-- 根据己方角色数量造成额外伤害(幻象个数也包含在内）
    eEFFECT_CAUSE_EXTRAHURT_BY_FRIENDNUM    = 153,-- 根据己方角色数量造成额外伤害(不包含幻象个数）
    eEFFECT_HURT_LIMIT                = 154,-- 伤害上限阈值,单次受到伤害不会超过自身最大生命值的X%
    eEFFECT_TRIGGER                   = 155,-- 当角色受到效果A时触发效果B（目标属性一列填效果A的效果id，在数值计算基准填效果B的效果id）
    eEFFECT_INVALIDATE_HURT_PROPERTY  = 156,-- 一段时间内使目标的所有攻击变成无属性（水，火等）
    eEFFECT_SHIELD_BYCOUNT_PHYSICS    = 157,-- 吸收伤害次数的物理攻击护盾
    eEFFECT_SHIELD_BYCOUNT_MAGIC      = 158,-- 吸收伤害次数的法术攻击护盾
    eEFFECT_SHIELD_BYCOUNT_BOTHPHMA   = 159,-- 吸收伤害次数的物理或法术攻击护盾
    eEFFECT_KNOCK_OUT                 = 160,-- 一击必杀
    eEFFECT_TRIGGER_ON_HEAL           = 161,-- 受到治疗时触发执行
    eEFFECT_CHARM_HALF                = 162,-- 新魅惑，将敌人魅惑后，我方可以继续攻击该目标
    eEFFECT_DEBUF_MOVECASTER          = 163,-- 免疫移动执行
    eEFFECT_TRIGGER_ANOTHER           = 164,-- 当角色受到效果A时触发效果B,与155类似,但触发时的技能释放者是该角色
    eEFFECT_CLEAR_BUFF                = 165,-- 驱散指定类型的buff效果执行(根据效果执行的isBuff字段)
    eEFFECT_PHYCRIT_MUL_REDUCE_ADD    = 166,-- 物理暴击倍数减免
    eEFFECT_MAGCRIT_MUL_REDUCE_ADD    = 167,-- 法术暴击倍数减免
    eEFFECT_PHYCRIT_PROB_REDUCE_ADD   = 168,-- 物理暴击率减免
    eEFFECT_MAGCRIT_PROB_REDUCE_ADD   = 169,-- 法术暴击率减免
    eEFFECT_PHYCRIT_REDUCE_ADD        = 170,-- 物理暴击值减免
    eEFFECT_MAGCRIT_REDUCE_ADD        = 171,-- 法术暴击值减免
    eEFFECT_FATAL_HURT_TRANS          = 172,-- 致命伤害转移到效果施法者
    eEFFECT_SUMMON_HARM_ADD           = 173,-- 受到幻象的伤害降低/提高
    eEFFECT_CAUSE_ADD_PROPERTY_HURT   = 174,-- 施法者属性伤害加成（火，雷，水等 !目标属性字段对应属性类型）
    eEFFECT_HIT_GAIN_ENERGY_ADD       = 175,-- 伤害在命中目标时，目标因受伤所获能量增加/减少
    eEFFECT_CAUSE_ON_ATTRDELTA        = 176,-- 根据属性A差值扣除目标属性B
    eEFFECT_MAG_BLOOD_SUCK            = 177,-- 法术伤害吸血
    eEFFECT_LINKEN_SPHERE             = 178,-- 林肯法球（在抵挡伤害后可以触发另一个效果）
    eEFFECT_FLIMSY_BURYING            = 179,-- 薄葬 血量冻结在特定值
    eEFFECT_SHARE_HURT                = 180,-- 伤害连结
    eEFFECT_CRITHIT_NOFAIL            = 181,-- 持续时间内若连续X次攻击不是暴击，则下一次攻击必定命中且暴击
    eEFFECT_NO_TURN_ACTION            = 182,-- 战斗中不能转向
    eEFFECT_PICK_RANGE_SKILL          = 183,-- 根据目标位置使用指定序列号的技能
    eEFFECT_LIMIT_MOVE_RANGE          = 184,-- 限制移动范围，按战场百分比算
    eEFFECT_LAZY_ENTER_BATTLE         = 185,-- 延迟入场
    eEFFECT_THRESHOLD_EXEC            = 186,-- 比较目标属性和效果阈值决定是否触发另一个执行
    eEFFECT_COUNT_MARK_LEVEL          = 187,-- 统计标记层数
    eEFFECT_HEAL_BY_EFFECT_COUNT      = 188,-- 根据场上同一施法者释放的指定效果ID的统计值给目标加血
    eEFFECT_EXEC_BY_JUEXINGLV         = 189,-- 根据当前角色觉醒等级调用不同的执行id
    eEFFECT_PHY_HARM_FACE_DOWN        = 190,-- 受到来自面朝方向的物理攻击伤害大幅降低
    eEFFECT_MAG_HARM_FACE_DOWN        = 191,-- 受到来自面朝方向的法术攻击伤害大幅降低
    eEFFECT_ALL_HARM_FACE_DOWN        = 192,-- 受到来自面朝方向的所有攻击伤害大幅降低
    eEFFECT_INSERT_ATTACK_ACTION      = 193,-- 已经解锁某技能,立即释放(无视大招)
    eEFFECT_EXTEND_BUFF_TICK          = 194,-- 改变指定类型的buff效果执行生效时持续时间(根据效果执行的isBuff字段)
    eEFFECT_DEBUF_MOVE_EX             = 195,-- 免疫敌方阵营的移动执行
    eEFFECT_BYPASS_AVOID              = 196,-- 无视目标闪避抵抗
    eEFFECT_ATTACK_GAIN_ENERGY_ADD    = 197,-- 效果拥有者攻击获取能量增加
    eEFFECT_ATTR_CONVERT_ADD          = 198,-- 根据一级属性影响二级属性
    eEFFECT_INSERT_ATTACK_ACTION_PLUS = 199,-- 已经解锁某技能,立即释放(无视大招且判断切换普攻)

    eEFFECT_ONLY_FOR_DAZE             = 200,-- 用于眩晕等不改变属性的效果，直接执行对应的特效
    eEFFECT_ONLY_FOR_DAZE_PLUS        = 201,-- 用于眩晕等不改变属性的效果，直接执行对应的特效
    eEFFECT_MISS_EXCUTE               = 202,-- 未命中时触发
    eEFFECT_ESCAPE_PHY_EXCUTE         = 203,-- 物理闪避时触发
    eEFFECT_ESCAPE_MAG_EXCUTE         = 204,-- 法术抵抗时触发
    eEFFECT_GOBACK_SAVEPOSITION		  = 205,-- 回到技能施放时的初始位置的效果
    eEFFECT_COUNT_MARK_LEVEL_PLUS     = 206,-- 统计标记层数(排除死亡角色)
    eEFFECT_PROP_HURT_ADD             = 207,-- 属性伤害减免
    eEFFECT_BEHIT_GAIN_ENERGY_ADD     = 208,-- 效果拥有者受击获取能量增加
    eEFFECT_EXCUTE_EXEC               = 209,-- 立即触发另一个执行
    eEFFECT_HEAL_BY_EFFECT_COUNT_PLUS = 210,-- 根据场上同一施法者释放的指定效果ID的统计值给目标加血,攻击力只取第一次加血时的值
    eEFFECT_RESIST_N_REBOUND          = 211,-- 完全抵挡下一次攻击并反伤（反伤数值/百分比需要可配置，若填0则不反伤），该效果触发后移除


    eEFFECT_PROMOTE_SKILL_LEVEL		  = 212, -- 提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）
    eEFFECT_IGNORE_NO_DIE             = 213, -- 使指定阵营的指定角色的某个技能在造成伤害（伤害执行）时无视敌人的不死
    eEFFECT_ATTACKLIST_ADD_ONE		  = 214, -- 目标的攻击序列中非普攻技能之前多加一个技能。
    eEFFECT_CREATE_HURT_ADD_VALUE	  = 215, -- 若对目标造成的伤害低于X点/X百分比目标血量，则附加理论计算值（无视防御）一定百分比的纯粹伤害。

    eEFFECT_IGNORE_SILENCE            = 216,-- 不受沉默效果影响
    eEFFECT_ADD_HURT_ON_DIST          = 217,-- 根据距离增加伤害
    eEFFECT_ADD_HURT_ON_SAME_TARGET   = 218,-- 对同一敌人连续攻击将造成更多伤害
    eEFFECT_EXCUTE_ON_ATTR            = 219,-- 目标属性达到条件触发执行
    eEFFECT_DUEL                      = 220,-- 决斗
    eEFFECT_CRIT_ON_ATTR              = 221,-- 目标属性达到条件时必定暴击
    eEFFECT_ADD_ATTR_ON_ATTR          = 222,-- 根据某一项属性值A赋予另一项属性值B
    eEFFECT_BE_TAUNTED_PRIME          = 223,-- 高优先级嘲讽

    eEFFECT_EXTEND_EXIST_BUFF         = 224,-- 延长已有buff持续时间
    eEFFECT_DEBUF_CLEAR_BODY_EXEC     = 225,-- 清除并免疫身体执行
    eEFFECT_SHIELD_BYCOUNT_ALL_HURT   = 226,-- 吸收伤害次数的所有类型攻击护盾 物理 法术 纯粹
    eEFFECT_EXTEND_HIT_FLYNDOWN       = 227,-- 延长 击飞 击倒 状态持续时间
    eEFFECT_CAST_IFHAS_SKILL          = 228,-- 已经解锁某技能,立即触发执行
    eEFFECT_NO_HIT_FORCE_DOWN         = 229,-- 免疫强制击倒目标(受创类型7)
    eEFFECT_CAST_ON_HURT_RESULT       = 230,-- 处于某种受创类型时连续执行
    eEFFECT_SKIP_SKILL_SEQ            = 231,-- 跳过攻击序列中指定序号的技能
    eEFFECT_SPECIFY_SEX_CONDITION     = 232,-- 我方（或者敌方）存活的角色中，性别A与性别B的人数满足指定关系
    eEFFECT_CHANGE_SEX_EFFECT		  = 233,-- 在一段时间内改变性别
    eEFFECT_ACCORDANCE_SEX_EXEC		  = 234,-- 根据指定性别调用不同执行（主要是动画执行）。
    eEFFECT_BAMA_CRIT_NCAST           = 235,-- 持续时间内，如果指定类型的（物理或法术或治疗）伤害击中敌人时，必定暴击
    eEFFECT_PRIMARYATTRIBUTE_CONDITION = 236,-- 若触发时主属性A与其他两个主属性BC的关系满足条件，则触发执行id
    eEFFECT_JUDGE_SUMMON_COUNT		   = 237,-- 拥有效果时（瞬时），判断自己的召唤物数量。
    eEFFECT_CURATT_MAXATT_CONDITION	   = 238,-- 瞬间释放（多为伤害执行中的击中执行），目标当前属性与当前最大属性的比值与填写的百分比作比较，决定触发执行。
    eEFFECT_SLIP_NCAST                = 239,-- 开始释放技能(释放条件为0且正常释放)时有概率滑倒并触发执行
    eEFFECT_SKIP_SKILLCD_NCAST        = 240,-- 跳过技能公共冷却时间并释放一个技能
    eEFFECT_SACRIFICE_HURT_RECV       = 241,-- 牺牲 伤害承担
    eEFFECT_SACRIFICE_HURT_SEND       = 242,-- 牺牲 伤害转移
    eEFFECT_ADD_HURT                  = 243,-- 增加目标造成的所有伤害(不含加血)
    eEFFECT_REVIVE                    = 244,-- 新复活
    eEFFECT_HURT_BY_DEADFRIEND        = 245,-- 根据己方已阵亡角色数量对目标造成伤害
    eEFFECT_KO_BY_COUNT               = 246,-- 目标在受到指定次数的直接攻击后死亡
    eEFFECT_RESIST_FATAL_HURT_NCAST   = 247,-- 抵挡一次致命伤害并触发一个执行
    eEFFECT_FORCE_ATTACK_SELF         = 248,-- 强制普攻自己N次
    eEFFECT_CAST_BY_TEAM_HP_RATIO     = 249,-- 团队血量百分比（指定阵营）达到条件触发执行
    eEFFECT_CAST_BY_TEAM_DEAD_CNT     = 250,-- 场上死亡人数（指定阵营）达到条件触发执行
    eEFFECT_CHANGE_ATTACK_SPEED       = 251,-- 改变攻击速度
    eEFFECT_BALANCE_ATTR              = 252,-- 拉平目标属性(取属性高的值)
    eEFFECT_TAUNT_EXTRA               = 253,-- 新新魅惑(目标只能受到真正敌对阵营的伤害，自己会攻击队友，队友不会攻击被控制的单位)

    eEFFECT_DUR_HIT_CRIT_EFFECT		  = 254, -- 持续时间内，如果物理法术伤害击中任何敌人且暴击，则触发一个执行（有触发间隔）

    eEFFECT_ALLOW_NEGTIVE_HEAL        = 255,-- 允许受到负治疗值
    eEFFECT_CAST_WHEN_FRIEND_SUMMON   = 256,-- 每当友军制造1个幻象就触发1个执行
    eEFFECT_CAST_WHEN_FRIEND_DIE      = 257,-- 友军发生减员时触发执行
    eEFFECT_CAST_BY_SPEC_LABEL_HURT   = 258,-- 受到特殊标签角色伤害时触发
    eEFFECT_HURT_DOWN_FROM_MARK       = 259,-- 受到指定标记原始伤害减免
    eEFFECT_CAST_BY_SPEC_EFFECT       = 260,-- 受指定效果时触发执行
    eEFFECT_CANNOT_FIND_TARGET        = 261,-- 不能攻击指定阵营的指定角色
    eEFFECT_CRIT_IF_HIT               = 262,-- 若伤害命中则必定暴击
    eEFFECT_CANNOT_BE_FOUND_BY_SIDE   = 263,-- 不能被指定阵营选中
    eEFFECT_CAST_BY_SPEC_STATE        = 264,-- 处于指定状态时触发执行
    eEFFECT_SHIELD_TRANSFER_HURT      = 265,-- 屏障球
    eEFFECT_STATE_VERTIGO             = 266,-- 眩晕（可改变状态）
    eEFFECT_CREATE_HURT_ADD_VALUE_NEW = 267, -- 新增效果：若对目标造成的伤害[是否只取来自施法者某执行id（伤害执行&效果执行）的伤害。]低于X点/X百分比目标血量，则附加理论计算值（无视防御）一定百分比的纯粹伤害。
    eEFFECT_DIRECT_KO                 = 268,-- 直接致死，无视buff，不触发死亡执行eEFFECT_CAUSE_HURT_BY_LOSE_HP
    eEFFECT_CAUSE_HURT_BY_CURR_HP     = 269,-- 根据目标当前血量百分比造成伤害
    eEFFECT_CONSUME_HP_ADD_CRUITHURT  = 270,-- 消耗自身当前生命值，造成额外纯粹伤害
    eEFFECT_ENDURE_HURT_BY_CONDITION  = 271,-- 承受伤害小于一定阈值伤害不生效，且触发执行id
    eEFFECT_FATAL_HURT_DO_EFFECT      = 272,-- 受到致命伤害（本次伤害会造成死亡）时，保留1滴血量不死，触发一个执行
    eEFFECT_ABSORB_ALLTYPE_HURT_SHIELD = 273,-- 吸收所有伤害类型的护盾，护盾的数值根据自身的某项属性
    eEFFECT_COPY_CREATURE_ANIMATION   = 274,-- 暂时变身为目标动画
    eEFFECT_IGNORE_FIGHT_PARAM_LIMIT  = 275,-- 拥有该效果的角色在攻击时计算暴击，闪避，抵抗，吸血，无视公式上下限
    eEFFECT_CAST_BY_TEAM_LOSE_HP      = 276,-- 友军每损失血量达到条件触发执行
    eEFFECT_CHANGE_ACCU_MAX_TICK      = 277,-- 改变蓄力大招的最大蓄力时间
    eEFFECT_CHANGE_HPADD1_BY_MARK     = 278,-- 根据标记层数改变吸血等级
    eEFFECT_CAST_BY_STATE             = 279,-- 处于指定状态时触发执行
    eEFFECT_FOCUS_TARGET              = 280,-- 集火
    eEFFECT_FREEZE_IN_EFFECT_ACTION   = 281,-- 眩晕石化期间暂停并触发执行

    -- 11.0
    -- 282 受到某种类型标签的角色攻击时必定受到暴击
    -- 283 反伤 以未计算减伤前的伤害值作为反伤初始值
    -- 284 根据飞行道具飞行距离触发执行
    -- 285 根据觉醒等级触发执行
    -- 285 判断觉醒等级触发执行
    -- 286 恢复上一次直接攻击造成的一定百分比的伤害(不包括效果执行的持续伤害)
    -- 287 每受到X次攻击，第X次会触发执行
    -- 288 统计目标受到的伤害
    -- 289 释放统计的伤害
    -- 290 攻击背后必定暴击
    -- 291 受到指定ID的角色伤害时，附加额外伤害到初始伤害
    -- 292 使指定效果失效
    -- 293 无法转身，且无法攻击背后的敌人
    -- 294 不能使用含有移动执行的技能
    -- 295 强制抓取，目标无法行动，但可受到移动执行影响
    -- 296 使死亡触发技能无效
    -- 297 不能被指定阵营的单体目标类型选中
    -- 298 免疫295
    -- 299 提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）
    -- 300 游戏音量渐变
    -- 301 清空288统计的伤害

    -- 12.0
    -- 302 角色处于丢失状态时触发执行
    -- 303 当角色产生位移（X轴、Y轴）时[移动执行、击飞、击倒]触发执行
    -- 304 被认为是幻象或英雄

    -- 13.0
    -- 305 不能被指定阵营的技能施法者选中
    -- 306 驱散并免疫指定效果执行的效果ID
    -- 307 保护指定效果执行的效果ID
    -- 308 根据指定技能的等级及效果数值区间，决定是否触发效果
    -- 309 释放指定类型的技能时触发执行
    -- 310 将指定属性转换为另一项属性

    -- 14.0
    -- 311 判断目标身上是否有指定执行ID

    -- 15.0
    -- 312 同时吸收物理和法术伤害的护盾

    -- 313 搜索最近目标时优先寻找非幻象
    -- 314 必杀技替换立绘
    -- 315 吸收能量的护盾
    -- 316 吸收治疗量的护盾
    -- 317 目标血量降低指定数量/百分比后触发执行
    -- 318 目标能量降低指定数量/百分比后触发执行
    -- 319 判断场上有几个带有指定标签的角色触发执行
    -- 320 吸收指定属性类型伤害的护盾
    -- 321 驱散指定的伤害执行
    -- 322 保护指定的伤害执行不被驱散
    -- 323 对盾的效果造成伤害，只会打盾不会对人造成伤害
    -- 324 判断目标的技能等级

    -- OP2 22.3
	-- 325 指定属性阈值
	
	-- OP2 22.4
	-- 326 完全抵挡一次伤害

	-- 25.0
	-- 327 拥有效果的目标，持续时间内受到伤害，对伤害来源的目标触发执行
	-- 328 释放技能时触发执行
	-- 329 强制转向(不打断当前动作)

    -- 14.0
	-- 400 改变目标状态的效果执行，只有在成功改变目标状态（即眩晕石化）后，才会显示绑定的UI特效

	-- id > 500 : native
	-- 25.0
	-- 501 效果拥有者攻击时无视目标的物理护甲
	-- 502 效果拥有者攻击时无视目标的法术抗性

	--
	-- 999 空效果，只用来显示对应的UI特效

    -- OP2 海战用
    -- 10001 (lua) 白刃战船只护盾(吸收所有伤害,优先级高-在OP1的护盾前使用)
}

