//
//  AIContinuousEffectCaster.h
//  OnePiece
//
//  Created by Beileng Shan on 14/5/4.
//
//

#ifndef __OnePiece__AIContinuousEffectCaster__
#define __OnePiece__AIContinuousEffectCaster__

#include <vector>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include "CommonType.h"
#include "AINode.h"

namespace OnePiece {
	class CharacterData;
	class AIContext;
	class AIProjectile;

	enum EffectType
	{
        eEFFECT_RESTORE                   = 101,// 复活
        eEFFECT_RESTORE_TMP               = 102,// 回光返照
        eEFFECT_NO_DIE                    = 103,// 不死				
        eEFFECT_BACRIT_ADD                = 104,// 物理暴击率提升
        eEFFECT_MACRIT_ADD                = 105,// 法术暴击率提升
        eEFFECT_BAHIT_ADD                 = 106,// 物理命中率降低
        eEFFECT_MAHIT_ADD                 = 107,// 法术命中率降低
        eEFFECT_BAAVOID_ADD               = 108,// 闪避几率
        eEFFECT_MAAVOID_ADD               = 109,// 抵抗几率
        eEFFECT_BAREPEL_ADD               = 110,// 物理免疫
        eEFFECT_MAREPEL_ADD               = 111,// 法术免疫
        eEFFECT_ALL_REPEL                 = 112,// 免疫所有攻击
        eEFFECT_REBOUND_HP                = 113,// 反弹攻击方血量		
        eEFFECT_NO_SKILL_MARK             = 114,// 不能释放技能 计数
        eEFFECT_NO_ATTACK_MARK            = 115,// 不能普通攻击 计数
        eEFFECT_CHARACTER_HARD            = 116,// 硬度buff
        eEFFECT_PHY_HARM_DOWN             = 117,// 物理伤害减免
        eEFFECT_MAG_HARM_DOWN             = 118,// 法术伤害减免
        eEFFECT_DEBUF_REPEL               = 119,// 免疫所有限制状态,是指effect.statementAnimation > 0
        eEFFECT_NO_HIT_DOWN_FLY_BACK      = 120,// 免疫被击飞击倒击退
        eEFFECT_ATTR_WITH_LOSEHP_ADD      = 121,// 根据损失血量增加属性
        eEFFECT_CAUSE_HURT_BY_CUR_HP      = 122,// 根据释放对象当前血量造成伤害
        eEFFECT_CAUSE_HURT_BY_LOSE_HP     = 123,// 根据被释放者当前损失血量造成伤害
        eEFFECT_ONLY_FINAL_SKILL          = 124,// 只能放大招(强高虚无)
        eEFFECT_CAUSE_HURT                = 125,// 直接造成伤害的效果
        eEFFECT_OTHER_SKLIST              = 126,// 使用备用的技能列表
        eEFFECT_CHARM                     = 127,// 魅惑
        eEFFECT_STRUCK_HOVER              = 128,// 击飞空中停留后落下
        eEFFECT_WUDI_NOSKILL_DISPEAR      = 129,// 不能攻击别人也不能被攻击 仅给召唤镜像使用
        eEFFECT_ONLY_HURT_BY_CONTINUE     = 130,// 只能被持续性伤害所伤害
        eEFFECT_HEALING_DOWN              = 131,// 降低治疗效果
        eEFFECT_EXCHANGE_MYLOWER_HP       = 132,// 己方血量最低单位的血量与随机敌方的血量调换
        eEFFECT_BE_TAUNTED                = 133,// 被嘲讽 导致目标改变
        eEFFECT_WUDI_NOSKILL_APPEAR       = 134,// 让镜像释放者出现 
        eEFFECT_REBOUND_BA_HURT           = 135,// 反弹攻击方物理伤害
        eEFFECT_REBOUND_MA_HURT           = 136,// 反弹攻击方魔法伤害
        eEFFECT_NO_UltraSkill             = 137,// 不能放大招
        eEFFECT_BAREDUCE_ADD              = 138,// 物理伤害吸收护盾
        eEFFECT_MAREDUCE_ADD              = 139,// 法术伤害吸收护盾
        eEFFECT_EXCHANGE_BA_MA_POWER      = 140,// 与一名随机己方的物理法术攻击力互换
        eEFFECT_ONLY_HURT_ABOVE_CONDITION = 141,// 只能被高于阈值的伤害所伤害

        //雷光火水暗毒系伤害免疫
        eEFFECT_HURT_PROPERTY_REPL_LEI    = 142,//雷
        eEFFECT_HURT_PROPERTY_REPL_GUANG  = 143,//光
        eEFFECT_HURT_PROPERTY_REPL_HUO    = 144,//火
        eEFFECT_HURT_PROPERTY_REPL_SHUI   = 145,//水
        eEFFECT_HURT_PROPERTY_REPL_AN     = 146,//暗
        eEFFECT_HURT_PROPERTY_REPL_DU     = 147,//毒

        eEFFECT_HEAL_ADD                  = 148,// 受治疗加成
        eEFFECT_HURT_ADD                  = 149,// 受伤害加成
        eEFFECT_DEBUF_EFFECT              = 150,// 免疫指定效果
        eEFFECT_GAIN_ENERGY_ON_ESCAPE     = 151,// 闪避后获得能量
		eEFFECT_CAUSE_EXTRAHURT_BY_ALLFRIENDNUM = 152,// 根据己方角色数量造成额外伤害(幻象个数也包含在内）
		eEFFECT_CAUSE_EXTRAHURT_BY_FRIENDNUM    = 153,// 根据己方角色数量造成额外伤害(不包含幻象个数）
        eEFFECT_HURT_LIMIT                = 154,// 伤害上限阈值,单次受到伤害不会超过自身最大生命值的X%
        eEFFECT_TRIGGER                   = 155,// 当角色受到效果A时触发效果B（目标属性一列填效果A的效果id，在数值计算基准填效果B的效果id）
        eEFFECT_INVALIDATE_HURT_PROPERTY  = 156,// 一段时间内使目标的所有攻击变成无属性（水，火等）
        eEFFECT_SHIELD_BYCOUNT_PHYSICS    = 157,// 吸收伤害次数的物理攻击护盾
        eEFFECT_SHIELD_BYCOUNT_MAGIC      = 158,// 吸收伤害次数的法术攻击护盾
        eEFFECT_SHIELD_BYCOUNT_BOTHPHMA   = 159,// 吸收伤害次数的物理或法术攻击护盾
        eEFFECT_KNOCK_OUT                 = 160,// 一击必杀
        eEFFECT_TRIGGER_ON_HEAL           = 161,// 受到治疗时触发执行
        eEFFECT_CHARM_HALF                = 162,// 新魅惑，将敌人魅惑后，我方可以继续攻击该目标
        eEFFECT_DEBUF_MOVECASTER          = 163,// 免疫移动执行
        eEFFECT_TRIGGER_ANOTHER           = 164,// 当角色受到效果A时触发效果B,与155类似,但触发时的技能释放者是该角色
        eEFFECT_CLEAR_BUFF                = 165,// 驱散指定类型的buff效果执行(根据效果执行的isBuff字段)
        eEFFECT_PHYCRIT_MUL_REDUCE_ADD    = 166,// 物理暴击倍数减免
        eEFFECT_MAGCRIT_MUL_REDUCE_ADD    = 167,// 法术暴击倍数减免
        eEFFECT_PHYCRIT_PROB_REDUCE_ADD   = 168,// 物理暴击率减免
        eEFFECT_MAGCRIT_PROB_REDUCE_ADD   = 169,// 法术暴击率减免
        eEFFECT_PHYCRIT_REDUCE_ADD        = 170,// 物理暴击值减免
        eEFFECT_MAGCRIT_REDUCE_ADD        = 171,// 法术暴击值减免
        eEFFECT_FATAL_HURT_TRANS          = 172,// 致命伤害转移到效果施法者
        eEFFECT_SUMMON_HARM_ADD           = 173,// 受到幻象的伤害降低/提高
        eEFFECT_CAUSE_ADD_PROPERTY_HURT   = 174,// 施法者属性伤害加成（火，雷，水等 !目标属性字段对应属性类型）
        eEFFECT_HIT_GAIN_ENERGY_ADD       = 175,// 伤害在命中目标时，目标因受伤所获能量增加/减少
        eEFFECT_CAUSE_ON_ATTRDELTA        = 176,// 根据属性A差值扣除目标属性B
        eEFFECT_MAG_BLOOD_SUCK            = 177,// 法术伤害吸血
        eEFFECT_LINKEN_SPHERE             = 178,// 林肯法球（在抵挡伤害后可以触发另一个效果）
        eEFFECT_FLIMSY_BURYING            = 179,// 薄葬 血量冻结在特定值
        eEFFECT_SHARE_HURT                = 180,// 伤害连结
        eEFFECT_CRITHIT_NOFAIL            = 181,// 持续时间内若连续X次攻击不是暴击，则下一次攻击必定命中且暴击
        eEFFECT_NO_TURN_ACTION            = 182,// 战斗中不能转向
        eEFFECT_PICK_RANGE_SKILL          = 183,// 根据目标位置使用指定序列号的技能
        eEFFECT_LIMIT_MOVE_RANGE          = 184,// 限制移动范围，按战场百分比算
        eEFFECT_LAZY_ENTER_BATTLE         = 185,// 延迟入场
        eEFFECT_THRESHOLD_EXEC            = 186,// 比较目标属性和效果阈值决定是否触发另一个执行
        eEFFECT_COUNT_MARK_LEVEL          = 187,// 统计标记层数
        eEFFECT_HEAL_BY_EFFECT_COUNT      = 188,// 根据场上同一施法者释放的指定效果ID的统计值给目标加血
        eEFFECT_EXEC_BY_JUEXINGLV         = 189,// 根据当前角色觉醒等级调用不同的执行id
        eEFFECT_PHY_HARM_FACE_DOWN        = 190,// 受到来自面朝方向的物理攻击伤害大幅降低
        eEFFECT_MAG_HARM_FACE_DOWN        = 191,// 受到来自面朝方向的法术攻击伤害大幅降低
        eEFFECT_ALL_HARM_FACE_DOWN        = 192,// 受到来自面朝方向的所有攻击伤害大幅降低
        eEFFECT_INSERT_ATTACK_ACTION      = 193,// 已经解锁某技能,立即释放(无视大招)
        eEFFECT_EXTEND_BUFF_TICK          = 194,// 改变指定类型的buff效果执行生效时持续时间(根据效果执行的isBuff字段)
        eEFFECT_DEBUF_MOVE_EX             = 195,// 免疫敌方阵营的移动执行
        eEFFECT_BYPASS_AVOID              = 196,// 无视目标闪避抵抗
        eEFFECT_ATTACK_GAIN_ENERGY_ADD    = 197,// 效果拥有者攻击获取能量增加
        eEFFECT_ATTR_CONVERT_ADD          = 198,// 根据一级属性影响二级属性
        eEFFECT_INSERT_ATTACK_ACTION_PLUS = 199,// 已经解锁某技能,立即释放(无视大招且判断切换普攻)

		eEFFECT_ONLY_FOR_DAZE             = 200,// 用于眩晕等不改变属性的效果，直接执行对应的特效
        eEFFECT_ONLY_FOR_DAZE_PLUS        = 201,// 用于眩晕等不改变属性的效果，直接执行对应的特效
        eEFFECT_MISS_EXCUTE               = 202,// 未命中时触发
        eEFFECT_ESCAPE_PHY_EXCUTE         = 203,// 物理闪避时触发
        eEFFECT_ESCAPE_MAG_EXCUTE         = 204,// 法术抵抗时触发
		eEFFECT_GOBACK_SAVEPOSITION		  = 205,// 回到技能施放时的初始位置的效果
        eEFFECT_COUNT_MARK_LEVEL_PLUS     = 206,// 统计标记层数(排除死亡角色)
        eEFFECT_PROP_HURT_ADD             = 207,// 属性伤害减免
        eEFFECT_BEHIT_GAIN_ENERGY_ADD     = 208,// 效果拥有者受击获取能量增加
        eEFFECT_EXCUTE_EXEC               = 209,// 立即触发另一个执行
        eEFFECT_HEAL_BY_EFFECT_COUNT_PLUS = 210,// 根据场上同一施法者释放的指定效果ID的统计值给目标加血,攻击力只取第一次加血时的值
        eEFFECT_RESIST_N_REBOUND          = 211,// 完全抵挡下一次攻击并反伤（反伤数值/百分比需要可配置，若填0则不反伤），该效果触发后移除


		eEFFECT_PROMOTE_SKILL_LEVEL		  = 212, // 提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）
        eEFFECT_IGNORE_NO_DIE             = 213, // 使指定阵营的指定角色的某个技能在造成伤害（伤害执行）时无视敌人的不死
		eEFFECT_ATTACKLIST_ADD_ONE		  = 214, // 目标的攻击序列中非普攻技能之前多加一个技能。
		eEFFECT_CREATE_HURT_ADD_VALUE	  = 215, // 若对目标造成的伤害低于X点/X百分比目标血量，则附加理论计算值（无视防御）一定百分比的真实伤害。

        eEFFECT_IGNORE_SILENCE            = 216,// 不受沉默效果影响
        eEFFECT_ADD_HURT_ON_DIST          = 217,// 根据距离增加伤害
        eEFFECT_ADD_HURT_ON_SAME_TARGET   = 218,// 对同一敌人连续攻击将造成更多伤害
        eEFFECT_EXCUTE_ON_ATTR            = 219,// 目标属性达到条件触发执行
        eEFFECT_DUEL                      = 220,// 决斗
        eEFFECT_CRIT_ON_ATTR              = 221,// 目标属性达到条件时必定暴击
        eEFFECT_ADD_ATTR_ON_ATTR          = 222,// 根据某一项属性值A赋予另一项属性值B
        eEFFECT_BE_TAUNTED_PRIME          = 223,// 高优先级嘲讽

        eEFFECT_EXTEND_EXIST_BUFF         = 224,// 延长已有buff持续时间
        eEFFECT_DEBUF_CLEAR_BODY_EXEC     = 225,// 清除并免疫身体执行
        eEFFECT_SHIELD_BYCOUNT_ALL_HURT   = 226,// 吸收伤害次数的所有类型攻击护盾 物理 法术 纯粹
        eEFFECT_EXTEND_HIT_FLYNDOWN       = 227,// 延长 击飞 击倒 状态持续时间
        eEFFECT_CAST_IFHAS_SKILL          = 228,// 已经解锁某技能,立即触发执行
        eEFFECT_NO_HIT_FORCE_DOWN         = 229,// 免疫强制击倒目标(受创类型7)
        eEFFECT_CAST_ON_HURT_RESULT       = 230,// 处于某种受创类型时连续执行
        eEFFECT_SKIP_SKILL_SEQ            = 231,// 跳过攻击序列中指定序号的技能
		eEFFECT_SPECIFY_SEX_CONDITION     = 232,// 我方（或者敌方）存活的角色中，性别A与性别B的人数满足指定关系
		eEFFECT_CHANGE_SEX_EFFECT		  = 233,// 在一段时间内改变性别
		eEFFECT_ACCORDANCE_SEX_EXEC		  = 234,// 根据指定性别调用不同执行（主要是动画执行）。
        eEFFECT_BAMA_CRIT_NCAST           = 235,// 持续时间内，如果指定类型的（物理或法术或治疗）伤害击中敌人时，必定暴击
		eEFFECT_PRIMARYATTRIBUTE_CONDITION = 236,// 若触发时主属性A与其他两个主属性BC的关系满足条件，则触发执行id
		eEFFECT_JUDGE_SUMMON_COUNT		   = 237,// 拥有效果时（瞬时），判断自己的召唤物数量。
		eEFFECT_CURATT_MAXATT_CONDITION	   = 238,// 瞬间释放（多为伤害执行中的击中执行），目标当前属性与当前最大属性的比值与填写的百分比作比较，决定触发执行。
        eEFFECT_SLIP_NCAST                = 239,// 开始释放技能(释放条件为0且正常释放)时有概率滑倒并触发执行
        eEFFECT_SKIP_SKILLCD_NCAST        = 240,// 跳过技能公共冷却时间并释放一个技能
        eEFFECT_SACRIFICE_HURT_RECV       = 241,// 牺牲 伤害承担
        eEFFECT_SACRIFICE_HURT_SEND       = 242,// 牺牲 伤害转移
        eEFFECT_ADD_HURT                  = 243,// 增加目标造成的所有伤害(不含加血)
        eEFFECT_REVIVE                    = 244,// 新复活
        eEFFECT_HURT_BY_DEADFRIEND        = 245,// 根据己方已阵亡角色数量对目标造成伤害
        eEFFECT_KO_BY_COUNT               = 246,// 目标在受到指定次数的直接攻击后死亡
        eEFFECT_RESIST_FATAL_HURT_NCAST   = 247,// 抵挡一次致命伤害并触发一个执行
        eEFFECT_FORCE_ATTACK_SELF         = 248,// 强制普攻自己N次
        eEFFECT_CAST_BY_TEAM_HP_RATIO     = 249,// 团队血量百分比（指定阵营）达到条件触发执行
        eEFFECT_CAST_BY_TEAM_DEAD_CNT     = 250,// 场上死亡人数（指定阵营）达到条件触发执行
        eEFFECT_CHANGE_ATTACK_SPEED       = 251,// 改变攻击速度
        eEFFECT_BALANCE_ATTR              = 252,// 拉平目标属性(取属性高的值)
        eEFFECT_TAUNT_EXTRA               = 253,// 新新魅惑(目标只能受到真正敌对阵营的伤害，自己会攻击队友，队友不会攻击被控制的单位)

		eEFFECT_DUR_HIT_CRIT_EFFECT		  = 254, // 持续时间内，如果物理法术伤害击中任何敌人且暴击，则触发一个执行（有触发间隔）

        eEFFECT_ALLOW_NEGTIVE_HEAL        = 255,// 允许受到负治疗值
        eEFFECT_CAST_WHEN_FRIEND_SUMMON   = 256,// 每当友军制造1个幻象就触发1个执行
        eEFFECT_CAST_WHEN_FRIEND_DIE      = 257,// 友军发生减员时触发执行
        eEFFECT_CAST_BY_SPEC_LABEL_HURT   = 258,// 受到特殊标签角色伤害时触发
        eEFFECT_HURT_DOWN_FROM_MARK       = 259,// 受到指定标记原始伤害减免
        eEFFECT_CAST_BY_SPEC_EFFECT       = 260,// 受指定效果时触发执行
        eEFFECT_CANNOT_FIND_TARGET        = 261,// 不能攻击指定阵营的指定角色
        eEFFECT_CRIT_IF_HIT               = 262,// 若伤害命中则必定暴击
        eEFFECT_CANNOT_BE_FOUND_BY_SIDE   = 263,// 不能被指定阵营选中
        eEFFECT_CAST_BY_SPEC_STATE        = 264,// 处于指定状态时触发执行
        eEFFECT_SHIELD_TRANSFER_HURT      = 265,// 屏障球
        eEFFECT_STATE_VERTIGO             = 266,// 眩晕（可改变状态）
		eEFFECT_CREATE_HURT_ADD_VALUE_NEW = 267, // 新增效果：若对目标造成的伤害[是否只取来自施法者某执行id（伤害执行&效果执行）的伤害。]低于X点/X百分比目标血量，则附加理论计算值（无视防御）一定百分比的真实伤害。
        eEFFECT_DIRECT_KO                 = 268,// 直接致死，无视buff，不触发死亡执行eEFFECT_CAUSE_HURT_BY_LOSE_HP
        eEFFECT_CAUSE_HURT_BY_CURR_HP     = 269,// 根据目标当前血量百分比造成伤害
		eEFFECT_CONSUME_HP_ADD_CRUITHURT  = 270, // 消耗自身当前生命值，造成额外真实伤害
		eEFFECT_ENDURE_HURT_BY_CONDITION  = 271, // 承受伤害小于一定阈值伤害不生效，且触发执行id
		eEFFECT_FATAL_HURT_DO_EFFECT      = 272,		// 受到致命伤害（本次伤害会造成死亡）时，保留1滴血量不死，触发一个执行
		eEFFECT_ABSORB_ALLTYPE_HURT_SHIELD = 273, // 吸收所有伤害类型的护盾，护盾的数值根据自身的某项属性
        eEFFECT_COPY_CREATURE_ANIMATION   = 274,// 暂时变身为目标动画
        eEFFECT_IGNORE_FIGHT_PARAM_LIMIT  = 275,// 拥有该效果的角色在攻击时计算暴击，闪避，抵抗，吸血，无视公式上下限
        eEFFECT_CAST_BY_TEAM_LOSE_HP      = 276,// 友军每损失血量达到条件触发执行
        eEFFECT_CHANGE_ACCU_MAX_TICK      = 277,// 改变蓄力大招的最大蓄力时间
        eEFFECT_CHANGE_HPADD1_BY_MARK     = 278,// 根据标记层数改变吸血等级
        eEFFECT_CAST_BY_STATE             = 279,// 处于指定状态时触发执行
        eEFFECT_FOCUS_TARGET              = 280,// 集火
        eEFFECT_FREEZE_IN_EFFECT_ACTION   = 281,// 眩晕石化期间暂停并触发执行
        // 11.0
        // 282 受到某种类型标签的角色攻击时必定受到暴击
        // 283 反伤 以未计算减伤前的伤害值作为反伤初始值
        // 284 根据飞行道具飞行距离触发执行
        // 285 根据觉醒等级触发执行
        // 285 判断觉醒等级触发执行
        // 286 恢复上一次直接攻击造成的一定百分比的伤害(不包括效果执行的持续伤害)
        // 287 每受到X次攻击，第X次会触发执行
        // 288 统计目标受到的伤害
        // 289 释放统计的伤害
        // 290 攻击背后必定暴击
        // 291 受到指定ID的角色伤害时，附加额外伤害到初始伤害
        // 292 使指定效果失效
        // 293 无法转身，且无法攻击背后的敌人
        // 294 不能使用含有移动执行的技能
        // 295 强制抓取，目标无法行动，但可受到移动执行影响
        // 296 使死亡触发技能无效
        // 297 不能被指定阵营的单体目标类型选中
        // 298 免疫295
        // 299 提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）
        // 300 游戏音量渐变
        // 301 清空288统计的伤害

        // 12.0
        // 302 角色处于丢失状态时触发执行
        // 303 当角色产生位移（X轴、Y轴）时[移动执行、击飞、击倒]触发执行
        // 304 被认为是幻象或英雄

        // 13.0
        // 305 不能被指定阵营的技能施法者选中
        // 306 免疫指定效果执行的效果ID
        // 307 保护指定效果执行的效果ID
		// 308 根据指定技能的等级及效果数值区间，决定是否触发效果执行
        // 309 释放指定类型的技能时触发执行
        // 310 将一项二级属性转化为另一项二级属性

        // 14.0
        // 311 判断目标身上是否有指定执行ID
        // 400 改变目标状态的效果执行，只有在成功改变目标状态（即眩晕石化）后，才会显示绑定的UI特效

        // 15.0
        // 312 同时吸收物理和法术伤害的护盾

		// 25.0
		eEFFECT_IGNORE_TARGET_BAREPEL = 501, // 效果拥有者攻击时无视目标的物理护甲
		eEFFECT_IGNORE_TARGET_MAREPEL = 502, // 效果拥有者攻击时无视目标的法术抗性
		eEFFECT_HURT_ADD_EXTRA        = 503, // 攻击者造成的伤害加成
		eEFFECT_HURT_SUB_EXTRA		  = 504, // 效果拥有者被攻击时伤害减少
        
        // 28.5
		eEFFECT_HP_LOCK		          = 505, // 生命值锁定
        eEFFECT_HP_LOST               = 506, // 生命流血
        eEFFECT_NEW_HURT_LIMIT        = 507, // 伤害上限阈值,单次受到伤害不会超过自身最大生命值的X% 新154，包括真实伤害

        //28.7
        eEFFECT_HEAL_EXT           = 508, //治疗量溢出时触发
        eEFFECT_HEAL_SHIELD        = 509, //治疗量溢出护盾

        eEFFECT_HURT_CHANGE_BY_EFFECT   = 510, //身上存在指定标签id士，受到伤害增加

        eEFFECT_SPURTING_HURT = 511,    //灼热，溅射伤害

        // OP2 海战用
        // 10001 白刃战船只护盾(吸收所有伤害,优先级高-在OP1的护盾前使用)

	};

    class AIInnerEffect;

	class AIContinuousEffectCaster : public AINode
	{
	public:
		virtual ~AIContinuousEffectCaster();
		AIContinuousEffectCaster(AIContext& aiContext, int effID, int skID, int skLv, CharacterData* origin, AIProjectile* projectile, CharacterData* target, int theMarkID);

		AIContext& aiContext;
		const int effectID;
        const int skillID;
		const int skillLevel;
        const int markID;
		
		EffectExec* staticData;
		bool firstRound;

		CharacterData* origin;
		AIProjectile* projectile;
		CharacterData* target;

		bool stopFurtherEffect;
        bool isInvalidated;
        int invalidateTick;
        bool shouldDeleteSelf = false;

        std::unique_ptr<int> originFirstHealMAPower; // 210效果第一次治疗时施法者的攻击力

        std::unordered_map<int, int> action_uieffect_affectedCharacters;//only to provent multiple execution
        std::unordered_map<int, bool> stateAniAffectedChars; //改变目标状态效果影响的目标
        std::unordered_map<int, int> stateAniAffectedResult; //改变目标状态效果是否成功 0-不成功 1-切换到EffectState成功 2-延长EffectState成功
        bool isCharacterAffectedUIEff(int characterMyID);
        void addUIEffAffectedCharacter(int characterMyID);

        //AffectedCharactersMap m_affectedCharactersMap;
        std::map<int/*innerExecID*/, std::set<int/*characterMyID*/, std::less<int>/*由小到大*/ > > innerEffs;
        std::vector<AIInnerEffect *> allEffs;
        //bool removeBaseAIEffect(int characterId, const AIEffectExecResultPtr& pEffect);
        void removeInnerEffRecord(int characterMyID, int innerExecID);
        //void scheduleInnerEff(int scheduleTick, AIEffectExecResultWeakPtr innerEff);
        void scheduleInnerEff(int scheduleTick, AIInnerEffect *pInnerEff);
        void unscheduleInnerEff(int innerExecUID);
        void appendScheduleInnerEff(int scheduleTick, AIInnerEffect *pInnerEff);
        bool hasInnerEffect(int execID);
        void extendInnerEffectUITick();
        void castInnerEffectByIdx(int idx, CharacterData *c);
        int getStateAnimationResult(CharacterData *c);

	public:
        void Excute(AIContext& aiContext);
        
		static int check_exert_Special_Effect(AIContext& aiContext, CharacterData& tmpTarget, int etype, int param);
		static void zero_Special_Effect(AIContext& aiContext, CharacterData& tmpTarget, int etype);
        static void dispelInnerEffect(AIContext& aiContext, CharacterData *c, int parentEffectCasterId, int innerEffectId);
        static void cover_Special_Effect(AIContext& aiContext, CharacterData& tmpTarget, int etype);
		static void change_Special_Effect(AIContext& aiContext, CharacterData& tmpTarget, int etype, int param);
        static void excertOnEscapeEffects(AIContext& aiContext, CharacterData& theTarget);
        static void startEffectCD(CharacterData& target, InnerEffectExec* pExec);
        static int getEffectValue(CharacterData* tmpTarget, int execID, int param = 0);
        static int getEffectValueWithParam(CharacterData* tmpTarget, int execID, int param);
        static void changeAttrWithLoseHp(AIContext& aiContext, CharacterData* tmpTarget);
        static void executeOnHealTriggerEffect(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget);
        static void killFollowSkillEffect(AIContext &aiContext, CharacterData *t);
        static bool isCasterGroupCovered(CharacterData *t, int execId);
        static int checkCoverGroupCasters(AIContext &aiContext, CharacterData *t, int execId, bool isCleanInnerEffect);

		void stopExecute();
        void removeAfterTick(int tick);
        void castOnTarget(CharacterData* tmpTarget);
        void cleanupInnerEffect();
        bool isFollowSkillTick();

        //28.7
        static void checkHealTriggerEffect(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget,int addValue);  //治疗量溢出 2023_1_11
        static void checkShieldEffectAdd(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget, int addValue);  //治疗量溢出产生的护盾 2023_1_11
        
		
	private:
		bool isHurtEffect(int type);
		//AIContinuousEffectCaster(const AIContinuousEffectCaster&);
		//AIContinuousEffectCaster(AIContinuousEffectCaster&&);
		//AIContinuousEffectCaster& operator=(const AIContinuousEffectCaster&);

        bool castInnerExec(InnerEffectExec& innerExec, CharacterData* target);
        void executeTriggerEffect(CharacterData* target, int triggerEffectID);

    private:
        std::unordered_map<int/*tick*/, std::map<int/*innerExecUID*/, AIInnerEffect *> > _scheduleMap;
        std::vector< std::tuple<int, int, AIInnerEffect*> > _appendScheduleEff; // <tick, innerUID, InnerEff*>

        std::map<int, std::map<int, AIInnerEffect *>> _schduleUIEffectMap; // 12.0 <tick <innerEffectUid, innerEffect>>
	};

	class AIInnerEffect
	{
	public:
        static AIInnerEffect *create(AIContinuousEffectCaster* parent, CharacterData* target, InnerEffectExec* exec);

        AIInnerEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);

        virtual ~AIInnerEffect();

        virtual void onEraseEffect();
        virtual void castUIEffect(int ticks);
        virtual void castBodyEffect(int ticks);
        virtual void onScheduleTick() {}
        //virtual void onParentCleanup() {}

        void removeUIBodyEffects();
        void removeUIEffect();
        void removeBodyEffect();

        int getUID() const {
            return _uid;
        }
		virtual bool onEnter(){ return false; } //return enter success
        virtual void onExit(){}
        virtual void checkCleanSameTypeExec();
		int getChangeValue(){ return _changeValue; }
		void setChangeValue(int value){ _changeValue = value; }
        int getUndividedChangeValue() const { return _undividedChangeValue; }
        void setUndividedChangeValue(int value) { _undividedChangeValue = value; }
        CharacterData* getTargetSafe();
        CharacterData* getOriginSafe();
        bool isDebuffed();
        bool isIgnoreHitLabel() const { return _ignoreHitLabel; }
        void setIsIgnoreHitLabel(bool theIsIgnore) { _ignoreHitLabel = theIsIgnore; }
        virtual int getHitLabelDisplayValue() const { return _changeValue; }

        virtual int caculateChangeValue(bool ignoreValuebase = false);
        virtual int caculatePassChangeValue(int realChangeValue = 0, bool ignoreValuebase = false);

        static int getBoundedChangeValue(int changeValue, InnerEffectExec* pExec, int skillLevel);

        virtual int calculateUndividedChangeValue();
        virtual int getBoundedUndividedChangeValue(int changeValue);

        int getOriginMyId() { return _originMyID; }
        int getTargetMyId() { return _targetMyID; }
        
        bool isErased() const { return _isErased; }
        bool isTempInvalid() const { return _isTempInvalid; }
        void setTempInvalid(bool tempInvalid);

		AIContext& aiContext;
		CharacterData* origin = nullptr;
		AIProjectile* projectile = nullptr;
		CharacterData* target = nullptr;

		InnerEffectExec* innerExec = nullptr;
		EffectExec* effectExec = nullptr;
        int skillID = 0;
		int skillLevel = 0;
        bool isValid = false;

		AIContinuousEffectCaster* parentCaster = nullptr;

        void setInnerData(const std::string &datakey, int dataval);
        bool hasInnerData(const std::string &datakey);
        int getInnerData(const std::string &datakey);

	protected:
        int _uid = 0;
		int _changeValue = 0;
        int _originMyID = 0;
        int _targetMyID = 0;
        bool _ignoreHitLabel = false;

        int _undividedChangeValue = 0;
        int _originChangeValue = 0; //没有计算百分比后的数值

        bool _isErased = false;
        bool _isTempInvalid = false;

        std::map<std::string, int> _innerData;
	};
    
    class AIAttributeEffectResult {
    public:
        enum ChangeType {
            ATTR_CHANGE_RAW = 0,
            ATTR_CHANGE_PERCENT = 1,
            ATTR_SWAP = 2
        };
        bool isValid = false;
        int changeType = ATTR_CHANGE_RAW;
        int innerEffectUid = 0;
        int attrIndex = 0;
        int attrValueOld = 0;
        int attrValueNew = 0;
        int changePercent = 0;
        CharacterData *target = nullptr;
        
        AIAttributeEffectResult(int theInnerEffectUid, int theAttrIndex);
        
        void applySwapResult(CharacterData *theTarget, int theSwapperAttrValue);
        void deapply();
        void reapply();
    };

    class AIAttributeEffect : public AIInnerEffect
	{
	public:
        AIAttributeEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AIAttributeEffect();
		virtual int caculateChangeValue();
        virtual int getHitLabelDisplayValue() const override { return _attrRealChangeValue; }
		int getAttrRealChangeValue() { return _attrRealChangeValue; }
        void setAttrRealChangeValue(int theValue) { _attrRealChangeValue = theValue; }
        int getAttrRealChangeValueForOrigin() { return _attrRealChangeValueForOrigin; }
        void setAttrRealChangeValueForOrigin(int theValue) { _attrRealChangeValueForOrigin = theValue; }

		virtual bool onEnter();
		virtual void onExit();
        virtual void checkCleanSameTypeExec();

        int applyOnTarget(CharacterData *theTarget, int attributeIndex, int theChangeValue);
        void deapplyOnTarget(CharacterData *theTarget, int attributeIndex, int theChangeValue, int theRealChangeValue);
    private:
        int _attrRealChangeValue = 0;
        int _attrRealChangeValueForOrigin = 0;
        int _attrIndex = 0;
	};

    class LoseHPAddAttrInfo
    {
    public:
        // {baseAddVal,haveAddVal,minAddVal,maxAddVal,changeType}
        int baseAddVal = 0;
        int haveAddVal = 0;
        int minAddVal = 0;
        int maxAddVal = 0;
        int changeType = 0;
    };

    class AILoseHPAddEffect : public AIInnerEffect
	{
	public:
        AILoseHPAddEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AILoseHPAddEffect();

		virtual bool onEnter();
		virtual void onExit();
        virtual void checkCleanSameTypeExec();
	};

    class AIStateEffect : public AIInnerEffect
	{
	public:
        AIStateEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AIStateEffect();
		bool onEnter() override;
		void onExit() override;
        void checkCleanSameTypeExec() override;
	};

    class AIExchangePowerEffect : public AIInnerEffect
    {
    public:
        AIExchangePowerEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIExchangePowerEffect();

        virtual bool onEnter();
        virtual void onExit();
        virtual void castUIEffect(int ticks);

    private:
        CharacterData* _exchangeTarget = nullptr;
        int _exchangeTargetHeroID = 0;
    };

    class AIKnockoutEffect : public AIInnerEffect
    {
    public:
        AIKnockoutEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIKnockoutEffect();

        virtual bool onEnter();
        virtual void onExit();
    };

    class AIFatalHurtTransferEffect : public AIInnerEffect
    {
    public:
        static AIFatalHurtTransferEffect *getActivatedFatalTransEff(CharacterData *transfer, long worldTick, int sourceMyID);
        static AIFatalHurtTransferEffect *getValidFatalTransferEffect(CharacterData *transfer, long worldTick, int sourceMyID);
        static void activateFatalTransferEffect(CharacterData *transfer, long worldTick, int sourceMyID, AIFatalHurtTransferEffect *pEffect);
        static AIFatalHurtTransferEffect *getUnactivatedFatalTransEff(CharacterData *transfer, long worldTick, int sourceMyID);
    public:
        AIFatalHurtTransferEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIFatalHurtTransferEffect();
        virtual bool onEnter();
        virtual void onExit();
        virtual void castUIEffect(int ticks);

        void onTrigger();

        bool isTriggered = false;
        long triggerWorldTick = -1;
        int triggerCharacterUUID = -1;
    };
    //typedef std::shared_ptr<AIFatalHurtTransferEffect> AIFatalHurtTransferEffectPtr;

    class AILinkenSphereEffect : public AIInnerEffect {
    public:
        //typedef std::shared_ptr<AILinkenSphereEffect> SharedPtr;
        // @return 是否触发林肯法球效果成功
        static bool activateLinkenSphereEffect(CharacterData *source, CharacterData *target, bool isSimulate);
        AILinkenSphereEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AILinkenSphereEffect();
        virtual bool onEnter();
        virtual void onExit();
        virtual void onScheduleTick();
        void onActivate();
        bool canActivate();

        int activateCD = 0; // 触发间隔
        int activateExecID = 0; // 触发执行
        int nextActivateTick = 0; // 触发CD结束后的tick
    };

    // 薄葬
    class AIFlimsyBuryingEffect : public AIInnerEffect {
    public:
        typedef std::shared_ptr<AIFlimsyBuryingEffect> SharedPtr;
        // @return 触发薄葬效果后的伤害值
        static int activateFilmsyBuringEffect(CharacterData *target, int HPDelta, bool isSimulate);
        AIFlimsyBuryingEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIFlimsyBuryingEffect();
        virtual bool onEnter();
        virtual void onExit();
		void onScheduleTick() override;

       bool canActivate(int HPDelta);
	   void onActivate();

        int limitHP = 0; // 触发临界值
        int activeTicks = 0; // 触发后持续时间
        bool isActivated = false;
        int endParentTick = 0; // 触发后结束tick
    };

    // 伤害连接
    class AIShareHurtEffect : public AIInnerEffect {
    public:
        static bool isShareHurtValid(AIInnerEffect *eff);
        static void activateShareHurt(AIShareHurtEffect *pEff);
        static AIShareHurtEffect *getFirstValidShareHurt(CharacterData *character);
        AIShareHurtEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIShareHurtEffect();
        virtual bool onEnter();
        virtual void onExit();

        void onActivate();

        int lastActivateWorldTick = 0;
    };

    class AICritHitNoFailEffect : public AIInnerEffect {
    public:
        static bool checkActivateCritHitNoFailEffect(CharacterData *target);
        static void activateCritHitNoFailEffect(CharacterData *target, bool isCritHit);
        AICritHitNoFailEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AICritHitNoFailEffect();
        virtual bool onEnter();
        virtual void onExit();
        
        bool isValid();
        bool canActivate();
        void onActivate();
        void onActivateNormalHit();

        void onHit();

        int normalHitCount = 0;
    };

    class AIPickRangeSkillEffect : public AIInnerEffect {
    public:
        static int getRangeSkillIndex(CharacterData *pTarget);
        AIPickRangeSkillEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIPickRangeSkillEffect();
        virtual bool onEnter();
        virtual void onExit();

        bool isCharacterInRange(CharacterData *character);

        int rangeLower = 0;
        int rangeUpper = 0;
    };

    class AILimitMoveRangeEffect : public AIInnerEffect {
    public:
        static bool isTargetOutofLimitMoveEffectRange(CharacterData *pTarget, int posX);
        static int getLimitMoveEffectLower(CharacterData *pTarget);
        static int getLimitMoveEffectUpper(CharacterData *pTarget);
        AILimitMoveRangeEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AILimitMoveRangeEffect();
        virtual bool onEnter();
        virtual void onExit();

        int rangeLower = 0;
        int rangeUpper = 0;
    };

    class AILazyEnterBattleEffect : public AIInnerEffect {
    public:
        AILazyEnterBattleEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AILazyEnterBattleEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIThresholdExecEffect : public AIInnerEffect {
    public:
        AIThresholdExecEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIThresholdExecEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AICountMarkLevelEffect : public AIInnerEffect {
    public:
        AICountMarkLevelEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AICountMarkLevelEffect();
        virtual bool onEnter();
        virtual void onExit();
        int getMarkCount();
    };

    class AIHealByEffCountEffect : public AIInnerEffect {
    public:
        AIHealByEffCountEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIHealByEffCountEffect();
        virtual bool onEnter();
        virtual void onExit();

    };


    class AIEscMissExcuteEffect : public AIInnerEffect {
    public:
        static void checkActivateEscMissExcute(CharacterData *pTarget, int effectID);
        AIEscMissExcuteEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIEscMissExcuteEffect();
        virtual bool onEnter();
        virtual void onExit();

        bool isActivated = false;
        bool isInActivating = false;
        int excuteID = 0;
        int excuteCD = 0;//触发后冷却CD
        int resetParentTick = 0;
    };

    // 效果值未除100，使用时根据精度需要转换
    class AIFloatStateEffect : public AIInnerEffect {
    public:
        AIFloatStateEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIFloatStateEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    //class AIAttrConvertAddEffect : public AIInnerEffect {
    //public:
    //    AIAttrConvertAddEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
    //    virtual ~AIAttrConvertAddEffect();
    //    virtual bool onEnter();
    //    virtual void onExit();
    //};

	class AISavePositionAndResetEffect : public AIInnerEffect
	{
	public:
		AISavePositionAndResetEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AISavePositionAndResetEffect();

		virtual bool onEnter();
		virtual void onExit();

	protected:
		int pos_x;
		int pos_y;
	};


    // clair_wang 新增效果：提升指定技能的等级 提升某个技能的等级（提升的等级数值随本身技能等级成长）	
    class AIPromoteAppointSkillLevelEffect : public AIInnerEffect
    {
    public:
        AIPromoteAppointSkillLevelEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIPromoteAppointSkillLevelEffect();
        
        virtual bool onEnter();
        virtual void onExit();
    private:
        int _realChangeValue = 0;
    };

	// clair_wang 新增效果：使指定阵营的指定角色的某个技能在造成伤害（伤害执行）时无视敌人的薄葬。
	class AIAttackIgnoreNoDieEffect : public AIInnerEffect
	{
	public:
        static bool checkExcute(AIContext &aiContext, CharacterData *target);
        AIAttackIgnoreNoDieEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIAttackIgnoreNoDieEffect();

		virtual bool onEnter();
		virtual void onExit();

		bool CanBeActivate();

	protected:

		int mCamp = 0;// 阵营
		int mCharacterID = 0; // 角色
		int mSkillID = 0; // 技能ID
	};

	// clair_wang 新增效果：目标的攻击序列中非普攻技能之前多加一个技能。
	class AIAttackListAddSomeOneEffect : public AIInnerEffect
	{
	public:
        static std::vector<int> *getAlternateAttackSeq(CharacterData *target, bool useSeq2);
		AIAttackListAddSomeOneEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIAttackListAddSomeOneEffect();

		virtual bool onEnter();
		virtual void onExit();

	protected:
        std::vector<int> _attackSeq;
        std::vector<int> _attackSeq2;

        std::vector<int> _newMapOldIndex1;
        std::vector<int> _newMapOldIndex2;

        std::vector<int> _oldMapNewIndex1;
        std::vector<int> _oldMapNewIndex2;
	};

	class AICreateHurtAddPureValueEffect : public AIInnerEffect
	{
	public:
		AICreateHurtAddPureValueEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AICreateHurtAddPureValueEffect();

		virtual bool onEnter();
		virtual void onExit();

	protected:
		float mAddHurtPercent = 0;// 附加真实伤害百分比
		int limitUpper = 0; // 上限
		int limitLower = 0; // 下限
	};


    class AIAddHurtOnSameTargetEffect : public AIInnerEffect {
    public:
        static int checkExcute(CharacterData *pSource, CharacterData *pTarget, int hpdelta, int hurtType);
        AIAddHurtOnSameTargetEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIAddHurtOnSameTargetEffect();
        virtual bool onEnter();
        virtual void onExit();
        
        void onHurtTarget(CharacterData *aTarget);

        CharacterData *lastTarget = nullptr;
        int sameCount = 0;
    };

    class AIExcuteOnAttrChangeEffect : public AIInnerEffect {
    public:
        static void checkExcute(CharacterData *source, int attr);
        AIExcuteOnAttrChangeEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AIExcuteOnAttrChangeEffect();
        virtual bool onEnter();
        virtual void onExit();
        void excute(int attr);

        int nextActiveParentTick = 0;
    };

    class AICritOnAttrEffect : public AIInnerEffect {
    public:
        static bool checkExcute(CharacterData *source, CharacterData *target);
        AICritOnAttrEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AICritOnAttrEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIDebufClearBodyExecEffect : public AIInnerEffect {
    public:
        static bool checkExcute(CharacterData *pTarget, int bodyExecID);
        AIDebufClearBodyExecEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AIDebufClearBodyExecEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIExcuteOnHurtResultEffect : public AIInnerEffect {
    public:
        static void checkExcute(CharacterData *target);
        AIExcuteOnHurtResultEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AIExcuteOnHurtResultEffect();
        virtual bool onEnter();
        virtual void onExit();
        virtual void onScheduleTick();
        void checkActivate();
        void onStart();
        void onEnd();

        bool isActionMatch();

        int excuteInterval = 0;
        int targetActionType = 0;
        int lastExcuteParentTick = -1;
        bool isStartInterval = false;
    };

    class AISkipSkillSeqEffect : public AIInnerEffect {
    public:
        static int checkExcute(CharacterData *target, std::vector<int> &seq, int currentIndex);
        AISkipSkillSeqEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AISkipSkillSeqEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIBAMACritCastEffect : public AIInnerEffect {
    public:
  //      static void checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target);
        AIBAMACritCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AIBAMACritCastEffect();
        virtual bool onEnter();
        virtual void onExit();

		bool isMatchCondition(int hurtType);

	private:
		std::vector<int> vCastTypes; // 指定类型伤害必定暴击
   //     int castInterval = 0; // 同一目标的触发间隔
   //     int castID = 0;
  //      std::map<int, int> castTargetRecord; // <targetMyID, parentEffectCasterLocalTick>
    };

    class AISlipCastEffect : public AIInnerEffect {
    public:
        static AISlipCastEffect *checkExcute(AIContext &aiContext, CharacterData *target);
        AISlipCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AISlipCastEffect();
        virtual bool onEnter();
        virtual void onExit();

        int castProb = 0;
        int ignoreDebuff = 0;
        int castInterval = 0;
        int slipTickLen = 0;
        int castID = 0;
        int lastCastParentLocalTick = -1;
    };

    class AISkipCDCastEffect : public AIInnerEffect {
    public:
        static int checkExcute(AIContext &aiContext, CharacterData *target);
        AISkipCDCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AISkipCDCastEffect();
        virtual bool onEnter();
        virtual void onExit();

        int castSkillID = 0;
    };

    class HurtCalcParam;

    class AISacrificeHurtRecv : public AIInnerEffect {
    public:
        static bool checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &hurtParam, int theSendOriginMyID, int theSendSacrificeID);
        AISacrificeHurtRecv(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AISacrificeHurtRecv();
        virtual bool onEnter();
        virtual void onExit();

        bool canRecvHurt(int theSendOriginMyID, int theSendSacrificeID, int hurtType, int hurtProperty);

        int sacrificeID = 0;
        //int supportHurtType; // 支持的伤害类型 0纯粹 1物理 2法术
        std::map<int, bool> supportHurtTypes;
        std::map<int, bool> supportHurtPropTypes; // 支持的伤害属性
    };

    class AISacrificeHurtSend : public AIInnerEffect {
    public:
        static int checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target, HurtCalcParam &hurtParam); // @return remain hurt
        AISacrificeHurtSend(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AISacrificeHurtSend();
        virtual bool onEnter();
        virtual void onExit();

        //bool canSendHurt(int hurtType, int hurtProperty);
        int getSendHurtDelta(HurtCalcParam &hurtParam);

        int sacrificeID = 0;
        //int supportHurtType; // 支持的伤害类型 0纯粹 1物理 2法术
        //std::map<int, bool> supportHurtPropTypes; // 支持的伤害属性
    };

    class AIHpLostEffect : public AIInnerEffect {
    public:
        static int checkExcute(AIContext& aiContext, CharacterData* source, CharacterData* target, HurtCalcParam& hurtParam);
        static void checkLostExcute(AIContext& aiContext, CharacterData* target);
        AIHpLostEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIHpLostEffect();
        virtual bool onEnter();
        bool canTrigger();
    
    private:
        int stepCount = 0;
        int triggerCount = 0;
        std::map<int, std::vector<std::tuple<CharacterData*, CharacterData*, HurtCalcParam>>> m_aiHurtMap; //start tick
    };
		
	// clair_wang 若触发时我方（或者敌方）存活的角色中，性别A与性别B的人数满足指定关系，则触发执行，否则跳过。
	class AISpecifySexConditionResultEffect : public AIInnerEffect {
	public:
		AISpecifySexConditionResultEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AISpecifySexConditionResultEffect();
		virtual bool onEnter();
		virtual void onExit();
	};

	// clair_wang // 在一段时间内改变性别
	class AIChangeSexEffect : public AIInnerEffect{
	public :
		AIChangeSexEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIChangeSexEffect();
		virtual bool onEnter();
		virtual void onExit();
	};

	// clair_wang // 根据指定性别调用不同执行（主要是动画执行）。
	class AIAccordanceSexExectueEffect : public AIInnerEffect{
	public:
		AIAccordanceSexExectueEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIAccordanceSexExectueEffect();
		virtual bool onEnter();
		virtual void onExit();
	};

	// clair_wang // 若触发时主属性A与其他两个主属性BC的关系满足条件，则触发执行id
	class AIPrimayAttributeConditionExectueEffect : public AIInnerEffect{
	public:
		AIPrimayAttributeConditionExectueEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIPrimayAttributeConditionExectueEffect();
		virtual bool onEnter();
		virtual void onExit();
	};


	// clair_wang // 拥有效果时（瞬时），判断自己的召唤物数量。
	class AIExecuteJudgeSummonCountEffect : public AIInnerEffect{
	public:
		AIExecuteJudgeSummonCountEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIExecuteJudgeSummonCountEffect();
		virtual bool onEnter();
		virtual void onExit();
	};

	// clair_wang // 瞬间释放（多为伤害执行中的击中执行），目标当前属性与当前最大属性的比值与填写的百分比作比较，决定触发执行。
	class AIReleaseCurAttrAndMaxAttrContionEffect : public AIInnerEffect{
	public:
		AIReleaseCurAttrAndMaxAttrContionEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIReleaseCurAttrAndMaxAttrContionEffect();
		virtual bool onEnter();
		virtual void onExit();
	};

    class AIHurtByDeadFriendEffect : public AIInnerEffect {
    public:
        AIHurtByDeadFriendEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIHurtByDeadFriendEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIKOByCountEffect : public AIInnerEffect {
    public:
        static void checkExcute(AIContext &aiContext, CharacterData *target, int hurtType, int hurtvalue);
        AIKOByCountEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIKOByCountEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIResistFatalHurtNCastEffect : public AIInnerEffect {
    public:
        static bool canCast(AIContext &aiContext, CharacterData *target, int hpdelta);
        static void checkExcute(AIContext &aiContext, CharacterData *target);
        AIResistFatalHurtNCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIResistFatalHurtNCastEffect();
        virtual bool onEnter();
        virtual void onExit();
    };

    class AIForceAttackSelfEffect : public AIInnerEffect {
    public:
        AIForceAttackSelfEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AIForceAttackSelfEffect();
        virtual bool onEnter();
        virtual void onExit();

        int expectedTick = -1;
    };

    class AICastByTeamHPRatioEffect : public AIInnerEffect {
    public:
        AICastByTeamHPRatioEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AICastByTeamHPRatioEffect();
        virtual bool onEnter();
        virtual void onExit();

        static void checkExcute(AIContext &aiContext, CharacterData *target);
    };

    class AICastByTeamDeadCntEffect : public AIInnerEffect {
    public:
        AICastByTeamDeadCntEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        virtual ~AICastByTeamDeadCntEffect();
        virtual bool onEnter();
        virtual void onExit();

        static void checkExcute(AIContext &aiContext, CharacterData *target);
    };

    class AICastWhenFriendSummonEffect : public AIInnerEffect {
    public:
        AICastWhenFriendSummonEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
        ~AICastWhenFriendSummonEffect();
        bool onEnter();
        void onExit();

        static void checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *summon);
    };
	
	class AIDurHitCritCastEffect : public AIInnerEffect 
	{
	public:
		static void checkExcute(AIContext &aiContext, CharacterData *source, CharacterData *target, int hurtType);
		AIDurHitCritCastEffect(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AIDurHitCritCastEffect();
		virtual bool onEnter();
		virtual void onExit();

		int castInterval = 0; // 触发间隔
		int castID = 0;
		std::map<int, int> castTargetRecord; // <targetMyID, parentEffectCasterLocalTick>
	//	int lastTriggerTick = 0; // 上次触发Tick parentEffectCasterLocalTick
		bool bIsTriggered = false;
		int castHurtType = 0;// 伤害类型
	};

    class AICastWhenFriendDownEffect : public AIInnerEffect {
    public:
        AICastWhenFriendDownEffect(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AICastWhenFriendDownEffect();
        bool onEnter() override;
        void onExit() override;

        static void checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *theFriend);

        int lastTriggerTick = -1;
    };

    class AIEffect258 : public AIInnerEffect {
    public:
        AIEffect258(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect258();
        bool onEnter() override;
        void onExit() override;

        static void checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *attacker, int theCasterId);
    };
    typedef AIEffect258 AICastBySpecLabelHurtEffect;

    class AIEffect259 : public AIInnerEffect {
    public:
        AIEffect259(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect259();
        bool onEnter() override;
        void onExit() override;

        static int checkExecute(AIContext &aiContext, CharacterData *target, int theMarkID, int hpdelta);
    };
    typedef AIEffect259 AIHurtDownFromMarkEffect;

    class AIEffect260 : public AIInnerEffect {
    public:
        AIEffect260(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect260();
        bool onEnter() override;
        void onExit() override;
        void executeActive();
        void executePassive(CharacterData *source, int theEffectId);

        static void checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *source, int theEffectId);

    private:
        int _triggerEffectId = 0;
        int _triggerMode = 0;
        int _triggerRelativeTeam = 0;
        int _triggerCharacterId = 0;
        bool _hasTriggered = false;

        bool isMatchCondition(CharacterData *source, int theEffectId);
        bool isEffectTargetMatchCondition();
    };
    typedef AIEffect260 AICastBySpecEffect;
    
    
    class AIEffect262 : public AIInnerEffect {
    public:
        AIEffect262(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect262();
        bool onEnter() override;
        void onExit() override;
        
        static int checkExecute(CharacterData *target, int skillId);
    };
    typedef AIEffect262 AICritIfHitEffect;

    
    class AIEffect264 : public AIInnerEffect {
    public:
        AIEffect264(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect264();
        bool onEnter() override;
        void onExit() override;
        void executeActive();
        void executePassive(int sourceMyId, int theTargetActionType);

        static void checkExecute(AIContext &aiContext, CharacterData *target, int causerMyId, int theTargetActionType);

    private:
        int _triggerActionType = 0;
        int _triggerMode = 0;
        int _triggerRelativeTeam = 0;
        int _triggerCharacterId = 0;
        bool _hasTriggered = false;
        
        bool isMatchCondition(int sourceMyId, int theActionType);
    };
    typedef AIEffect264 AICastBySpecState;

    class AIEffect265 : public AIInnerEffect {
    public:
        AIEffect265(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect265();
        bool onEnter() override;
        void onExit() override;

        static int checkExecute(AIContext &aiContext, CharacterData *target, int hpdelta, bool isSimulate);

    private:
        int _absorbedHpdelta = 0;
        void absorbHpdelta(int hpdelta);
    };
    typedef AIEffect265 AIShieldTransferHurtEffect;

    class AIEffect266 : public AIInnerEffect {
    public:
        AIEffect266(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect266();
        bool onEnter() override;
        void onExit() override;
        int getRemainEffectActionTick() const {
            return _remainEffectActionTick;
        }
        void setRemainEffectActionTick(int theRemainTick);

        static bool checkExecute(AIContext &aiContext, CharacterData *target);
        static void checkSaveRemainTick(AIContext &aiContext, CharacterData *target);
        
    private:
        int _remainEffectActionTick = 0;
    };
    typedef AIEffect266 AIStateVertigoEffect;


	class AIEffect267 : public AIInnerEffect
	{
	public:
		AIEffect267(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		virtual ~AIEffect267();

		virtual bool onEnter();
		virtual void onExit();

		static int checkExecute(AIContext &aiContext, CharacterData *target, CharacterData *attacker);

		void setBaseInfo();
		int executeActive(CharacterData *attacker);

	protected:
		float mCauseHurt = 0.0f;
		float mAddHurtPercent = 0;// 附加真实伤害百分比
		int limitUpper = 0; // 上限
		int limitLower = 0; // 下限
		bool bIsUsefulFlag = false;//
		int mTriggerExecId = 0;
	private:
		bool _hasTriggered = false;
	};
	typedef AIEffect267 AICreateHurtSpecialAddPureValueEffect;

    class AIRepelEffect : public AIInnerEffect {
    public:
        AIRepelEffect(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIRepelEffect();
        bool onEnter() override;
        void onExit() override;

        static int checkExecute(int repelEffectType, AIContext &aiContext, CharacterData *target, int HPDelta, bool isSimulate);
        static void checkCleanEffectOnCharacterHpChange(AIContext &aiContext, CharacterData *target);
        static void checkCleanOneEffectOnCharacterHpChange(int repelEffectType, AIContext &aiContext, CharacterData *target);
    };

    class AIEffect268 : public AIInnerEffect
    {
    public:
        AIEffect268(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect268();
        bool onEnter() override;
        void onExit() override;
    };
    typedef AIEffect268 AIDirectKoEffect;

    class AIEffect269 : public AIInnerEffect
    {
    public:
        AIEffect269(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect269();
        bool onEnter() override;
        void onExit() override;
        static int getExtraHurt(AIContext &aiContext, CharacterData *theTarget, int theHpDelta);
        static int getHitLabel(AIContext &aiContext, CharacterData *theTarget);
    };
    typedef AIEffect269 AICauseExtraHurtByHp;

    class AIEffect274 : public AIInnerEffect
    {
    public:
        AIEffect274(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect274();
        bool onEnter() override;
        void onExit() override;

    private:
        int _transType = 0;
        int _transLimit = 0;
        int _transExecType = 0;
        int _transInterval = 0;
        int _lastTransParentLocalTick = -1;
        int _copyCharacterMyId = -1;
    };
    typedef AIEffect274 AICopyCreatureAnimation;

    class AIEffect276 : public AIInnerEffect
    {
    public:
        AIEffect276(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect276();
        bool onEnter() override;
        void onExit() override;
        static void checkExecute(AIContext &aiContext, CharacterData *theTarget, CharacterData *theTrigger, int hplose);

    private:
        int _teamHpLose = 0;
        int getTeamHpLose();
        void addTeamHpLose(int hplose);
        void resetTeamHpLose();
        void triggerEffect();
    };
    typedef AIEffect276 AICastByTeamLoseHpEffect;

    class AIEffect277 : public AIInnerEffect
    {
    public:
        AIEffect277(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect277();
        bool onEnter() override;
        void onExit() override;
    };
    typedef AIEffect277 AIChangeAccuMaxTickEffect;
    
    class AIEffect278 : public AIInnerEffect
    {
    public:
        AIEffect278(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect278();
        bool onEnter() override;
        void onExit() override;
        static int getExtraHpSuck(AIContext &aiContext, CharacterData *theAttacker, CharacterData *theTarget);
    };
    typedef AIEffect278 AIChangeHPAdd1ByMarkEffect;

    class AIEffect279 : public AIInnerEffect
    {
    public:
        AIEffect279(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect279();
        bool onEnter() override;
        void onExit() override;
        static void checkExecuteEnterAction(AIContext &aiContext, CharacterData *theTarget, int theAction);
        static void checkExecuteExitAction(AIContext &aiContext, CharacterData *theTarget, int theAction);

    private:
        int _innerLocalTick = 0;
        int _innerInterval = 0;
        int _innerTriggerCd = 0;
        int _innerTriggerCntLowerLimit = 0;
        int _innerLastTrigParentTick = -1;
        int _innerPreTriggerCnt = 0;
        void onStartTrigger();
        void castInnerExec();
    };
    typedef AIEffect279 AICastByStateEffect;

	class AIEffect270 : public AIInnerEffect
	{
	public:
		AIEffect270(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AIEffect270();

		virtual bool onEnter();
		virtual void onExit();
	};
	typedef AIEffect270 AIConsumeHpAddCruitHurtEffect;
	
	// 271 - 承受伤害小于一定阈值伤害不生效，且触发执行id
	class AIEffect271 : public AIInnerEffect
	{
	public:
		AIEffect271(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AIEffect271();

		virtual bool onEnter();
		virtual void onExit();

		bool onStartTrigger();
		void castInnerExec();
		static void checkExecute(AIContext &aiContext, CharacterData *target);

	private:
		int _triggerCD;
		int _lastTrigParentTick = -1;
	};
	typedef AIEffect271 AIEndureHurtByConditionEffect;
	
	// 272 -  承受伤害小于一定阈值伤害不生效，且触发执行id
	class AIEffect272 : public AIInnerEffect
	{
	public:
		AIEffect272(AIContinuousEffectCaster* pParent, CharacterData* pTarget, InnerEffectExec* pExec);
		~AIEffect272();

		virtual bool onEnter();
		virtual void onExit();
        void onScheduleTick() override;

		static bool canCast(AIContext &aiContext, CharacterData *target, int hpdelta);
		static void checkExecute(AIContext &aiContext, CharacterData *target);
		static bool checkZeroEffect(AIContext &aiContext, CharacterData *target);

    private:
        int _triggerCnt = 0;
	};
	typedef AIEffect272 AIFatalHurtDoEffect;

		
	class AIEffect273 : public AIInnerEffect {
	public:
		AIEffect273(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
		~AIEffect273();
		bool onEnter() override;
		void onExit() override;
        
        bool isBroken = false;

		static int checkExecute(AIContext &aiContext, CharacterData *target, int HPDelta);

	};
	typedef AIEffect273 AIAbsorbAllTypeHurtShieldEffect;

    class AIEffect281 : public AIInnerEffect {
    public:
        AIEffect281(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect281();
        bool onEnter() override;
        void onExit() override;
        static void checkExecuteEnterAction(AIContext &aiContext, CharacterData *target);

    private:
        std::list<int> _casterMyIdsToNotifyOnExit;
        std::unordered_map<int, bool> _effectCastsToRemoveOnExit;
        int _oldAction = -1;
        int _oldActionRemainTick = 0;
        int _effectStateAnimation = 0;
    };


    //////////////////////////////////////AIEffect508 begin  28.7   eEFFECT_HEAL_EXT  ///////////////////////////////////////////////////////////////
    class AIEffect508 : public AIInnerEffect {

    public:

        int _leftExecuteNum = 0;  //剩余触发次数
        int _triggerHealCD = 0;
        


    public:
        AIEffect508(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect508();
        bool onEnter() override;
        void onExit() override;

        virtual void onScheduleTick();
        void checkExecuteHeal(AIContext& aiContext, CharacterData* tmpSource, CharacterData* tmpTarget, int addValue);

        //int getShieldValue() { return _shieldValue; }
       // void setShieldValue(int value) { _shieldValue = value; }



    private:

    };
    //////////////////////////////////////AIEffect507  end 28.7///////////////////////////////////////////////////////////////

    //////////////////////////////////////AIEffect509 begin  28.7   eEFFECT_HEAL_EXT  ///////////////////////////////////////////////////////////////
    class AIEffect509 : public AIInnerEffect {

    protected:
        int  _shieldValue = 0;  //当前护盾值
        int _leftExecuteNum = 0;  //剩余触发次数

        int _leftShieldBreakExecuteNum = 0;  //护盾被打破执行某个ID 的剩余次数

        int _healEffectTick = 0;  //  能触发增加护盾的持续时间

        int _shieldRunLeftTick = 0;   //  护盾自动爆破的剩余时间


    public:
        AIEffect509(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect509();
        bool onEnter() override;
        void onExit() override;

        virtual void checkCleanSameTypeExec();

        //判断时候可执行
        bool commonCheckVaild();
        //检测增加护盾值
        void checkShieldAdd(int addValue);

        int doAbsorbHurt(int hurt);
        //护盾值吸收伤害
        static  int  checkAbsorbHurt(AIContext& aiContext, CharacterData* target,int hurt, int hurtType,bool isPureHurt = false);

        int getShieldValue() { return _shieldValue; }
        void setShieldValue(int value) { _shieldValue = value; }



    private:

    };
    //////////////////////////////////////AIEffect509  end 28.7///////////////////////////////////////////////////////////////
        //////////////////////////////////////AIEffect511 begin  28.7   eEFFECT_HEAL_EXT  ///////////////////////////////////////////////////////////////
    class AIEffect511 : public AIInnerEffect {

    private:
        int _leftEffectTime = 0;  

    public:




    public:
        AIEffect511(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffect511();
        bool onEnter() override;
        void onExit() override;
        static int checkExecute(AIContext& aiContext, CharacterData* target, CharacterData* source,int HPDelta, int propetyType, bool isSimulate);
        void checkAndDoEfect(AIContext& aiContext, CharacterData* target, CharacterData* source, int HPDelta, int propetyType, bool isSimulate);
    private:

    };
 
    /// ////////////////////////////////////////AIEffect511  end //////////////////////////////////////////////////////////////

    class AIEffectScript : public AIInnerEffect {
    public:
        AIEffectScript(AIContinuousEffectCaster* theParent, CharacterData* theTarget, InnerEffectExec* theExec);
        ~AIEffectScript();
        bool onEnter() override;
        void onExit() override;
        void onScheduleTick() override;
    };

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    namespace EffectHelper {

        // 还原人物物攻法攻
        extern void RestoreCharacterBAMAPower(CharacterData* hero);

        // 改变目标状态的叠加
        extern bool IsEffectStateAnimationSupportMix(int stateAnimation);

        // @return real change value
        extern int applyNoneHPChange(CharacterData *pTarget, int attributeIndex, int changeValue);
        // @return real change value
        extern int applyHPChange(AIContext &aiContext, CharacterData *pSource, AIProjectile *pProjectile, CharacterData *pTarget, int skillID, int skillLevel, int attributeIndex, int changeValue, InnerEffectExec *pInnerExec, int markID, int casterID, AIInnerEffect *pInnerEffect);
        extern int applyHPChangeNative(AIContext &aiContext, CharacterData *pSource, AIProjectile *pProjectile, CharacterData *pTarget, int skillID, int skillLevel, int attributeIndex, int changeValue, InnerEffectExec *pInnerExec, int markID, int casterID, AIInnerEffect *pInnerEffect);
        
        extern void applyAttrSwapEffect(int theInnerEffectUid, int theAttrIndex, CharacterData *theTarget, int theSwapperAttrValue);
        extern void deapplyAttrEffect(int theInnerEffectUid, int theAttrIndex, CharacterData *theTarget);

        extern int checkExcuteCauseAddPropertyHurtResult(CharacterData *target, int hurtProperty, int param, bool isSimulate);

        extern bool isAnyInnerExecInCD(CharacterData *target, int effectExecID);

        extern int getExtendBuffTick(CharacterData *pTarget, int buffType, int oldTick);

        extern bool isDebuffedByType(CharacterData *pTarget, int buffType);

        extern int checkActivatePropHurtAdd(CharacterData *pTarget, int hurtProp, int hpdelta);

        extern float checkExcuteFloatEffect(CharacterData *pTarget, int etype, float param);

        extern void applyConvertAttrChange(AIContext &aiContext, CharacterData *pTarget, int firstAttrIndex, int firstAttrChange, int theInnerEffectUid, bool isDeapply);

        extern int checkExcuteMixChangeTypeEff(CharacterData *pTarget, int efftype, int param);

        extern void activateResistNReboundEff(AIContext &aiContext, CharacterData *target, CharacterData *reboundedTarget, int skillID, int skillLevel);

        extern int getAddHurtOnDistChangeValue(AIContext &aiContext, CharacterData *source, CharacterData *target, int hpdelta);

        extern int getExtendHitdownNFlyChangeValue(AIContext &aiContext, CharacterData *source, CharacterData *target, int tickLen);

        extern void checkActivateReviveEffect(AIContext &aiContext, CharacterData *target);
        
        // 12.0
        extern void reshowUIEffectsWhenCharacterRevive(AIContext &aiContext, CharacterData *t);

		extern void limitAttr(AIContext *ai, CharacterData *c, int idx);

		//---
		void collect_innereffect_record(AIInnerEffect *eff);

    } // namespace EffectHelper

} // namespace OnePiece

#endif /* defined(__OnePiece__AIContinuousEffectCaster__) */
